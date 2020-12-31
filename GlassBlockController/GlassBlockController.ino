/*
  The Glass block controller is a BLE peripheral driven animation
  stat machine for controlling custom LED matrixes.

  This particular code does the setup and run loop for the app.
  It coordinates BLE messages with operations on the LEDs
  and the LED animation attributes.
*/

#if defined(FASTLED_VERSION) && (FASTLED_VERSION < 3001000)
#warning "Requires FastLED 3.1 or later; check github for latest code."
#endif

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

  // Start with Rainbow Row Animation
  processArgb(1, 33, 99, 133);
}

void loop() {
  
  // Glass Block LED Matrix Animations
  animationLoop();  

  // Check for BLE messages
  loopBluetooth();   
}
