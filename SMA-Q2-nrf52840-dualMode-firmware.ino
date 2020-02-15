/*********************************************************************
  Basic firmware for the SMA-Q2 custom replacement board (v 2.x.x) based on the nRF52840 Feather.
  SPI, I2C and UART, the VDIV pin as well as the pushbutton (BCK button) have the default feather pinout.
  Uses the Adafruit nRF52 core version 0.17.0 and the feather bootloader

  Features and advantages over the native watch mainboard:
  -nRF52840: use the Adafruit nrf52 Adafruit Arduino core with all its great features and active developer community
  -FreeRTOS-Based core so most stuff is handled inside callbacks
  -code structure makes it easy to add your own "apps"
  -easy to use BLE central and peripheral functionality, use the thing as a standalone device to connect to multile sensors at the same time: star networks easy to implement
  Mobile <--> DualRole <--> peripheral Ble Uart
  -dual role functionality, always on display and accelerometer drawing ~1,1mA. OK for what it's capable of, actually, with central scanning only on demand, the current consumption is even less and the standby time is about one week.
  -native USB over the native charging cable
  -flash drive using the built-in qspi 2mbyte flash chip for example for datalogging (record gestures, train external ML algorithms and recognize them on the watch.....)
  -HID and USB serial port functionality (apparently with USB and flash drive functionality enabled the device draws ~200µA more)
  -of course programming via USB bootloader, which speeds up the development process
  -better accelerometer: extremely low power BMA400 or BMA456 (or other footprint-compatible Bosch parts) with low power step counting optimized for wrist
  -MS5611 altitude sensor/barometer (only in hw rev 2.1.x)
  -HW rev. 2.2 has a hardware power latching circuit so you can shut down the device completely.
  -open source hardware! modify it to accomodate your own parts on the PCB and leave out stuff you don't need
  -analog and easy to use heart rate monitor https://github.com/BigCorvus/PulseSensor-for-SMA-Q2
  -in one version I added a normally on reed switch which can disconnect the battery if a magnet (the charging cable) is placed near it.
   This can be useful if the device hangs and needs a reset or if the watch has to be stored for longer periods of time.

   So far implemented commands: "AT+DT=YYYYMMDDHHMMSS" //sent via mobile: set date and time
                                "AT+NO=someNotification" //sent via mobile: draw the someNotification string on a "window" on a main screen and trigger vibration pattern. Can be deleted via up button
                                "AT+SE=someData" //either sent from mobile or external nRF UART sensor, will display its data within the appBLEsensors() app screen

  Note: I'm including some source files of publicly available libraries in the project folder to maintain
  compatibility and try to provide URL references to the repositories.
  If this violates licences of any kind please let me know and I will
  correct that.
*********************************************************************/
#include "FreeStack.h"
#include <Arduino.h>
#include <bluefruit.h>
#include <SPI.h>
#include <Wire.h>
#include <stdint.h>
#include "Adafruit_GFX.h" //we all know where this comes from
#include "ColorMemLCD.h" //https://github.com/BigCorvus/ColorMemLCD
#include "BMA400.h" //https://github.com/kriswiner/BMA400 I modified it to enable step counting
#include "TimeLib.h"
#include "MS5611.h" //https://github.com/jarzebski/Arduino-MS5611
//external libraries:
//hoping they will stay compatible
#include "SdFat.h" //this is the adafruit fork https://github.com/adafruit/SdFat
#include "Adafruit_SPIFlash.h" //https://github.com/adafruit/Adafruit_SPIFlash
#include "Adafruit_TinyUSB.h" //https://github.com/adafruit/Adafruit_TinyUSB_Arduino

#include "FreeSerifBold24pt7b.h"
#include "FreeSerifBold12pt7b.h"
#include "FreeSerifBold9pt7b.h"

