/*
  Button LED

  This example creates a BLE peripheral with service that contains a
  characteristic to control an LED and another characteristic that
  represents the state of the button.

  The circuit:
  - Arduino MKR WiFi 1010, Arduino Uno WiFi Rev2 board, Arduino Nano 33 IoT,
    Arduino Nano 33 BLE, or Arduino Nano 33 BLE Sense board.
  - Button connected to pin 4

  You can use a generic BLE central app, like LightBlue (iOS and Android) or
  nRF Connect (Android), to interact with the services and characteristics
  created in this sketch.

  This example code is in the public domain.
*/

#include <ArduinoBLE.h>
#include <FastLED.h>

// FastLED "100-lines-of-code" demo reel, showing just a few 
// of the kinds of animation patterns you can quickly and easily 
// compose using FastLED.  
//
// This example also shows one easy way to define multiple 
// animations patterns and have them automatically rotate.
//
// -Mark Kriegsman, December 2014

#if defined(FASTLED_VERSION) && (FASTLED_VERSION < 3001000)
#warning "Requires FastLED 3.1 or later; check github for latest code."
#endif

#define DATA_PIN    5
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB
#define NUM_LEDS    350
CRGB leds[NUM_LEDS];

uint8_t globalBrightness = 100;
// This is a limiter to not draw too much power from the system
#define MAX_BRIGHTNESS 200

// LED srevice for controlling the glass block wall
BLEService ledService("6e400001-b5a3-f393-e0a9-e50e24dcca9e"); // create service

// Sets all LEDs to this HSV value immediately upon write
BLECharacteristic argbChar =  BLECharacteristic("6e400002-b5a3-f393-e0a9-e50e24dcca9e", BLERead | BLEWrite | BLENotify, 4);

// Receive updates about when low, mid, or high beats are detected from a device recording sound
BLECharacteristic lowMidHighChar =  BLECharacteristic("6e400003-b5a3-f393-e0a9-e50e24dcca9e", BLERead | BLEWrite | BLENotify, 3);

// Receive updates 9 bands of spectrum analysis, normalized to block row
BLECharacteristic equalizerChar =  BLECharacteristic("6e400004-b5a3-f393-e0a9-e50e24dcca9e", BLERead | BLEWrite | BLENotify, 4);

// Receive updates from 9 bands without bitwise shifting to shorten the message
BLECharacteristic equalizerLongChar =  BLECharacteristic("6e400005-b5a3-f393-e0a9-e50e24dcca9e", BLERead | BLEWrite | BLENotify, 9);

uint8_t currentFunction = 0;

byte* argb = new byte[4];
byte* lowMidHigh = new byte[3];
int LOW_IDX = 0, MID_IDX = 1, HIGH_IDX = 2;

byte* eq = new byte[4];
byte* unpackedEqVals = new byte[9];

byte functionVal = 0;
byte FUNCTION_OFF = 0;
byte FUNCTION_RAINBOW = 1;
byte FUNCTION_RAINBOW_ROW = 2;

int* avgLowMidHigh = new int[3];

float gHue = 0.0; // rotating "base color" used by many of the patterns
float speedHue = 2.0; // rotating speed

float row0Hue = 0.0; // rotating "base color" used by many of the patterns
float row1Hue = 50.0; // rotating "base color" used by many of the patterns
float row2Hue = 100.0; // rotating "base color" used by many of the patterns
float row3Hue = 150.0; // rotating "base color" used by many of the patterns
float row4Hue = 200.0; // rotating "base color" used by many of the patterns

//float row0Hue = 24.0; // rotating "base color" used by many of the patterns
//float row1Hue = 36.0; // rotating "base color" used by many of the patterns
//float row2Hue = 48.0; // rotating "base color" used by many of the patterns
//float row3Hue = 36.0; // rotating "base color" used by many of the patterns
//float row4Hue = 24.0; // rotating "base color" used by many of the patterns

float row0HueSpeed = 2.0; // rotating "base color" used by many of the patterns
float row1HueSpeed = 2.0; // rotating "base color" used by many of the patterns
float row2HueSpeed = 2.0; // rotating "base color" used by many of the patterns
float row3HueSpeed = 2.0; // rotating "base color" used by many of the patterns
float row4HueSpeed = 2.0; // rotating "base color" used by many of the patterns

uint8_t fps = 60;
float fpsMillis = 12.5;
uint8_t fpsCtr = 0;
unsigned long lastFrameMillis = millis();

unsigned long debugCtr = 0;
unsigned long debugLastClock = millis();

CRGB colorNautical = CRGB(0, 255, 198);
CRGB colorRaspberrySorbet = CRGB(255, 0, 127);
CRGB colorWhite = CRGB(255, 255, 255);
CRGB colorLavender = CRGB(158, 0, 255);
CRGB colorYellow = CRGB(255, 194, 9);

