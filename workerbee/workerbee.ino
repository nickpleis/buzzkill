#include <WS2812FX.h>
#include <bluefruit.h>

#define LED_COUNT 205
#define LED_PIN 16
#define DATA_BUFFER_LENGTH 9
#define MAX_SPEED 16384

#define BUTTON_PIN 11

#ifndef WORKERBEE_NAME
  #define WORKERBEE_NAME "Test Workerbee"
#endif

WS2812FX ws2812fx = WS2812FX(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

BLEDis  bledis;
BLEUart bleuart;
BLEBas  blebas;

char dataBuffer[DATA_BUFFER_LENGTH];
bool commandQueued = false;
unsigned long referenceTime = 0;

#define CUSTOM_MODE_OFF     0xFF
#define CUSTOM_MODE_MAX     0xFE
#define CUSTOM_MODE_BEE     0xFD

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
  Bluefruit.configPrphBandwidth(BANDWIDTH_LOW);
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

void reset() {
  memset(dataBuffer, 0, DATA_BUFFER_LENGTH);
  commandQueued = false;
  referenceTime = 0;
}

void runCommand()
{
  uint8_t mode = dataBuffer[0];
  Serial.print("MODE: ");
  Serial.println(mode);

  uint8_t modeSpeed = dataBuffer[1];
  Serial.print("SPEED: ");
  Serial.println(modeSpeed);

  uint8_t brightness = dataBuffer[2];
  Serial.print("BRIGHTNESS: ");
  Serial.println(brightness);

  uint8_t red = dataBuffer[3];
  Serial.print("RED: ");
  Serial.println(red);

  uint8_t green = dataBuffer[4];
  Serial.print("GREEN: ");
  Serial.println(green);

  uint8_t blue = dataBuffer[5];
  Serial.print("BLUE: ");
  Serial.println(blue);

  // Speed in ws218fx is defined roughly as an int from 0-MAX_SPEED with MAX_SPEED being the SLOWEST.
  // So we normalize our 1-100 to that by calculating what percentage of MAX_SPEED we are, but
  // you know..reversed
  float normalizedSpeed = (float)modeSpeed/100;
  normalizedSpeed = 1 - normalizedSpeed;
  normalizedSpeed *= MAX_SPEED;

  Serial.print("NORMALIZED SPEED: ");
  Serial.println(normalizedSpeed);

  if (mode == CUSTOM_MODE_MAX)
  {
    mode = FX_MODE_STATIC;
    red = 255;
    green = 255;
    blue = 255;
    brightness = 100;
  } else if (mode == CUSTOM_MODE_OFF) {
    mode = FX_MODE_STATIC;
    red = 0;
    green = 0;
    blue = 0;
    brightness = 0;
  } else if (mode == CUSTOM_MODE_BEE) {
    mode = FX_MODE_BLINK;
    red = 255;
    green = 255;
    blue = 0;
  }
 
  ws2812fx.stop();
  ws2812fx.setMode(mode);
  ws2812fx.setSpeed(normalizedSpeed);
  ws2812fx.setBrightness(brightness);
  ws2812fx.setColor(red, green, blue);
  ws2812fx.start();

  reset();
}

void readAndDispatchFromBluetooth()
{
  if ( Bluefruit.connected() && bleuart.notifyEnabled() )
  {
    while ( bleuart.available() )
    {
      //The idea here is to read one byte at a time until we hit  a 32 bit null
      //terminator. Then we can dispatch the command to do stuff.
      bleuart.read(dataBuffer, 8);
      commandQueued = true;
      referenceTime = millis();
    }
  }
}

void loop()
{
  readAndDispatchFromBluetooth();
  
  if (commandQueued)
  {
    uint16_t executeAt = dataBuffer[6];
    unsigned long currentTime = millis();
    
    Serial.print("Command queued for: ");
    Serial.println(executeAt);

    Serial.print("MS since command received: ");
    Serial.println((currentTime - referenceTime));

    if ((currentTime - referenceTime) > executeAt)
    {
      runCommand();
    }
  }

  ws2812fx.service();
  waitForEvent();
}