//choose only one, if you chose none, you'll compile for hw ver. 2.1.1. which uses both BMA 4xx interrupts and an MS5611
//#define VER2_2_0 //comment this out to disable the MS5611 and invert logic of BTN_OK, which will be the power-up button
//#define VER2_1_0 //comment this out to tie the accelerometer power pin to GPIO 9 instead of having a second accel interrupt (only for hw ver 2.1.0)

#define VIBRO 12 //P0.08
#define VBAT 20 //P0.29
#define HR_SIG 19 //p0.03
#define HRM_PWR 18//P0.02
#define BLT 17//P0.28
#define CH_STAT 16//P0.30
#define DISP_SS 2//P0.10
#define BTN_UP 10//P0.27
#define BTN_OK 13//P1.02
#define BTN_DN 6//P0.07
#define BTN_BCK 7//P1.02
#define SCK 26//
#define MOSI 25//
#define EXTCOMIN 5 //P1.08
#define DISP 11 //P0.06
#define AINT1 8//0.16

#ifndef VER2_1_0
#define AINT2 9 //P0.26
#else
#define ACCEL_PWR 9 //P0.26
#endif

#define PHOLD A0 //P0.04
//default hw version if neither of the below is commented out will be 2.1.1 (both accel interrupts connected, MS5611 installed, no board for this version exists becaue it was mod-wired from 2.1.0)
//#define VER2_2_0 //comment this out to disable the MS5611 and invert logic of BTN_OK, which will be the power-up button
//#define VER2_1_0 //comment this out to tie the accelerometer power pin to GPIO 9 instead of having a second accel interrupt

//-------------------menue variables-------------------
//in order to add a new app you have to:
//add the name to appNames String array, take care of the order
//create a boolean "insideFuncXXX"
//add a "case" in the appropriate order to void ok_callback(void) ISR
//add an if statement with function call in loop()
//create the app function inside a new .ino file ("add new tab") ideally named appFuncXXX (easier to find)
//in the sketch folder with the following structure:
/*
  void funcXXX(){
  //start sequence
  //display Stuff, initialize....
  clearBlack();
  display.setTextSize(3); //standard font is 5x8, so scale it by 3 ->15x24
  display.setCursor(5, 0);
  display.setTextColor(LCD_COLOR_WHITE);
  display.println("funcXXX");
  display.refresh();
  while (insideSubMenue) { //the back button terminates the loop
    //loop of the APP
    delay(100); //get some sleep else the app hangs!
   }
   insideFuncXXX=false;
  displayMenue(menueIndex);
  }
*/
//note: since the project structure is based on freeRTOS and is callback-driven, not all app code is inside this app function
//for example the button callbacks and bluetooth functions have to be adapted
unsigned long prevStepCount = 0;
byte menueIndex = 0;
String appNames [] = {"BLE Scan", "BLE Sensors", "Rec Gesture", "Timer", "PhotoPleth", "Shutdown"}; //enter your app names in correct order
#define NUMITEMS(arg) ((unsigned int) (sizeof (arg) / sizeof (arg [0]))) // number of items in an array
byte maxMenueIndex = NUMITEMS(appNames) - 1;  //the actual nuber of apps minus one
boolean insideHRfunc = false;
boolean insideTimer = false;
boolean insideBLEsensors = false;
boolean recordGest, gestRecButUsed, insideGestRecFunc = false;
boolean insideBLEscanFunc = false;
//------------------------------------------------------
boolean booting, insideMenue, insideSubMenue, centralConnected, peripheralConnected, charging, notificationHere = false;
boolean inMotion = false;
String notificationString = "";
float mv_per_lsb = 3000.0F / 4096.0F; // 12-bit ADC with 3.0V input range

// OTA DFU service
BLEDfu bledfu;
// Peripheral uart service
BLEUart bleuart;
// Central uart client
BLEClientUart clientUart;

