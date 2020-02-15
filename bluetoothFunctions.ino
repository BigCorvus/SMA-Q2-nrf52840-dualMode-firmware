void initBLE() {
  // Initialize Bluefruit with max concurrent connections as Peripheral = 1, Central = 1
  // SRAM usage required by SoftDevice will increase with number of connections
  Bluefruit.begin(1, 1);
  // - nRF52840: -40dBm, -20dBm, -16dBm, -12dBm, -8dBm, -4dBm, 0dBm, +2dBm, +3dBm, +4dBm, +5dBm, +6dBm, +7dBm and +8dBm.
  Bluefruit.setTxPower(5);    // Check bluefruit.h for supported values
  Bluefruit.setName("Q2 nRF52840 Dual");
  // Callbacks for Peripheral
  Bluefruit.Periph.setConnectCallback(prph_connect_callback);
  Bluefruit.Periph.setDisconnectCallback(prph_disconnect_callback);
  // Callbacks for Central
  Bluefruit.Central.setConnectCallback(cent_connect_callback);
  Bluefruit.Central.setDisconnectCallback(cent_disconnect_callback);
  //   Bluefruit.Central.setConnIntervalMS (uint16_t min_ms, uint16_t max_ms)
  // To be consistent OTA DFU should be added first if it exists
  //bledfu.begin();
  // Configure and Start BLE Uart Service
  bleuart.begin();
  bleuart.setRxCallback(prph_bleuart_rx_callback);
  // Init BLE Central Uart Serivce
  clientUart.begin();
  clientUart.setRxCallback(cent_bleuart_rx_callback);
  /* Start Central Scanning
     - Enable auto scan if disconnected
     - Interval = 100 ms, window = 80 ms
     - Filter only accept bleuart service
     - Don't use active scan
     - Start(timeout) with timeout = 0 will scan forever (until connected)
  */
  /* Advertising Guideline from Apple
     https://developer.apple.com/library/content/qa/qa1931/_index.html

     The recommended advertising pattern and advertising intervals are:
     - First, advertise at 20 ms intervals for at least 30 seconds
     - If not discovered after 30 seconds, you may change to one of the following
     longer intervals: 152.5 ms, 211.25 ms, 318.75 ms, 417.5 ms, 546.25 ms,
     760 ms, 852.5 ms, 1022.5 ms, 1285 ms
  */
  Bluefruit.Scanner.setRxCallback(scan_callback);
  Bluefruit.Scanner.restartOnDisconnect(false);
  Bluefruit.Scanner.setInterval(160, 80); // in unit of 0.625 ms
  Bluefruit.Scanner.filterUuid(bleuart.uuid);
  Bluefruit.Scanner.useActiveScan(false);
  //Bluefruit.Scanner.start(0);                   // 0 = Don't stop scanning after n seconds
  // Set up and start advertising
  startAdv();
}

void startAdv(void)
{
  // Advertising packet
  Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
  Bluefruit.Advertising.addTxPower();

  // Include bleuart 128-bit uuid
  Bluefruit.Advertising.addService(bleuart);

  // Secondary Scan Response packet (optional)
  // Since there is no room for 'Name' in Advertising packet
  Bluefruit.ScanResponse.addName();

  /* Start Advertising
     - Enable auto advertising if disconnected
     - Interval:  fast mode = 20 ms, slow mode = 152.5 ms
     - Timeout for fast mode is 30 seconds
     - Start(timeout) with timeout = 0 will advertise forever (until connected)

     For recommended advertising interval
     https://developer.apple.com/library/content/qa/qa1931/_index.html
  */
  Bluefruit.Advertising.restartOnDisconnect(true);
  Bluefruit.Advertising.setInterval(32, 244);    // in unit of 0.625 ms
  Bluefruit.Advertising.setFastTimeout(30);      // number of seconds in fast mode
  Bluefruit.Advertising.start(0);                // 0 = Don't stop advertising after n seconds
}


/*------------------------------------------------------------------*/
/* Peripheral
  ------------------------------------------------------------------*/
void prph_connect_callback(uint16_t conn_handle)
{
  peripheralConnected = true;
  if (!insideMenue && !booting) mainScreen();
  // Get the reference to current connection
  BLEConnection* connection = Bluefruit.Connection(conn_handle);

  char peer_name[32] = { 0 };
  connection->getPeerName(peer_name, sizeof(peer_name));

  Serial.print("[Prph] Connected to ");
  Serial.println(peer_name);

}

void prph_disconnect_callback(uint16_t conn_handle, uint8_t reason)
{
  peripheralConnected = false;
  if (!insideMenue && !booting) mainScreen();
  (void) conn_handle;
  (void) reason;

  Serial.println();
  Serial.println("[Prph] Disconnected");
}

