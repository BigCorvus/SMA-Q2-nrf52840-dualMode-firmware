void heartRate() {
  //Simple function to demonstrate the quality of the analog HR sensor data and its vulnerability to artifacts
  //to do: HR calculation and movement noise cancellation based on accelerometer data

  
  clearBlack();
  display.setTextSize(3);
  display.setCursor(5, 0);
  display.setTextColor(LCD_COLOR_WHITE);
  display.println("HR Sensor");
  display.refresh();
  digitalWrite(HRM_PWR, LOW); //turn the HRM LED on
  int prevY = 40;
  int X = 0;
  int photoPleth = 0;

  while (insideSubMenue) { //the back button terminates the loop
    accel.readBMA400AccelData(accelCount); // get 12-bit signed accel data

    // Now we'll calculate the accleration value into actual g's
    ax = (float)accelCount[0] * aRes - offset[0]; // get actual g value, this depends on scale being set
    ay = (float)accelCount[1] * aRes - offset[1];
    az = (float)accelCount[2] * aRes - offset[2];

    photoPleth = analogRead(HR_SIG);
    //figured the optimum range out by trial and error. This depends on the brightness of the LEDs, skin color, position of the sensor etc.
    photoPleth = constrain(photoPleth, 1948, 2600);
    photoPleth = map(photoPleth, 1948, 2600, 120, 30);

    display.fillRect(X, 30, 6, 120, LCD_COLOR_BLACK);
    display.drawLine(X - 1, prevY, X, photoPleth, LCD_COLOR_WHITE);
    display.refresh();
    prevY = photoPleth;
    X++;
    if (X >= 175) X = 0;

    delay(1);
  }
  insideHRfunc=false;
  digitalWrite(HRM_PWR, HIGH); //turn the HRM LEDs off again
  displayMenue(menueIndex);
}