ColorMemLCD display(SCK, MOSI, DISP_SS, EXTCOMIN);
#ifndef VER2_2
MS5611 ms5611;
#endif
BMA400 accel(AINT1, AINT2); // instantiate BMA400 class
SoftwareTimer extcommTimer; //trigger the EXTCOMIN pin of the dislay a couple of times a second

//start QSPI, Flash storage, file system and USB only if it is needed, because they draw additional 200-300µA
Adafruit_FlashTransport_QSPI flashTransport(PIN_QSPI_SCK, PIN_QSPI_CS, PIN_QSPI_IO0, PIN_QSPI_IO1, PIN_QSPI_IO2, PIN_QSPI_IO3);
Adafruit_SPIFlash flash(&flashTransport);
// file system object from SdFat
FatFileSystem fatfs;
FatFile root;
FatFile file;
// USB Mass Storage object
Adafruit_USBD_MSC usb_msc;
// Set to true when PC writes to flash
bool changed;
/* Specify sensor parameters (sample rate is twice the bandwidth)
   choices are:
      AFS_2G, AFS_4G, AFS_8G, AFS_16G
      SR_15_5Hz, SRW_25Hz, SR_50Hz, SR_100Hz, SR_200Hz, SR_400Hz, SR_800Hz
      sleep_Mode, lowpower_Mode, normal_Mode
      osr0 (lowest power, lowest oversampling,lowest accuracy), osr1, osr2, osr3 (highest power, highest oversampling, highest accuracy)
      acc_filt1 (variable filter), acc_filt2 (fixed 100 Hz filter), acc_filt_lp (fixed 100 Hz filter, 1 Hz bandwidth)
*/
uint8_t Ascale = AFS_2G, SR = SR_200Hz, power_Mode = lowpower_Mode, OSR = osr0, acc_filter = acc_filt2;

float aRes;             // scale resolutions per LSB for the sensor
int16_t accelCount[3];  // Stores the 16-bit signed accelerometer sensor output
int16_t tempCount;      // temperature raw count output
float   temperature;    // Stores the real internal chip temperature in degrees Celsius
float ax, ay, az;       // variables to hold latest sensor data values
float offset[3];        // accel bias offsets

/*------------------------------------------------------------------*/
/* Setup
  ------------------------------------------------------------------*/

void setup()
{
  booting = true;
  initPins();

  Wire.begin();
  Serial.begin(115200);
  delay(2000);
  //while ( !Serial ) delay(10);   // for nrf52840 with native usb

  // Configure the timer with xxx ms interval, with our callback
  extcommTimer.begin(500, extcomm_timer_callback);
  // Start the timer
  extcommTimer.start();

  display.begin();
  delay(100);
  welcomeScreenAndSelfTest();
  delay(100);

  initBLE();

  Serial.println("SMA Q2 retrofit v2.x.x Dual Role BLEUART FW");
  Serial.println("-------------------------------------\n");

  initAccelAndBarometer();
  delay(2000);
  //initUSBandFS();

  SetDateTimeString("20200118095900");
  // Create loop2() using Scheduler to run in 'parallel' with loop()
  //this will refresh the main screen every minute
  //Scheduler.startLoop(loop2);
  booting = false;
}

/*------------------------------------------------------------------*/
/* END Setup
  ------------------------------------------------------------------*/


/*------------------------------------------------------------------*/
/* Loop
  ------------------------------------------------------------------*/
void loop()
{
  if (insideHRfunc) {
    heartRate();
  }

  if (insideGestRecFunc) {
    recGest();
  }

  if (insideBLEscanFunc) {
    BLEscan();
  }

  if (insideTimer) {
    appTimer();
  }

  if (insideBLEsensors) {
    appBLEsensors();
  }

  //refresh dislay approx. every minute. Not perfect, I know.
  if (second() == 0)
    if (!insideMenue) mainScreen();

  //reset step count every day
  if (second() == 0 && minute() == 0 && hour() == 0) {
    prevStepCount = accel.readBMA400StepCount();
  }


  delay(500);
}

