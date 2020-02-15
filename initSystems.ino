void initPins(){
  //the first thing we do is pull the LDO enable pin high to keep the device on.
  //(only implemented in hw ver. 2.2)
  pinMode(PHOLD, OUTPUT);
  digitalWrite(PHOLD, HIGH); 
  //init pins
  pinMode(CH_STAT, INPUT_PULLUP);//status pin of the LIPO charger, LOW means "charging"
  pinMode(AINT1, INPUT); //accel interrupt 1
  #ifndef VER2_1_0
  pinMode(AINT2, INPUT); //
  #else
  pinMode(ACCEL_PWR, OUTPUT); //
  digitalWrite(ACCEL_PWR, HIGH); //turn on the accel
  #endif
  pinMode(BTN_UP, INPUT_PULLUP);
  #ifndef VER2_2
  pinMode(BTN_OK, INPUT_PULLUP);
  #else
  pinMode(BTN_OK, INPUT); //2.2 has inverted logic on this pin to drive the EN pin of the LDO high via a capacitor
  #endif
  pinMode(BTN_DN, INPUT_PULLUP);
  pinMode(BTN_BCK, INPUT_PULLUP);
  // ISR_DEFERRED flag causees the callback to be deferred from ISR context
  // and invoked within a callback thread.
  // It is required to use ISR_DEFERRED if callback function take long time
  // to run e.g Serial.print() or using any of Bluefruit API() which will
  // potentially call rtos API
  attachInterrupt(BTN_BCK, bck_callback, ISR_DEFERRED | FALLING);
  #ifndef VER2_2
  attachInterrupt(BTN_OK, ok_callback, ISR_DEFERRED | FALLING);
  #else
  attachInterrupt(BTN_OK, ok_callback, ISR_DEFERRED | RISING);
  #endif
  attachInterrupt(BTN_DN, dn_callback, ISR_DEFERRED | FALLING);
  attachInterrupt(BTN_UP, up_callback, ISR_DEFERRED | FALLING);
  attachInterrupt(AINT1, accOne_callback,  RISING);
  #ifndef VER2_1_0
  attachInterrupt(AINT2, accTwo_callback,  RISING);
  #endif
  attachInterrupt(CH_STAT, charge_callback, ISR_DEFERRED | CHANGE);
  //output pins
  pinMode(VIBRO, OUTPUT);
  pinMode(DISP, OUTPUT);
  pinMode(EXTCOMIN, OUTPUT);
  pinMode(HRM_PWR, OUTPUT);
  pinMode(BLT, OUTPUT);

  digitalWrite(BLT, LOW); //LOW is OFF
  digitalWrite(VIBRO, LOW); //LOW is OFF
  digitalWrite(HRM_PWR, HIGH); //HIGH is OFF
  digitalWrite(DISP, HIGH);

  //************************************************************************
  // Set the analog reference to 3.0V (default = 3.6V)
  analogReference(AR_INTERNAL_3_0);
  // Set the resolution to 12-bit (0..4095)
  analogReadResolution(12); // Can be 8, 10, 12 or 14
  //************************************************************************
}

void welcomeScreenAndSelfTest(){
  clearBlack();
  display.refresh();
  display.setTextSize(1);
  display.setTextColor(LCD_COLOR_YELLOW);
  display.setCursor(0, 0);
  display.println("SMA-Q2 retrofit with");
  display.println("nRF52840");
  display.println("Hardware v2.1.1");
  display.println("Dual Role  FW");
  display.refresh();

  //test HRM, BLT and motor
  digitalWrite(VIBRO, HIGH);
  digitalWrite(HRM_PWR, LOW);
  digitalWrite(BLT, HIGH);
  delay(500);
  digitalWrite(VIBRO, LOW);
  digitalWrite(HRM_PWR, HIGH);
  digitalWrite(BLT, LOW);
  
}

void initAccelAndBarometer(){
  //https://github.com/kriswiner/BMA400
  // Read the BMA400 Chip ID register, this is a good test of communication
  Serial.println("BMA400 accelerometer...");
  byte c = accel.getChipID();  // Read CHIP_ID register for BMA400
  Serial.print("BMA400 ");
  Serial.print("I AM ");
  Serial.print(c, HEX);
  Serial.print(" I should be ");
  Serial.println(0x90, HEX);
  Serial.println(" ");
  delay(500);

  if (c == 0x90) // check if all I2C sensors with WHO_AM_I have acknowledged
  {
    Serial.println("BMA400 is online"); Serial.println(" ");
    display.setTextColor(LCD_COLOR_WHITE);
    display.println("BMA400 good, calibrating...");
    display.refresh();
    aRes = accel.getAres(Ascale);                                       // get sensor resolutions, only need to do this once
    accel.resetBMA400();                                                // software reset before initialization
    delay(100);
    accel.selfTestBMA400();                                             // perform sensor self test
    accel.resetBMA400();                                                // software reset before initialization
    delay(1000);                                                         // give some time to read the screen
    accel.CompensationBMA400(Ascale, SR, normal_Mode, OSR, acc_filter, offset); // quickly estimate offset bias in normal mode
    accel.initBMA400(Ascale, SR, power_Mode, OSR, acc_filter);          // Initialize sensor in desired mode for application

  }
  else
  {
    if (c != 0x90) Serial.println(" BMA400 not functioning!");
  }
  accel.getStatus(); // read status of interrupts to clear
#ifndef VER2_2
  // Ultra high resolution: MS5611_ULTRA_HIGH_RES
  // (default) High resolution: MS5611_HIGH_RES
  // Standard: MS5611_STANDARD
  // Low power: MS5611_LOW_POWER
  // Ultra low power: MS5611_ULTRA_LOW_POWER
  while (!ms5611.begin(MS5611_ULTRA_HIGH_RES))
  {
    Serial.println("Could not find a valid MS5611, check wiring!");
    delay(500);
  }
  display.println("MS5611 good");
#endif
  display.refresh();
}

void initUSBandFS(){

  pinMode(LED_BUILTIN, OUTPUT);
  flash.begin();
  // Set disk vendor id, product id and revision with string up to 8, 16, 4 characters respectively
  usb_msc.setID("Q2 nRF52840", "External Flash", "1.0");
  // Set callback
  usb_msc.setReadWriteCallback(msc_read_cb, msc_write_cb, msc_flush_cb);
  // Set disk size, block size should be 512 regardless of spi flash page size
  usb_msc.setCapacity(flash.pageSize()*flash.numPages()/512, 512);
  // MSC is ready for read/write
  usb_msc.setUnitReady(true);  
  usb_msc.begin();
  // Init file system on the flash
  fatfs.begin(&flash);
  Serial.println("Adafruit TinyUSB Mass Storage External Flash example");
  Serial.print("JEDEC ID: "); Serial.println(flash.getJEDECID(), HEX);
  Serial.print("Flash size: "); Serial.println(flash.size());
  changed = true; // to print contents initially
}
