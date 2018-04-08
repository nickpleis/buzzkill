#include <WS2812FX.h>
#include <bluefruit.h>

#define LED_COUNT 24
#define LED_PIN 16

#ifndef WORKERBEE_NAME
  #define WORKERBEE_NAME "Worker Bee"
#endif

WS2812FX ws2812fx = WS2812FX(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

BLEDis  bledis;
BLEUart bleuart;
BLEBas  blebas;

void setup() {
  Serial.begin(115200);
  Serial.println("Buzzkill Worker Bee");
  Serial.println(`);
  Serial.println("--------------------------------");

  setupLights();
  Serial.println("Lights initialized and ready to go...");

  setupBluetooth();
  Serial.println("Bluetooth initialized, waiting for central....");
}

void setupLights() {
  ws2812fx.init();
  ws2812fx.setBrightness(100);
  ws2812fx.setSpeed(10000);
  ws2812fx.setMode(FX_MODE_THEATER_CHASE_RAINBOW);
  ws2812fx.start();
}

void setupBluetooth() {
  Bluefruit.configPrphBandwidth(BANDWIDTH_MAX);
  Bluefruit.begin();

  // Set max power. Accepted values are: -40, -30, -20, -16, -12, -8, -4, 0, 4
  Bluefruit.setTxPower(4);
  Bluefruit.setName(WORKERBEE_NAME);
  Bluefruit.setConnectCallback(connect_callback);
  Bluefruit.setDisconnectCallback(disconnect_callback);
  
  // Configure and Start Device Information Service
  bledis.setManufacturer("Buzkillians");
  bledis.setModel("Bluefruit Feather52");
  bledis.begin();  

  // Configure and start BLE UART service
  bleuart.begin();

  // Start BLE Battery Service
  blebas.begin();
  blebas.write(100);

  // Set up and start advertising
  startAdv();
}

void startAdv()
{
  Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
  Bluefruit.Advertising.addTxPower();
 
  // Include bleuart 128-bit uuid
  Bluefruit.Advertising.addService(bleuart);
 
  // There is no room for Name in Advertising packet
  // Use Scan response for Name
  Bluefruit.ScanResponse.addName();

  /* Start Advertising
   * - Enable auto advertising if disconnected
   * - Interval:  fast mode = 20 ms, slow mode = 152.5 ms
   * - Timeout for fast mode is 30 seconds
   * - Start(timeout) with timeout = 0 will advertise forever (until connected)
   * 
   * For recommended advertising interval
   * https://developer.apple.com/library/content/qa/qa1931/_index.html   
   */
  Bluefruit.Advertising.restartOnDisconnect(true);
  Bluefruit.Advertising.setInterval(32, 244);    // in unit of 0.625 ms
  Bluefruit.Advertising.setFastTimeout(30);      // number of seconds in fast mode
  Bluefruit.Advertising.start(0);                // 0 = Don't stop advertising after n seconds  
}

void connect_callback(uint16_t conn_handle)
{
  Serial.print("Connect callback");

  char central_name[32] = { 0 };
  Bluefruit.Gap.getPeerName(conn_handle, central_name, sizeof(central_name));

  Serial.print("Connected to ");
  Serial.println(central_name);

  Serial.println("Waiting for commands...");
}

void disconnect_callback(uint16_t conn_handle, uint8_t reason)
{
  (void) conn_handle;
  (void) reason;

  Serial.println();
  Serial.println("Disconnected");
}

void loop() {
  ws2812fx.service();
  waitForEvent();
}