//not used
void loop2()
{
  if (!insideMenue) mainScreen();
  delay(60000);              // wait for a minute, remember: delay() means "sleep"
}
/*------------------------------------------------------------------*/
/* END Loop
  ------------------------------------------------------------------*/



/*------------------------------------------------------------------*/
/* button interrupt service routines
    to do: maybe implement debounce and long/double press states in the future?
    the buttons do not seem to be that bouncy
  ------------------------------------------------------------------*/
void bck_callback(void)
{

  //turnOff = true;
  //shutdownSystem();
  if (insideSubMenue) {
    insideSubMenue = false;
    displayMenue(menueIndex);
  } else if (insideMenue && !insideSubMenue) {
    insideMenue = false;
    mainScreen();
  } else {
    //what to do while in mainScreen
  }

}
void ok_callback(void)
{

  //digitalToggle(BLT);
  if (insideMenue && !insideSubMenue) {
    insideSubMenue = true;
    switch (menueIndex) {
      case 0:
        //enter BLE scan function
        insideBLEscanFunc = true;
        break;

      case 1:
        //Show data of connected BLE UART sensors
        insideBLEsensors = true;
        break;

      case 2:
        insideGestRecFunc = true;
        break;

      case 3:
        insideTimer = true;
        break;

      case 4:
        insideHRfunc = true;
        break;

      case 5:
        clearBlack();
        display.setTextSize(2);
        display.setCursor(2, 70);
        display.setTextColor(LCD_COLOR_WHITE);
        display.println("Shutdown...");
        display.setTextColor(LCD_COLOR_YELLOW);
        display.println("press OK --->");
        display.println("to turn on");
        display.refresh();
        delay(3000); //some delay to make sure the device does not reboot again immediately
        clearBlack();
        display.refresh();
        shutdownSystem();
        break;

    }
  } else if (!insideMenue && !insideSubMenue) {
    insideMenue = true;
    menueIndex = 0;
    displayMenue(menueIndex);
  }

}
void dn_callback(void)
{
  //enterOTADfu();
  if (insideMenue && !insideSubMenue) {
    if (menueIndex >= maxMenueIndex) menueIndex = maxMenueIndex; else menueIndex++;
    displayMenue(menueIndex);
  } else if (insideSubMenue) {
    //it depends
    if (insideGestRecFunc) {
      //this decides whether we record or recognize gestures
      if (!recordGest) recordGest = true; else recordGest = false;

    }
  } else {
    digitalToggle(BLT);
  }

}
void up_callback(void)
{
  if (insideMenue && !insideSubMenue) {
    if (menueIndex <= 0) menueIndex = 0; else menueIndex--;
    displayMenue(menueIndex);
  } else if (insideSubMenue) {
    //it depends

  } else {
    notificationHere = false; //delete the notification window on the main screen.
    notificationString = "";
    mainScreen();
  }
}

/*------------------------------------------------------------------*/
/* accelerometer interrupt service routines
  ------------------------------------------------------------------*/
void accOne_callback(void)
{
  inMotion = true;
  //digitalWrite(BLT, HIGH);
}

#ifndef VER2_1_0
void accTwo_callback(void)
{

  //digitalWrite(BLT, LOW);
}
#endif


/**
   Software Timer callback is invoked via a built-in FreeRTOS thread with
   minimal stack size. Therefore it should be as simple as possible. If
   a periodically heavy task is needed, please use Scheduler.startLoop() to
   create a dedicated task for it.

   More information http://www.freertos.org/RTOS-software-timer.html
*/
void extcomm_timer_callback(TimerHandle_t xTimerID)
{
  // freeRTOS timer ID, ignored if not used
  (void) xTimerID;

  digitalToggle(EXTCOMIN);
}

//show the main screen if the charge cable connects/disconnects
void charge_callback(void) {
  if (!insideMenue && !booting) mainScreen();
}
