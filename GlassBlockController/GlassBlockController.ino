/*
  The Glass block controller is a BLE peripheral driven animation
  stat machine for controlling custom LED matrixes.

  This particular code does the setup and run loop for the app.
  It coordinates BLE messages with operations on the LEDs
  and the LED animation attributes.
*/
// #define DEBUG_GLASS_BLOCK_CONTROLLER 1

#include <FastLED.h>

void setup() {    
  Serial.begin(9600);
  while (!Serial); 

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

void loop() {  
  // Check for run loops on active controller  
  loopControllerManager();   

  // Check for BLE messages
  loopBluetooth();
}