void prph_bleuart_rx_callback(uint16_t conn_handle)
{
  (void) conn_handle;

  // Forward data from Mobile to our peripheral
  char str[20 + 1] = { 0 };
  bleuart.read(str, 20);

  //set date and time
  //command: AT+DT=
  //format: YYYYMMDDHHMMSS
  String prphInString = str;
  String DateTimeString = "";
  if (prphInString.indexOf("AT+DT=") >= 0 ) {
    //****************************set date and time****************************
    DateTimeString = prphInString.substring(6, 19);
    SetDateTimeString(DateTimeString);
    if (!insideMenue) mainScreen();
    //Scheduler.startLoop(loop2);
  } else if (prphInString.indexOf("AT+SE=") >= 0) {
    //****************************for the BLE Sensors app****************************
    if (insideBLEsensors) {
      display.fillRect(0, 25, 176, 176, LCD_COLOR_BLACK);
      display.setTextSize(2);
      display.setTextColor(LCD_COLOR_CYAN);
      display.setCursor(0, 25);
      display.println(prphInString.substring(6));
      display.refresh();
    }

  } else if (prphInString.indexOf("AT+NO=") >= 0) {
    //****************************receive notification****************************
    notificationHere = true;
    notificationString = prphInString.substring(6);
    if (!insideMenue) mainScreen();
    nTimesVibe(4, 200); //vibrate!
  }

  Serial.print("[Prph] RX: ");
  Serial.println(str);

  if ( clientUart.discovered() )
  {
    clientUart.print(str);
  } else
  {
    bleuart.println("[Prph] Central role not connected");
  }
}

/*------------------------------------------------------------------*/
/* Central
  ------------------------------------------------------------------*/
void scan_callback(ble_gap_evt_adv_report_t* report)
{

  uint8_t buffer[32];
  memset(buffer, 0, sizeof(buffer));
  if (insideBLEscanFunc) {
    //to be implemented.
    //display discovered BLE devices
    //    Serial.println("Timestamp Addr              Rssi Data");
    //
    //  Serial.printf("%09d ", millis());
    //
    //  // MAC is in little endian --> print reverse
    //  Serial.printBufferReverse(report->peer_addr.addr, 6, ':');
    //  Serial.print(" ");
    //
    //  Serial.print(report->rssi);
    //  Serial.print("  ");
    //
    //  Serial.printBuffer(report->data.p_data, report->data.len, '-');
    //  Serial.println();
    //
    //  // Check if advertising contain BleUart service
    //  if ( Bluefruit.Scanner.checkReportForUuid(report, BLEUART_UUID_SERVICE) )
    //  {
    //    Serial.println("                       BLE UART service detected");
    //  }
    //
    //  Serial.println();
    //
    //  // For Softdevice v6: after received a report, scanner will be paused
    //  // We need to call Scanner resume() to continue scanning
    //  Bluefruit.Scanner.resume();

    display.setTextSize(1);
    display.setCursor(0, 30);
    display.println("BLE UART found");
    display.printBufferReverse(report->peer_addr.addr, 6, ':');
    display.println();
    display.printf("%0s %d dBm\n", "RSSI", report->rssi);
    display.println();

      /* Complete Local Name */
  if(Bluefruit.Scanner.parseReportByType(report, BLE_GAP_AD_TYPE_COMPLETE_LOCAL_NAME, buffer, sizeof(buffer)))
  {
    display.printf("%0s %s\n", "NAME", buffer);
    memset(buffer, 0, sizeof(buffer));
  }
    display.refresh();


    // Since we configure the scanner with filterUuid()
    // Scan callback only invoked for device with bleuart service advertised
    // Connect to the device with bleuart service in advertising packet
    Bluefruit.Central.connect(report);



  }
}

void cent_connect_callback(uint16_t conn_handle)
{
  centralConnected = true;
  if (!insideMenue && !booting) mainScreen();
  // Get the reference to current connection
  BLEConnection* connection = Bluefruit.Connection(conn_handle);

  char peer_name[32] = { 0 };
  connection->getPeerName(peer_name, sizeof(peer_name));

  Serial.print("[Cent] Connected to ");
  Serial.println(peer_name);;

  if ( clientUart.discover(conn_handle) )
  {
    // Enable TXD's notify
    clientUart.enableTXD();
  } else
  {
    // disconnect since we couldn't find bleuart service
    Bluefruit.disconnect(conn_handle);
  }
}

void cent_disconnect_callback(uint16_t conn_handle, uint8_t reason)
{
  centralConnected = false;
  if (!insideMenue && !booting) mainScreen();
  (void) conn_handle;
  (void) reason;

  Serial.println("[Cent] Disconnected");
}

/**
   Callback invoked when uart received data
   @param cent_uart Reference object to the service where the data
   arrived. In this example it is clientUart
*/
void cent_bleuart_rx_callback(BLEClientUart& cent_uart)
{
  char str[20 + 1] = { 0 };
  cent_uart.read(str, 20);

  Serial.print("[Cent] RX: ");
  Serial.println(str);
  String centInString = str;
  if (centInString.indexOf("AT+SE=") >= 0) {
    //****************************for the BLE Sensors app****************************
    if (insideBLEsensors) {
      display.fillRect(0, 25, 176, 176, LCD_COLOR_BLACK); //clear text field
      display.setTextSize(2);
      display.setTextColor(LCD_COLOR_CYAN);
      display.setCursor(0, 25);
      display.println(centInString.substring(6));
      display.refresh();
    }

  }

  if ( bleuart.notifyEnabled() )
  {
    // Forward data from our peripheral to Mobile
    bleuart.print( str );
  } else
  {
    // response with no prph message
    clientUart.println("[Cent] Peripheral role not connected");
  }
}
