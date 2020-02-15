void appBLEsensors() {
  //start sequence
  //display Stuff, initialize....
  clearBlack();
  display.setTextSize(3);
  display.setCursor(0, 0);
  display.setTextColor(LCD_COLOR_WHITE);
  display.println("BLE Sens.");
  display.setTextSize(2);
  display.setTextColor(LCD_COLOR_CYAN);
  display.setCursor(0, 25);
  display.println("waiting");
  display.println("for data...");
  display.refresh();
  while (insideSubMenue) { //the back button terminates the loop
    //loop of the APP
    delay(100);
  }
  insideBLEsensors = false;
  displayMenue(menueIndex);
}