byte functionAlpha = 1;

byte rainbowRed = 33;
byte rainbowGreen = 99;
byte rainbowBlue = 133;

byte rainbowRowRed = 33;
byte rainbowRowGreen = 99;
byte rainbowRowBlue = 134;

#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

void blePeripheralConnectHandler(BLEDevice central);
void blePeripheralDisconnectHandler(BLEDevice central);

// Glass block buffer
int* blockLeds = new int[maxLedsPerBlock];
void lightBlock(uint8_t row, uint8_t col, uint8_t hue, uint8_t val) {
  to_led_idx_set(row, col, blockLeds);
  for (int led = 0; led < maxLedsPerBlock; led++) {
    if (blockLeds[led] != IGNORE_LED) {  // Black out LED in block
      leds[blockLeds[led]] = CHSV(hue, 255, val);
    }
  }
}

// Glass block buffer
void lightBlockRGB(uint8_t row, uint8_t col, uint8_t red, uint8_t green, uint8_t blue) {
  to_led_idx_set(row, col, blockLeds);
  for (int led = 0; led < maxLedsPerBlock; led++) {
    if (blockLeds[led] != IGNORE_LED) {  // Black out LED in block
      leds[blockLeds[led]] = CRGB(red, green, blue);
    }
  }
}

void setup() {
  Serial.begin(9600);
  while (!Serial);

  setupBluetooth();

  delay(3000); // 3 second delay for recovery
  setupFastLed();
}

int globalBrightnessScaled() {
  return ((float)globalBrightness / 255.0) * MAX_BRIGHTNESS;
}

void setupFastLed() {
  // tell FastLED about the LED strip configuration
  FastLED.addLeds<LED_TYPE,DATA_PIN,COLOR_ORDER>(leds, NUM_LEDS);
  //FastLED.addLeds<LED_TYPE,DATA_PIN,CLK_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);

  // set master brightness control
  FastLED.setBrightness(globalBrightnessScaled());
  
  FastLED.clear();
  FastLED.show(); 
}

void setupBluetooth() {
  // begin initialization
  if (!BLE.begin()) {
    Serial.println("starting BLE failed!");
    // TODO: mdephillips 12/15/20 flashing red LED instead?
    while (1);
  }

  // set the local name peripheral advertises
  BLE.setLocalName("Glass Block Bar Controller");

  // assign event handlers for connected, disconnected to peripheral
  BLE.setEventHandler(BLEConnected, blePeripheralConnectHandler);
  BLE.setEventHandler(BLEDisconnected, blePeripheralDisconnectHandler);
  
  // set the UUID for the service this peripheral advertises:
  BLE.setAdvertisedService(ledService);

  // add the characteristics to the service
  ledService.addCharacteristic(argbChar);
  ledService.addCharacteristic(lowMidHighChar);
  ledService.addCharacteristic(equalizerChar);
  ledService.addCharacteristic(equalizerLongChar); 

  // add the service
  BLE.addService(ledService);

  // start advertising
  BLE.advertise();

  Serial.println("Bluetooth device active, waiting for connections...");
}

void loop() {
  loopBluetooth();
  
  if (functionVal > 0) {
    doFunction();
  }
}

void loopBluetooth() {
  BLE.poll();

  if (argbChar.written()) {
    argbChar.readValue(argb, 4);
    processArgb();
  }

  if (lowMidHighChar.written()) {
    functionVal = FUNCTION_OFF;
    lowMidHighChar.readValue(lowMidHigh, 3);    
    processLowMidHigh();
  }

  if (equalizerChar.written()) {
    functionVal = FUNCTION_OFF;
    // Equalizer vals need bitwise operations to unpack
    equalizerChar.readValue(eq, 4);    
    processBitwiseEqualizer();
  }

  if (equalizerLongChar.written()) {
    functionVal = FUNCTION_OFF;
    // Equalizer vals need bitwise operations to unpack
    equalizerLongChar.readValue(unpackedEqVals, 9);    
    showUnpackedEqValues();
  }
}

void doFunction() {
  if (functionVal == FUNCTION_RAINBOW) {
    fastLedRainbow();
  } else if (functionVal == FUNCTION_RAINBOW_ROW) {
    rainbowRow();
  }
}

