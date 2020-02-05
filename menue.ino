

void displayMenue(int itemIndex) {
  //clearBlack();
  display.fillRect(0, 0, 176, 176, LCD_COLOR_BLUE);
  display.setTextSize(3);
  display.setCursor(50, 0);
  display.setTextColor(LCD_COLOR_WHITE);
  display.println("Menu");
  display.setTextSize(2);

  for(int i=0; i<=maxMenueIndex; i++){
    if(i==itemIndex){
      //highlight the selcted item
      display.setTextColor(LCD_COLOR_WHITE, LCD_COLOR_GREEN); 
    }else {
        display.setTextColor(LCD_COLOR_WHITE, LCD_COLOR_BLUE);
    }
     display.println(appNames[i]);
  }

  display.refresh();
}
