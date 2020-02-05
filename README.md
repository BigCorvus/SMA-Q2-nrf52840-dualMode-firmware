# SMA-Q2-nrf52840-dualMode-firmware
Basic firmware for the SMA-Q2 custom replacement board (v 2.x.x) based on the nRF52840 Feather.
  SPI, I2C and UART, the VDIV pin as well as the pushbutton (BCK button) have the default feather pinout.
  Uses the Adafruit nRF52 core version 0.17.0 and the feather bootloader

  Features and advantages over the native watch mainboard:  
  -nRF52840: use the Adafruit nrf52 Adafruit Arduino core with all its great features and active developer community  
  -FreeRTOS-Based core so most stuff is handled inside callbacks  
  -code structure makes it easy to add your own "apps"  
  -easy to use BLE central and peripheral functionality, use the thing as a standalone device to connect to multile sensors at the same time: star networks easy to implement  
  Mobile <--> DualRole <--> peripheral Ble Uart  
  -dual role functionality, always on display and accelerometer drawing ~1,1mA. OK for what it's capable of.  
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
   -issues: RF performance is worse than the original antenna/board (maybe due to close proximity of the antenna to the battery?)  
   -https://github.com/adafruit/Adafruit_nRF52_Arduino/issues/150 sometimes central connection issues seem to occur after some reconnects
   
   


   So far implemented commands:  
   "AT+DT=YYYYMMDDHHMMSS" //sent via mobile: set date and time  
   "AT+NO=someNotification" //sent via mobile: draw the someNotification string on a "window" on a main screen and trigger vibration pattern. Can be deleted via up button  
   "AT+SE=someData" //either sent from mobile or external nRF UART sensor, will display its data within the appBLEsensors() app screen  

Note: I'm including some source files of publicly available libraries in the project folder to maintain 
compatibility and try to provide URL references to the repositories.
If this violates licences of any kind please let me know and I will 
correct that. 
