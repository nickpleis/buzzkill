#include <WS2812FX.h>
#include <bluefruit.h>

#include "patterns.h"

#define DATA_BUFFER_LENGTH 256

#define LED1_COUNT 24
#define LED1_PIN 15

#define LED2_COUNT 24
#define LED2_PIN 16

#ifndef WORKERBEE_NAME
  #define WORKERBEE_NAME "Worker Bee Ring 24 #2"
#endif

WS2812FX ws2812fx1 = WS2812FX(LED1_COUNT, LED1_PIN, NEO_GRB + NEO_KHZ800);
WS2812FX ws2812fx2 = WS2812FX(LED2_COUNT, LED2_PIN, NEO_GRB + NEO_KHZ800);


BLEDis  bledis;
BLEUart bleuart;
BLEBas  blebas;

uint8_t lightMode = 0;
char dataBuffer[DATA_BUFFER_LENGTH];
uint16_t dataOffset = 0;

// The app sends a string containing all of the information we need to execute a pattern.
// The string is formatted as PATTERN,SPEED,BRIGHTNESS,R,G,B
// PATTERN: 4 character code indicating the pattern to display
// SPEED: Number from 1-100 giving the speed
// BRIGHTNESS: Number from 1-100 giving the brightness
// R,G,B: Numbers giving the single byte value for Red, Green, and Blue 

struct CmdParams
{
  CmdParams()
  {
    mPattern = FX_MODE_THEATER_CHASE_RAINBOW;
    mSpeed = 50;
    mBrightness = 50;
    mRed = 255;
    mGreen = 0;
    mBlue = 0;
  }
  
  String mPattern;
  int mSpeed;
  int mBrightness;
  unsigned char mRed;
  unsigned char mBlue;
  unsigned char mGreen;
};

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
  ws2812fx1.init();
  ws2812fx2.init();

  ws2812fx1.setBrightness(100);
  ws2812fx2.setBrightness(100);

  ws2812fx1.setSpeed(10000);
  ws2812fx2.setSpeed(10000);

  ws2812fx1.setMode(FX_MODE_THEATER_CHASE_RAINBOW);
  ws2812fx2.setMode(FX_MODE_THEATER_CHASE_RAINBOW);

  ws2812fx1.start();
  ws2812fx2.start();
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

void parseParams(CmdParams& params, char* cmdString)
{
  Serial.println("STARTING PARSING");
  char* rawPattern = strtok(cmdString, ",");
  if (rawPattern != NULL)
  {
    params.mPattern = rawPattern;
    params.mPattern.toUpperCase();
  }

  char* rawSpeed = strtok(NULL, ",");
  if (rawSpeed != NULL)
  {
    String strSpeed = rawSpeed;
    params.mSpeed = (int)strSpeed.toInt();
    if (params.mSpeed < 1)
    {
      params.mSpeed = 1;
    }
    else if(params.mSpeed >= 100)
    {
      params.mSpeed = 100;
    }
  }

  char* rawBright = strtok(NULL, ",");
  if (rawBright != NULL)
  {
    String strBright = rawBright;
    params.mBrightness = (int)strBright.toInt();

    if (params.mBrightness < 0)
    {
      params.mBrightness = 0;
    }
    else if(params.mBrightness >= 100)
    {
      params.mBrightness = 100;
    }
  }

  char* rawRed = strtok(NULL, ",");
  if (rawRed != NULL)
  {
    String strRed = rawRed;
    params.mRed = (char)strRed.toInt();
  }

  char* rawGreen = strtok(NULL, ",");
  if (rawGreen != NULL)
  {
    String strGreen = rawGreen;
    params.mGreen = (char)strGreen.toInt();
  }

  char* rawBlue = strtok(NULL, ",");
  if (rawBlue != NULL)
  {
    String strBlue = rawBlue;
    params.mBlue = (char)strBlue.toInt();
  }
}

void RunCommandOnPin(WS2812FX& fx, uint8_t mode, float normalizedSpeed, const CmdParams& params)
{
  fx.stop();
  fx.setMode(mode);
  fx.setSpeed(normalizedSpeed);
  fx.setBrightness(params.mBrightness);
  fx.setColor(params.mRed, params.mGreen, params.mBlue);
  fx.start();
}

void runCommand()
{
  CmdParams params;
  parseParams(params, (char*)dataBuffer);

  Serial.print("PATTERN:");
  Serial.println(params.mPattern);

  uint8_t mode = modeForPattern(params.mPattern);
  Serial.print("MODE: ");
  Serial.println(mode);

  Serial.print("SPEED: ");
  Serial.println(params.mSpeed);

  Serial.print("BRIGHTNESS: ");
  Serial.println(params.mBrightness);

  Serial.print("RED: ");
  Serial.println(params.mRed);

  Serial.print("GREEN: ");
  Serial.println(params.mGreen);

  Serial.print("BLUE: ");
  Serial.println(params.mBlue);

  // Speed in ws218fx is defined roughly as an int from 0-3000 with 3000 being the SLOWEST.
  // So we normalize our 1-100 to that by calculating what percentage of 3000 we are, but
  // you know..reversed
  float normalizedSpeed = (float)params.mSpeed/100;
  normalizedSpeed = 1 - normalizedSpeed;
  normalizedSpeed *= 3000;

  Serial.print("NORMALIZED SPEED: ");
  Serial.println(normalizedSpeed);

  RunCommandOnPin(ws2812fx1, mode, normalizedSpeed, params);
  RunCommandOnPin(ws2812fx2, mode, normalizedSpeed, params);

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
      bleuart.read(dataBuffer + dataOffset, 1);
      const char lastChar = (const char)(*(dataBuffer + dataOffset));
      Serial.print("LAST CHAR: ");
      Serial.println(lastChar);
    
      dataOffset++;
      
      if (lastChar == 0) {
        Serial.println("FOUND TERMINATOR");
        runCommand();
      }
    }
  }
}

void loop()
{
  readAndDispatchFromBluetooth();

  ws2812fx1.service();
  ws2812fx2.service();

  waitForEvent();
}



