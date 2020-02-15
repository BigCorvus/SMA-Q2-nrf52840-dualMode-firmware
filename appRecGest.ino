#include "model.h"


//record gesture accelerometer data
//store it in .csv files in the internal flash
//create support vectors and classifiers using the python code on the bottom of the page (slightly modified to work with my packages)
//as described in the eloquent arduino tutorial
//https://eloquentarduino.github.io/2019/12/how-to-do-gesture-identification-on-arduino/
//recompile and re-upload the code with the newly created model.h file
//recognize the gestures on device

// Configuration for the datalogging file:
String logFileName = "gesture";
int logFileIndex = 0;
String fileType = ".csv";
char filename[64];
String gestFileName;


#define NUM_SAMPLES 60 
#define NUM_AXES 3
#define DEL_INTERVAL 10
double features[NUM_SAMPLES * NUM_AXES]; //don't make the array too large! Required flash storage will rise quickly and the classifier won't run anymore (device will hang and require a power toggle!)

void recGest() {

  clearBlack();
  display.setTextSize(3);
  display.setCursor(5, 0);
  display.setTextColor(LCD_COLOR_WHITE);
  display.println("RecGest");
  display.refresh();

//when the app is started a new file is created each time. Upon bootup gesture0.csv is created, so make sure all gesture files are deleted
  gestFileName = logFileName + String(logFileIndex) + fileType;
  gestFileName.toCharArray(filename, sizeof(filename));
  //the back button terminates the loop
  while (insideSubMenue) {


    if (inMotion) {
      recordIMU();
      singleVibe(); //single vibration means that recording is over

      if (recordGest) {
        if(!gestRecButUsed) { //this runs only once after power up to init the energy-hungry USB MSC and file system stuff (draws about 300µA more)
          initUSBandFS();
        }
        display.setTextSize(2);
        display.setCursor(5, 125);
        display.setTextColor(LCD_COLOR_WHITE, LCD_COLOR_BLACK);
        display.print("recording");
        display.setCursor(5, 25);
        display.print("          "); //delete the classifier messages
        display.refresh();
        //printFeatures();
        storeFeatures();
        gestRecButUsed =true; //this is set true once and for all
      } else {
        display.setTextSize(2);
        display.setCursor(5, 125);
        display.setTextColor(LCD_COLOR_WHITE, LCD_COLOR_BLACK);
        display.print("         ");
        display.refresh();
        classify();
      }

      inMotion = false;
      delay(3000); //some time to re-position your arm
      doubleVibe(); //double vibration means that a new gest can be recorded
    }
    delay(3);
  }

  insideGestRecFunc = false;
  recordGest = false;
  displayMenue(menueIndex);
  logFileIndex++;
}


void recordIMU() {
  float aX, aY, aZ;

  for (int i = 0; i < NUM_SAMPLES; i++) {
    accel.readBMA400AccelData(accelCount); // get 12-bit signed accel data

    // Now we'll calculate the accleration value into actual g's
    aX = (float)accelCount[0] * aRes - offset[0]; // get actual g value, this depends on scale being set
    aY = (float)accelCount[1] * aRes - offset[1];
    aZ = (float)accelCount[2] * aRes - offset[2];

    //    Serial.print(aX);
    //    Serial.print('\t');
    //    Serial.print(aY);
    //    Serial.print('\t');
    //    Serial.println(aZ);

    features[i * NUM_AXES + 0] = aX;
    features[i * NUM_AXES + 1] = aY;
    features[i * NUM_AXES + 2] = aZ;

    delay(DEL_INTERVAL);
  }
}

void printFeatures() {
  const uint16_t numFeatures = sizeof(features) / sizeof(double);

  for (int i = 0; i < numFeatures; i++) {
    Serial.print(features[i]);
    Serial.print(i == numFeatures - 1 ? "\r\n" : ","); //conditional operator(?:), operands to ?: have to have the same types
    delay(5);
  }
}

void classify() {
  Serial.print("Detected gesture: ");
  Serial.println(classIdxToName(predict(features)));

  display.setTextSize(2);
  display.setCursor(5, 25);
  display.setTextColor(LCD_COLOR_WHITE, LCD_COLOR_BLACK);
  display.print(classIdxToName(predict(features)));
  display.print("  ");
  display.refresh();
}

void storeFeatures() {
  File dataFile;
  // Open the datalogging file for writing.  The FILE_WRITE mode will open
  // the file for appending, i.e. it will add new data to the end of the file.
  dataFile = fatfs.open(filename, FILE_WRITE);
  // Check that the file opened successfully and write a line to it.
  if (dataFile) {
    // Take a new data reading from a sensor, etc.  For this example just
    const uint16_t numFeatures = sizeof(features) / sizeof(double);
    // Write a line to the file.  You can use all the same print functions
    // as if you're writing to the serial monitor.
    for (int i = 0; i < numFeatures; i++) {
      dataFile.print(features[i]);
      dataFile.print(i == numFeatures - 1 ? "\n" : ","); //conditional operator(?:), operands to ?: have to have the same types
      delay(1); //who knows how fast this qspi flash transfer is?
    }

    // Finally close the file when done writing.  This is smart to do to make
    // sure all the data is written to the file.
    dataFile.close();
  }
}



//python code
/*
  from sklearn.svm import SVC
  from micromlgen import port
  import numpy as np
  from glob import glob
  from os.path import basename

  #run in anaconda env py3k
  def load_features(folder):
    dataset = None
    classmap = {}
    for class_idx, filename in enumerate(glob('%s/*.csv' % folder)):
        class_name = basename(filename)[:-4]
        classmap[class_idx] = class_name
        samples = np.loadtxt(filename, delimiter=',') #the delimiter argument was missing
        labels = np.ones((len(samples), 1)) * class_idx
        samples = np.hstack((samples, labels))
        dataset = samples if dataset is None else np.vstack((dataset, samples))
    return dataset, classmap

  # put your samples in the dataset folder
  # one class per file
  # one feature vector per line, in CSV format
  features, classmap = load_features('dataset/')
  X, y = features[:, :-1], features[:, -1]
  classifier = SVC(kernel='poly',degree=2,gamma=0.001).fit(X, y)
  c_code = port(classifier, classmap=classmap)
  print(c_code)
*/