void processArgb() {
  Serial.print(argb[0]);
  Serial.print(", ");
  Serial.print(argb[1]);
  Serial.print(", ");
  Serial.print(argb[2]);  
  Serial.print(", ");
  Serial.print(argb[3]);    
  Serial.println();

  if (functionAlpha == argb[0] &&
       rainbowRed == argb[1] &&
       rainbowGreen == argb[2] &&
       rainbowBlue == argb[3]) {

      functionVal = FUNCTION_RAINBOW;        
      return;      
  }

  if (functionAlpha == argb[0] &&
       rainbowRowRed == argb[1] &&
       rainbowRowGreen == argb[2] &&
       rainbowRowBlue == argb[3]) {

      functionVal = FUNCTION_RAINBOW_ROW;        
      return;      
  }

  // Hue 
  if (1 == argb[0] && 0 == argb[1] && 0 == argb[2]) {  
    speedHue = (float)((uint8_t)argb[3]) * 0.25;
    row0HueSpeed = speedHue;
    row1HueSpeed = speedHue;
    row2HueSpeed = speedHue;
    row3HueSpeed = speedHue;
    row4HueSpeed = speedHue;
    return;
  }

  // Make sure the brightness is relative to the max in this code
  globalBrightness = argb[0];
  FastLED.setBrightness(globalBrightnessScaled());

  // Sending black will just control the global brightness 
  if (0 == argb[1] && 0 == argb[2] && 0 == argb[3]) {            
      return;      
  }

  functionVal = FUNCTION_OFF;
  fill_solid(leds, NUM_LEDS, CRGB(argb[1], argb[2], argb[3]));
  FastLED.show();
}

void processLowMidHigh() {
  // Too frequent to always log
//    Serial.print(lowMidHigh[0]);
//    Serial.print(", ");
//    Serial.print(lowMidHigh[1]);
//    Serial.print(", ");
//    Serial.print(lowMidHigh[2]);  
//    Serial.print(", ");
//    Serial.print(lowMidHigh[3]);    
//    Serial.println();

  for (int i = 0; i < 140; i++) {
    if (lowMidHigh[LOW_IDX] > 0) {
      leds[i] = colorNautical;
    } else {
      leds[i] = CRGB::Black;
    }
  }

  for (int i = 140; i < 210; i++) {
    if (lowMidHigh[MID_IDX] > 0) {
      leds[i] = colorRaspberrySorbet;
    } else {
      leds[i] = CRGB::Black;
    }
  }

  for (int i = 210; i < 350; i++) {      
    if (lowMidHigh[HIGH_IDX] > 0) {
      leds[i] = colorWhite;
    } else {
      leds[i] = CRGB::Black;
    }
  }
  
  FastLED.show();
}

void rainbowTestLoop() {
  unsigned long currentMillis = millis();

  debugCtr++;
  if (currentMillis - debugLastClock > 1000) {
    Serial.println(debugCtr);
    debugCtr = 0;
    debugLastClock = currentMillis;
  }

  // Check clock time for animation
  fpsCtr++;
  if ((currentMillis - lastFrameMillis) > fpsMillis) {
    // Do a frame of LED animation    
    fastLedRainbow();
    lastFrameMillis = currentMillis;    
  }
}

uint8_t hues[blocksPerRow] = { 0, 21, 42, 63, 84, 105, 126, 147, 168, 189, 210, 231 };

void processBitwiseEqualizer() {

//  Serial.print(eq[0]);
//  Serial.print(", ");
//  Serial.print(eq[1]);
//  Serial.print(", ");
//  Serial.print(eq[2]);
//  Serial.print(", ");
//  Serial.println(eq[3]); 

   byte eqMask = 0x07; // 111

  // Unpack the binary representations of 9 numbers [0..4] with 4 bytes
  unpackedEqVals[0] = (eq[0] >> 5) & eqMask;
  unpackedEqVals[1] = (eq[0] >> 2) & eqMask;
  unpackedEqVals[2] = ((eq[0] << 1) & eqMask) | ((eq[1] >> 7) & eqMask);
  unpackedEqVals[3] = (eq[1] >> 4) & eqMask;
  unpackedEqVals[4] = (eq[1] >> 1) & eqMask;
  unpackedEqVals[5] = ((eq[1] << 2) & eqMask) | ((eq[2] >> 6) & eqMask);
  unpackedEqVals[6] = (eq[2] >> 3) & eqMask;
  unpackedEqVals[7] = eq[2] & eqMask;
  unpackedEqVals[8] = eq[3] & eqMask;

//for (int i = 0; i < 9; i++) {
//  Serial.print(unpackedEqVals[i]);
//  Serial.print(", ");
//}
//  Serial.println(); 
  

  //gHue+=12;
  // FastLED's built-in rainbow generator
  //fill_rainbow( leds, NUM_LEDS, gHue, 7);

  // Too much to always output
//  for (int i = 0; i < 12; i++) {
//    Serial.print(equalizerVals[0]);
//    Serial.print(", ");
//  }
//  Serial.println();

  showUnpackedEqValues();
}

