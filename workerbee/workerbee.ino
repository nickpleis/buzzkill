#include <WS2812FX.h>
#include <bluefruit.h>

#include "patterns.h"

#define LED_COUNT 24
#define LED_PIN 16
#define DATA_BUFFER_LENGTH 256

#define BUTTON_PIN 11

#ifndef WORKERBEE_NAME
  #define WORKERBEE_NAME "Worker Bee"
#endif

WS2812FX ws2812fx = WS2812FX(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

BLEDis  bledis;
BLEUart bleuart;
BLEBas  blebas;

uint8_t lightMode = 0;
char dataBuffer[DATA_BUFFER_LENGTH];
uint16_t dataOffset = 0;

void setup() {
  Serial.begin(115200);
  Serial.print("Buzzkill Worker Bee: ");
  Serial.println(WORKERBEE_NAME);
  Serial.println("--------------------------------");

  reset();

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

  reset();

  Serial.println();
  Serial.println("Disconnected");
}

void sendResponse(char const *response) {
    Serial.printf("Send Response: %s\n", response);
    bleuart.write(response, strlen(response)*sizeof(char));
}

void reset() {
  memset(dataBuffer, 0, DATA_BUFFER_LENGTH);
  dataOffset = 0;
}

void runCommand() {
  const char* pattern = (const char*)(dataBuffer);
  Serial.print("PATTERN:");
  Serial.println(pattern);

  uint8_t mode = modeForPattern(pattern);
  Serial.print("MODE: ");
  Serial.println(mode);

  ws2812fx.stop();
  ws2812fx.setMode(mode);
  ws2812fx.start();

  reset();
}

void readAndDispatchFromBluetooth() {
  if ( Bluefruit.connected() && bleuart.notifyEnabled() )
  {
    while ( bleuart.available() )
    {
      //The idea here is to read one byte at a time until we hit  a 32 bit null
      //terminator. Then we can dispatch the command to do stuff.
      bleuart.read(dataBuffer + dataOffset, 1);
      Serial.print("DATA: ");
      Serial.println((const char*)dataBuffer);

      const char lastChar = (const char)(*(dataBuffer + dataOffset));
      Serial.print("LAST CHAR: ");
      Serial.println(lastChar);

      dataOffset++;
      Serial.print("DATA OFFSET: ");
      Serial.println(dataOffset);

      if (lastChar == 0) {
        Serial.println("FOUND TERMINATOR");
        runCommand();
      }
    }
  }
}

void loop() {
  readAndDispatchFromBluetooth();
  ws2812fx.service();
  waitForEvent();
}



