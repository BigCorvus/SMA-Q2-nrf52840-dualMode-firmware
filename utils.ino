void clearBlack() {
  display.fillRect(0, 0, 176, 176, LCD_COLOR_BLACK);
  //display.refresh();
}


void singleVibe() {
  digitalWrite(VIBRO, HIGH);
  delay(200);
  digitalWrite(VIBRO, LOW);
}

void doubleVibe() {
  digitalWrite(VIBRO, HIGH);
  delay(100);
  digitalWrite(VIBRO, LOW);
  delay(100);
  digitalWrite(VIBRO, HIGH);
  delay(100);
  digitalWrite(VIBRO, LOW);
}

void nTimesVibe(uint8_t times, uint8_t wait) {
  for (int i = 0; i < times; i++) {
    digitalWrite(VIBRO, HIGH);
    delay(wait);
    digitalWrite(VIBRO, LOW);
    delay(wait);
  }
}

//https://github.com/atc1441/D6-arduino-nRF5/blob/master/libraries/D6Examples/examples/D6Emulator/D6Emulator.ino
String GetDateTimeString() {
  String datetime = String(year());
  if (month() < 10) datetime += "0";
  datetime += String(month());
  if (day() < 10) datetime += "0";
  datetime += String(day());
  if (hour() < 10) datetime += "0";
  datetime += String(hour());
  if (minute() < 10) datetime += "0";
  datetime += String(minute());
  return datetime;
}

void SetDateTimeString(String datetime) {
  int year = datetime.substring(0, 4).toInt();
  int month = datetime.substring(4, 6).toInt();
  int day = datetime.substring(6, 8).toInt();
  int hr = datetime.substring(8, 10).toInt();
  int min = datetime.substring(10, 12).toInt();
  int sec = datetime.substring(12, 14).toInt();
  setTime( hr, min, sec, day, month, year);
}

//https://github.com/G6EJD/LiPo_Battery_Capacity_Estimator/blob/master/ReadBatteryCapacity_LIPO.ino
int readBattery() {
  uint8_t percentage = 100;
  //calculate average of multiple readings
  //    int numIterations = 5;
  //    int readingArray[numIterations] ;
  //    int avgReading = 0;
  //    for (int i = 0; i < numIterations - 1; i++) {
  //      readingArray[i] = analogRead(VBAT);
  //      delay(10);
  //    }
  //    for (int i = 0; i < numIterations - 1; i++) {
  //      avgReading += readingArray[i];
  //    }
  //    avgReading = avgReading / numIterations;

  float voltage = ((float)analogRead(VBAT) * mv_per_lsb * 2.0) / 1000.0;     //
  percentage = 2808.3808 * pow(voltage, 4) - 43560.9157 * pow(voltage, 3) + 252848.5888 * pow(voltage, 2) - 650767.4615 * voltage + 626532.5703;
  if (voltage > 4.19) percentage = 100;
  else if (voltage <= 3.50) percentage = 0;
  return percentage;
}

//shutdown function, draws about 120µA with only the accel on (if the LDO pin is no pulled low)
void shutdownSystem() {
#ifndef VER2_2
  ms5611.begin(MS5611_ULTRA_LOW_POWER);
#endif
  //turn off all components
  digitalWrite(VIBRO, LOW);
  digitalWrite(HRM_PWR, HIGH); //high is off
  digitalWrite(BLT, LOW); //LOW is OFF
  digitalWrite(DISP, LOW);
  digitalWrite(PHOLD, LOW); //pull LDO EN pin low to kill power (only implemented in hw ver. 2.2)
  systemOff(BTN_OK, LOW);//arg 1 : pin, arg 2: wake logic
  while (1);
}
