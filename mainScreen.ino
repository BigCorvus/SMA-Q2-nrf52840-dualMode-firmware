void mainScreen() {
  if (!insideMenue) {
    //display.clearDisplay();
    //let's use a dark theme
    //draw time
    display.fillRect(0, 0, 176, 176, LCD_COLOR_BLACK);
    display.setTextSize(1);
    display.setTextColor(LCD_COLOR_YELLOW);
    display.setFont(&FreeSerifBold24pt7b);
    //display.setFont(&DSEG7_dig_only);
    display.setCursor(20, 100);
    if (hour() < 10) display.print("0");
    display.print(hour());
    display.print(" : ");
    //display.setCursor(4, 50);
    //display.setCursor(10, 45);
    if (minute() < 10) display.print("0");
    display.print(minute());
    
    //draw date
    display.setFont(&FreeSerifBold12pt7b);
    display.setCursor(30, 130);
    display.setTextSize(1);
    display.setTextColor(LCD_COLOR_GREEN);
    if (day() < 10) display.print("0");
    display.print(day());
    display.print(".");
    if (month() < 10) display.print("0");
    display.print(month());
    display.print(".");
    display.println(year());
    display.setFont();
    //draw battery status
    int battLevel = readBattery();
    //color coding batt levels
    if (battLevel > 70) {
      display.drawRect(0, 1, 21, 11, LCD_COLOR_GREEN);
      display.fillRect(21, 4, 3, 5, LCD_COLOR_GREEN);
      display.setTextColor(LCD_COLOR_GREEN);
    } else if (battLevel > 10) {
      display.drawRect(0, 1, 21, 11, LCD_COLOR_YELLOW);
      display.fillRect(21, 4, 3, 5, LCD_COLOR_YELLOW);
      display.setTextColor(LCD_COLOR_YELLOW);
    } else {
      display.drawRect(0, 1, 21, 11, LCD_COLOR_RED);
      display.fillRect(21, 4, 3, 5, LCD_COLOR_RED);
      display.setTextColor(LCD_COLOR_RED);
    }
    display.setTextSize(1);
    display.setCursor(2, 3);
    display.print(battLevel);
    display.println("%");

    //draw bluetooth logo
    if (peripheralConnected) {
      display.setCursor(166, 5);
      display.drawCircle(168, 8, 6, LCD_COLOR_BLUE);
      display.setTextColor(LCD_COLOR_WHITE);
      display.print("P");
    } else {
      display.setCursor(166, 5);
      display.drawCircle(168, 8, 6, LCD_COLOR_BLACK);
      display.setTextColor(LCD_COLOR_WHITE);
      display.print(" ");
    }

    if (centralConnected) {
      display.setCursor(150, 5);
      display.drawCircle(152, 8, 6, LCD_COLOR_BLUE);
      display.setTextColor(LCD_COLOR_WHITE);
      display.print("C");
    } else {
      display.setCursor(150, 5);
      display.drawCircle(152, 8, 6, LCD_COLOR_BLACK);
      display.setTextColor(LCD_COLOR_WHITE);
      display.print(" ");
    }

    //draw charging logo
    if (digitalRead(CH_STAT) == LOW) {
      display.setCursor(65, 3);
      display.setTextColor(LCD_COLOR_RED);
      display.print("charging");
    } else {
      display.setCursor(65, 3);
      display.setTextColor(LCD_COLOR_RED);
      display.print("        ");
    }
    //draw notification, if any
    if (notificationHere) {
      drawNoti();
    } else {
      deleteNoti();
    }

    //draw accel data
    accel.readBMA400AccelData(accelCount); // get 12-bit signed accel data

    // Now we'll calculate the accleration value into actual g's
    ax = (float)accelCount[0] * aRes - offset[0]; // get actual g value, this depends on scale being set
    ay = (float)accelCount[1] * aRes - offset[1];
    az = (float)accelCount[2] * aRes - offset[2];
    display.setCursor(2, 160);
    display.setTextColor(LCD_COLOR_YELLOW);
    display.setTextSize(1);
    // display.print("ax=");  display.print((int)1000 * ax);
    // display.print(" ay="); display.println((int)1000 * ay);
    // display.print(" az="); display.print((int)1000 * az); display.println(" mg");

#ifndef VER2_2
    display.setFont(&FreeSerifBold9pt7b);
    display.setTextSize(1);
    display.setCursor(95, 170);
    display.setTextColor(LCD_COLOR_RED);
    // Get reference pressure for relative altitude
    double referencePressure = ms5611.readPressure();
    // Read pressure and stuff
    uint32_t rawTemp = ms5611.readRawTemperature();
    uint32_t rawPressure = ms5611.readRawPressure();

    // Read true temperature & Pressure
    double realTemperature = ms5611.readTemperature();
    long realPressure = ms5611.readPressure();

    // Calculate altitude
    float absoluteAltitude = ms5611.getAltitude(realPressure);
    float relativeAltitude = ms5611.getAltitude(realPressure, referencePressure);
    display.print(absoluteAltitude); display.print("m");
#endif
    display.setFont(&FreeSerifBold12pt7b);
    display.setTextSize(1);
    display.setCursor(5, 170);
    display.setTextColor(LCD_COLOR_CYAN);
    display.print(accel.readBMA400StepCount() - prevStepCount); //subtract the steps the accelerometer has accumulated

    display.setFont(); //return to stock font again
    display.refresh();
  }
}

void drawNoti() {
  display.fillRect(0, 16, 176, 48, LCD_COLOR_MAGENTA);
  display.setFont(&FreeSerifBold9pt7b);
  display.setCursor(10, 40);
  display.setTextSize(1);
  display.setTextColor(LCD_COLOR_YELLOW);
  display.print(notificationString);
  display.setFont(); //return to stock font again
  display.setCursor(169, 16);
  display.setTextSize(1);
  display.print("x"); //have it look like a "real window on a pc" :>

}

void deleteNoti() {
  display.fillRect(0, 16, 176, 48, LCD_COLOR_BLACK);
}
