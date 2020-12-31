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

//#define SPEED_TEST 1

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

#ifdef SPEED_TEST 
  unsigned long bleStrt = micros();
  unsigned long bleSum = 0;  
  unsigned long animLoopStr = micros();
  unsigned long animSum = 0;
  int speedCount = 0;
  int animCount = 0;
#endif

void loop() {

#ifdef SPEED_TEST   
  bleStrt = micros();
  speedCount++;
#endif

  // Check for BLE messages
  loopBluetooth();  

#ifdef SPEED_TEST 
  bleSum += (micros() - bleStrt);
#endif

#ifdef SPEED_TEST 
  animLoopStr = micros();
#endif

  // Glass Block LED Matrix Animations
  animationLoop();  

#ifdef SPEED_TEST 
  animSum += (micros() - animLoopStr);

  if (speedCount > 100) {
    Serial.print("BLE avg = ");
    Serial.println((float)bleSum/speedCount);
    bleSum = 0;       
    Serial.print("ANIM avg = ");
    Serial.println((float)animSum/speedCount);
    animSum = 0;  
    speedCount = 0;
  }
#endif
  
}
