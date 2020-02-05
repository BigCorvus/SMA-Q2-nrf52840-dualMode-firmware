void BLEscan() {
  //Simple function to demonstrate the Central scan functionality

  
  clearBlack();
  display.setTextSize(3);
  display.setCursor(5, 0);
  display.setTextColor(LCD_COLOR_WHITE);
  display.println("BLE Scan");
  display.setTextSize(1);
  display.setCursor(5, 50);
  display.println("work in progress");
  display.refresh();

   while (insideSubMenue) { //the back button terminates the loop
    delay(100);
   }
   insideBLEscanFunc=false;
  displayMenue(menueIndex);

}