void showUnpackedEqValues() {  
  
  avgLowMidHigh[0] = max(max(unpackedEqVals[0], unpackedEqVals[1]), unpackedEqVals[2]);
  avgLowMidHigh[1] = max(max(unpackedEqVals[3], unpackedEqVals[4]), unpackedEqVals[5]);
  avgLowMidHigh[2] = max(max(unpackedEqVals[6], unpackedEqVals[7]), unpackedEqVals[8]);
  
  for (int col = 0; col < 3; col++) {   
    for (int row = 0; row < blocksRowCount; row++) {
      uint8_t rowAdjusted = (ledsRowCount - 1) - row;
      if (avgLowMidHigh[col] > row) {
        if (rowAdjusted == 4) {
           lightBlockRGB(rowAdjusted, col, 158, 0, 255);
        } else if (rowAdjusted == 3) {
           lightBlockRGB(rowAdjusted, col, 0, 255, 198);
        } else if (rowAdjusted == 2) {
           lightBlockRGB(rowAdjusted, col, 255, 194, 9);
        } else if (rowAdjusted == 1) {
           lightBlockRGB(rowAdjusted, col, 255, 0, 127);
        } else if (rowAdjusted == 0) {
           lightBlockRGB(rowAdjusted, col, 255, 255, 255);
        }        
      } else {
        lightBlockRGB(rowAdjusted, col, 50, 50, 50);
      }
    }
  }

  for (int col = 3; col < blocksPerRow; col++) {
    for (int row = 0; row < blocksRowCount; row++) {
      uint8_t rowAdjusted = (ledsRowCount - 1) - row;
      if (unpackedEqVals[col - 3] > row) {
        if (rowAdjusted == 4) {
           lightBlockRGB(rowAdjusted, col, 158, 0, 255);
        } else if (rowAdjusted == 3) {
           lightBlockRGB(rowAdjusted, col, 0, 255, 198);
        } else if (rowAdjusted == 2) {
           lightBlockRGB(rowAdjusted, col, 255, 194, 9);
        } else if (rowAdjusted == 1) {
           lightBlockRGB(rowAdjusted, col, 255, 0, 127);
        } else if (rowAdjusted == 0) {
           lightBlockRGB(rowAdjusted, col, 255, 255, 255);
        }       
      } else {
        lightBlockRGB(rowAdjusted, col, 50, 50, 50);
      }
    }
  }
  
  FastLED.show(); 
}

void fastLedRainbow () {
  gHue+=speedHue;
  if (gHue > 255) {
    gHue = gHue - 255;
  }
  // FastLED's built-in rainbow generator
  fill_rainbow( leds, NUM_LEDS, (byte)gHue, 7);
  
  // send the 'leds' array out to the actual LED strip
  FastLED.show(); 
}

void rainbowRow() {

  row0Hue += row0HueSpeed;
  if (row0Hue > 255) {
    row0Hue = row0Hue - 255;
  }

  row1Hue += row1HueSpeed;
  if (row1Hue > 255) {
    row1Hue = row1Hue - 255;
  }

  row2Hue += row2HueSpeed;
  if (row2Hue > 255) {
    row2Hue = row2Hue - 255;
  }

  row3Hue += row3HueSpeed;
  if (row3Hue > 255) {
    row3Hue = row3Hue - 255;
  }

  row4Hue += row4HueSpeed;
  if (row4Hue > 255) {
    row4Hue = row4Hue - 255;
  }

  uint8_t hue0 = (uint8_t)row0Hue, hue1 = (uint8_t)row1Hue, hue2 = (uint8_t)row2Hue, hue3 = (uint8_t)row3Hue, hue4 = (uint8_t)row4Hue;

  for (int i = 0; i < 70; i++) {
    hue0 += row0HueSpeed;
    leds[i] = CHSV(hue0, 255, globalBrightnessScaled());
  }

  for (int i = 139; i >= 70; i--) {
    hue1 += row1HueSpeed;
    leds[i] = CHSV(hue1, 255, globalBrightnessScaled());
  }

  for (int i = 140; i < 210; i++) {
    hue2 += row2HueSpeed;
    leds[i] = CHSV(hue2, 255, globalBrightnessScaled());
  }

  for (int i = 279; i >= 210; i--) {
    hue3 += row3HueSpeed;
    leds[i] = CHSV(hue3, 255, globalBrightnessScaled());
  }

  for (int i = 280; i < 350; i++) {
    hue4 += row4HueSpeed;
    leds[i] = CHSV(hue4, 255, globalBrightnessScaled());
  }

//  // FastLED's built-in rainbow generator
//  fill_rainbow( leds, NUM_LEDS, (byte)gHue, 7);
  
  // send the 'leds' array out to the actual LED strip
  FastLED.show(); 
}

void blePeripheralConnectHandler(BLEDevice central) {
  // central connected event handler
  Serial.print("Connected event, central: ");
  Serial.println(central.address());
}

void blePeripheralDisconnectHandler(BLEDevice central) {
  // central disconnected event handler
  Serial.print("Disconnected event, central: ");
  Serial.println(central.address());
}
