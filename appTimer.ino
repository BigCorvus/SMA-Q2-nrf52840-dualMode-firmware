void appTimer(){
  //start sequence
  //display Stuff, initialize....
  clearBlack();
  display.setTextSize(3);
  display.setCursor(5, 0);
  display.setTextColor(LCD_COLOR_WHITE);
  display.println("Timer");
  display.setTextSize(1);
  display.setCursor(5, 50);
  display.println("work in progress");
  display.refresh();
  while (insideSubMenue) { //the back button terminates the loop
    //loop of the APP
    delay(100);
   }
   insideTimer=false;
  displayMenue(menueIndex);
}
