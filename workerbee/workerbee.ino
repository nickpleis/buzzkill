#include <WS2812FX.h>
#include <bluefruit.h>

#include "patterns.h"

#define LED_COUNT 102
#define LED_PIN 16
#define DATA_BUFFER_LENGTH 256
#define MAX_SPEED 65535

#define BUTTON_PIN 11

#ifndef WORKERBEE_NAME
  #define WORKERBEE_NAME "Test Workerbee"
#endif

WS2812FX ws2812fx = WS2812FX(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

BLEDis  bledis;
BLEUart bleuart;
BLEBas  blebas;

// The app sends a string containing all of the information we need to execute a pattern.
// The string is formatted as PATTERN,SPEED,BRIGHTNESS,R,G,B
// PATTERN: 4 character code indicating the pattern to display
// SPEED: Number from 1-100 giving the speed
// BRIGHTNESS: Number from 1-100 giving the brightness
// R,G,B: Numbers giving the single byte value for Red, Green, and Blue 

struct CmdParams {
  CmdParams() { reset(); }

  void reset() {
    mPattern = FX_MODE_THEATER_CHASE_RAINBOW;
    mSpeed = 50;
    mBrightness = 50;
    mRed = 255;
    mGreen = 0;
    mBlue = 0;
    mExecuteAt = 0;
  }
  
  String mPattern;
  int mSpeed;
  int mBrightness;
  unsigned char mRed;
  unsigned char mBlue;
  unsigned char mGreen;
  int mExecuteAt;
};

uint8_t lightMode = 0;
char dataBuffer[DATA_BUFFER_LENGTH];
uint16_t dataOffset = 0;

CmdParams queuedCmd;
unsigned long referenceTime = 0;
bool commandQueued = false;

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

//  char central_name[32] = { 0 };
//  Bluefruit.Gap.getPeerName(conn_handle, central_name, sizeof(central_name));
//
//  Serial.print("Connected to ");
//  Serial.println(central_name);

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
  queuedCmd.reset();
  commandQueued = false;
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

  char* rawExecuteAt = strtok(NULL, ",");
  if (rawExecuteAt != NULL)
  {
    String strExecuteAt = rawExecuteAt;
    params.mExecuteAt = strExecuteAt.toInt();
  }
}

void runCommand()
{
  Serial.print("PATTERN:");
  Serial.println(queuedCmd.mPattern);

  uint8_t mode = modeForPattern(queuedCmd.mPattern);
  Serial.print("MODE: ");
  Serial.println(mode);

  Serial.print("SPEED: ");
  Serial.println(queuedCmd.mSpeed);

  Serial.print("BRIGHTNESS: ");
  Serial.println(queuedCmd.mBrightness);

  Serial.print("RED: ");
  Serial.println(queuedCmd.mRed);

  Serial.print("GREEN: ");
  Serial.println(queuedCmd.mGreen);

  Serial.print("BLUE: ");
  Serial.println(queuedCmd.mBlue);

  // Speed in ws218fx is defined roughly as an int from 0-MAX_SPEED with MAX_SPEED being the SLOWEST.
  // So we normalize our 1-100 to that by calculating what percentage of MAX_SPEED we are, but
  // you know..reversed
  float normalizedSpeed = (float)queuedCmd.mSpeed/100;
  normalizedSpeed = 1 - normalizedSpeed;
  normalizedSpeed *= MAX_SPEED;

  Serial.print("NORMALIZED SPEED: ");
  Serial.println(normalizedSpeed);

  if (mode == CUSTOM_MODE_MAX)
  {
    mode = FX_MODE_STATIC;
    queuedCmd.mRed = 255;
    queuedCmd.mGreen = 255;
    queuedCmd.mBlue = 255;
    queuedCmd.mBrightness = 100;
  } else if (mode == CUSTOM_MODE_OFF) {
    mode = FX_MODE_STATIC;
    queuedCmd.mRed = 0;
    queuedCmd.mGreen = 0;
    queuedCmd.mBlue = 0;
    queuedCmd.mBrightness = 0;
  }

  Serial.print("Execute At: ");
  Serial.println(queuedCmd.mExecuteAt);
 
  ws2812fx.stop();
  ws2812fx.setMode(mode);
  ws2812fx.setSpeed(normalizedSpeed);
  ws2812fx.setBrightness(queuedCmd.mBrightness);
  ws2812fx.setColor(queuedCmd.mRed, queuedCmd.mGreen, queuedCmd.mBlue);
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
      bleuart.read(dataBuffer + dataOffset, 1);
      const char lastChar = (const char)(*(dataBuffer + dataOffset));
    
      dataOffset++;
      
      if (lastChar == 0) {
        Serial.println("Parsing Parameters");
        parseParams(queuedCmd, (char*)dataBuffer);
        referenceTime = millis();
        commandQueued = true;
      }
    }
  }
}

void loop()
{
  readAndDispatchFromBluetooth();
  ws2812fx.service();

  if (commandQueued)
  {
    Serial.print("Command queued for: ");
    Serial.println(queuedCmd.mExecuteAt);

    unsigned long currentTime = millis();
    Serial.print("Current time: ");
    Serial.println(currentTime);

    Serial.print("Reference time: ");
    Serial.println(referenceTime);

    Serial.print("MS since command received: ");
    Serial.println((currentTime - referenceTime));

    if ((currentTime - referenceTime) > queuedCmd.mExecuteAt)
    {
      runCommand();
    }
  }

  waitForEvent();
}



