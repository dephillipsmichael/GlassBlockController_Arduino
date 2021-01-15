/*
  The Glass block controller is a BLE peripheral driven animation
  stat machine for controlling custom LED matrixes.

  This particular code does the setup and run loop for the app.
  It coordinates BLE messages with operations on the LEDs
  and the LED animation attributes.
*/
#define DEBUG_GLASS_BLOCK_CONTROLLER 1

#include <Wire.h>
#include <FastLED.h>

/**
 * Use I2C to get an accurate picture
 * of the current millis()
 */
unsigned long lastMillis = 0;
/**
 * @return glassBlock_Millis from another Arduino board
 */
unsigned long glassBlock_Millis() {
  refreshI2cTimeMillis();
  return lastMillis;
}

unsigned long millisBuffer = 0;
byte millisCtr = 0;
unsigned long maxFrameTimeMillis = 0;

void setup() {
  Wire.begin();        // join i2c bus (address optional for master)
  delay(100);
  
  Serial.begin(9600);
  while (!Serial); 

  pinMode(LED_BUILTIN, OUTPUT);

  // BluetoothComm Setup
  setupBluetooth();

  // 2 second delay for BLE setup recovery
  delay(2000); 

  // Setup the LED
  setupFastLed();
  
  // 2 second delay for LED pins setup recovery
  delay(2000);     

  // Setup controller manager, defaults to color controller
  setupControllerManager();
}

unsigned long debugTimeDuration = 0;
int debugTimeCtr = 0;

void loop() { 

  #ifdef DEBUG_GLASS_BLOCK_CONTROLLER
  
    unsigned long before = lastMillis;  
    refreshI2cTimeMillis();  
    unsigned long diff = (lastMillis - before);    
    debugTimeDuration += diff;
    maxFrameTimeMillis = max(maxFrameTimeMillis, diff);
    debugTimeCtr++;
    if (debugTimeCtr > 100) {      
      if (debugTimeDuration > 100) {
        Serial.print((debugTimeDuration / 100L));
        Serial.print(", ");
        Serial.print(maxFrameTimeMillis);
        Serial.println();
      }           
      maxFrameTimeMillis = 0;
      debugTimeCtr = 0;
      debugTimeDuration = 0;
    }

  #else

    // Refresh the time in millis() from another Arduino
    // or really any I2C compatible device.
    // Takes approx. 600 microseconds
    refreshI2cTimeMillis();  
  
  #endif         

  // Check for BLE messages
  loopBluetooth();
  
  // Check for run loops on active controller
  loopControllerManager(lastMillis);   

  // Check for BLE messages
  loopBluetooth();
}

/**
 * This function grabs an accurate millis() from 
 * another Slave Arduino device.
 * 
 * The reasoning behind this is that
 * using this Arduino's millis() function
 * is inaccurate when using FastLED with WS2812B LEDs.
 * As, FastLED.show() will stop system clock interupts.
 * 
 * Setting up another Arudino gives is 
 */
void refreshI2cTimeMillis() {
  millisBuffer = 0;
  millisCtr = 4;
  
  // Grabbing the time takes approximately 0.590 milliseconds
  Wire.requestFrom(8, 4);    // request 4 bytes from slave device #8  
  while (Wire.available()) { // slave may send less than requested
    millisCtr--;  
    byte readByte = Wire.read();
    millisBuffer = millisBuffer | ((unsigned long)readByte << (8 * millisCtr));         
  }  

  // We read all 4 bytes 
  if (millisCtr == 0) {
    lastMillis = millisBuffer;
  } else {
    #ifdef DEBUG_GLASS_BLOCK_CONTROLLER
      Serial.println("Error reading millis buffer");
    #endif
  }
  
  // Because grabbing the time takes approximately 0.590 milliseconds
  // We would rather be slightly ahead for the beat analysis
  lastMillis += 1;  
}
