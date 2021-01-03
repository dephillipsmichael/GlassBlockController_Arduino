/*
  The Glass block controller is a BLE peripheral driven animation
  stat machine for controlling custom LED matrixes.

  This particular code does the setup and run loop for the app.
  It coordinates BLE messages with operations on the LEDs
  and the LED animation attributes.
*/

#include <FastLED.h>

FASTLED_USING_NAMESPACE

/**
 * BPM (beats per minute) controls the refresh rate of the LEDs
 * so that the screen can be reactive as possible to musical beats.
 */
uint16_t animBpm = 120; // Fast refresh rate
unsigned long bpmStartTimeMicros = 0;
unsigned long bpmStartTimeOffsetMicros = 0;
unsigned long bleDelayMicros = 0;

void setBpm(uint16_t newBpm, unsigned long startTimeMicros) {
  animBpm = newBpm;
  bpmStartTimeMicros = startTimeMicros;
  bpmStartTimeOffsetMicros = bpmStartTimeMicros - bleDelayMicros;
  Serial.print("New start time ");
  Serial.print(bpmStartTimeMicros);
  Serial.print(", New offset micros ");
  Serial.print(bpmStartTimeOffsetMicros);
  Serial.println();
}
void setBpmDelay(uint8_t bleDelay) {
  bleDelayMicros = bleDelay * 1000;
  bpmStartTimeOffsetMicros = bpmStartTimeMicros - bleDelayMicros;
  Serial.print("New offset micros ");
  Serial.print(bpmStartTimeOffsetMicros);
  Serial.println();
}
void addFastLEDShowDelay(uint16_t delayMicros) {
  bpmStartTimeOffsetMicros -= delayMicros; 
}

uint16_t last24thBeat = 0;
uint16_t current24thBeat = 0;
unsigned long lastBeatTime = 0;
unsigned long nowLoop = micros();

uint16_t beatCtr = 0;

void setup() {
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

  // Start with a low brightness Rainbow Row Animation
  setGlobalBrightness(100);
  processArgb(1, 33, 99, 133);
}

void loop() {
  nowLoop = micros();

  current24thBeat = quantizeBeatWithinMeasureTruncate(24, 4, nowLoop, animBpm, bpmStartTimeOffsetMicros);
  if (last24thBeat != current24thBeat) {  

//    Serial.print("b = ");
//    Serial.print(current24thBeat);
//    Serial.print(" t = ");
//    Serial.print(nowLoop);
//    Serial.println();
    
    // Glass Block LED Matrix Animations
    
    animationLoop(current24thBeat); 
   
    if (current24thBeat == 0) {      
//      Serial.print("BPM ");
//      Serial.println(animBpm);
//      Serial.print("StartTime ");
//      Serial.println(bpmStartTimeOffsetMicros);
//      for (int i = 0; i < 96; i++) {
//        Serial.print(debugCaptureMicros[i]);
//        Serial.print(", ");
//        debugCaptureMicros[i] = 0;
//      }      
//      Serial.println();
//      for (int i = 0; i < 96; i++) {
//        Serial.print(debugCaptureBeats[i]);
//        Serial.print(", ");
//        debugCaptureBeats[i] = 0;
//      }      
//      Serial.println();
    }

    // Debugging
//    Serial.println(nowLoop - lastBeatTime);
    lastBeatTime = nowLoop;
  }
  last24thBeat = current24thBeat;

  // Check for BLE messages
  loopBluetooth();
}
