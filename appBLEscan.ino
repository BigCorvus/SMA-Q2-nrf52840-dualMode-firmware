void BLEscan() {
  //Simple function to demonstrate the Central scan functionality

  
  clearBlack();
  display.setTextSize(3);
  display.setCursor(5, 0);
  display.setTextColor(LCD_COLOR_WHITE);
  display.println("BLE Scan");
  display.refresh();

  Bluefruit.Scanner.start(0);                   // 0 = Don't stop scanning after n seconds

   while (insideSubMenue) { //the back button terminates the loop
     if (centralConnected) {
      display.setTextSize(1);
      display.setCursor(5, 150);
      display.print("central connected");
      display.refresh();
    }
    delay(100);
   }
  if (!centralConnected) Bluefruit.Scanner.stop(); //saves power
   insideBLEscanFunc=false;
  displayMenue(menueIndex);

}
