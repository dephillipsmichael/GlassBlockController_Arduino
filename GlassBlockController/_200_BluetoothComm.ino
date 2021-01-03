/*
  This portion of the app has only been tested with the 
  
  * Arduino Wifi Rev 2

  It may work on the nRF Feather and other BLE enabled
  Arduinos microcontrollers.
*/
#include <ArduinoBLE.h>

// LED srevice for controlling the glass block wall
BLEService ledService("6e400001-b5a3-f393-e0a9-e50e24dcca9e"); // create service

// Sets all LEDs to this HSV value immediately upon write
BLECharacteristic argbChar =  BLECharacteristic("6e400002-b5a3-f393-e0a9-e50e24dcca9e", BLERead | BLEWrite | BLENotify, 4);

// Receive updates about when low, mid, or high beats are detected from a device recording sound
BLECharacteristic lowMidHighChar =  BLECharacteristic("6e400003-b5a3-f393-e0a9-e50e24dcca9e", BLERead | BLEWrite | BLENotify, 3);

// Receive bpm and timing info for animation speed
BLECharacteristic bpmChar =  BLECharacteristic("6e400004-b5a3-f393-e0a9-e50e24dcca9e", BLERead | BLEWrite | BLENotify, 4);

// Receive updates from 9 bands without bitwise shifting to shorten the message
//BLECharacteristic equalizerLongChar =  BLECharacteristic("6e400005-b5a3-f393-e0a9-e50e24dcca9e", BLERead | BLEWrite | BLENotify, 9);

BLECharacteristic beatSequenceChar =  BLECharacteristic("6e400005-b5a3-f393-e0a9-e50e24dcca9e", BLERead | BLEWrite | BLENotify, 20);

#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

void blePeripheralConnectHandler(BLEDevice central);
void blePeripheralDisconnectHandler(BLEDevice central);

// Buffer to read ARGB bytes from BLE characteristic
byte* argb = new byte[4];
int ALPHA_IDX = 0, RED_IDX = 1, GREEN_IDX = 2, BLUE_IDX = 3;

// Buffer to read low, mid, high beats from the BLE central
byte* lowMidHigh = new byte[3];
int LOW_IDX = 0, MID_IDX = 1, HIGH_IDX = 2;

// Buffer to read bpm info
byte* bpmVals = new byte[4];

// Buffer to Equalizer 9 freq band intensity values from BLE central
byte* unpackedEqVals = new byte[9];

// Buffer to retrieve the next part of the beat sequence
byte* beatSequenceVals = new byte[20];

void setupBluetooth() {
  // begin initialization
  if (!BLE.begin()) {
    Serial.println("starting BLE failed!");
    // TODO: mdephillips 12/15/20 flashing red LED instead?
    while (1);
  }

  // Set the local name peripheral advertises
  BLE.setLocalName("Glass Block Bar Controller");

  // Assign event handlers for connected, disconnected to peripheral
  BLE.setEventHandler(BLEConnected, blePeripheralConnectHandler);
  BLE.setEventHandler(BLEDisconnected, blePeripheralDisconnectHandler);
  
  // set the UUID for the service this peripheral advertises:
  BLE.setAdvertisedService(ledService);

  // add the characteristics to the service
  ledService.addCharacteristic(argbChar);
  ledService.addCharacteristic(lowMidHighChar);
  ledService.addCharacteristic(bpmChar);
  //ledService.addCharacteristic(equalizerLongChar); 
  ledService.addCharacteristic(beatSequenceChar);   

  // add the service
  BLE.addService(ledService);

  // start advertising
  BLE.advertise();

  Serial.println("Bluetooth device active, waiting for connections...");
}

unsigned long now = 0;
void loopBluetooth() {  
  
  now = micros();
   
  // This function on avg takes only
  // ~31 MICRO seconds to perform
  // However, if there is a characteristic
  // to read, like the 9 byte EQ,
  // This takes ~1000 MICRO secodns to perform.
  BLE.poll();

  if (argbChar.written()) {
    // Alpha, red, green, blue
    argbChar.readValue(argb, 4);    
    processArgb(argb[0], argb[1], argb[2], argb[3]);
  }

  if (lowMidHighChar.written()) {
    setLedAnimationPattern(ANIMATION_OFF);
    lowMidHighChar.readValue(lowMidHigh, 3);    
    processLowMidHigh(lowMidHigh[LOW_IDX], lowMidHigh[MID_IDX], lowMidHigh[HIGH_IDX]);
  }

  if (bpmChar.written()) {
    setLedAnimationPattern(ANIMATION_BPM_TEST);
    // BPM Info 
    bpmChar.readValue(bpmVals, 4);   
    if (bpmVals[0] == 0) {  
      Serial.println("Set BPM");   
      Serial.println(bpmVals[3]);    
      setBpm((uint16_t)bpmVals[3], now);
      //processBPM(bpmVals[3]);    
    } else if (bpmVals[0] == 1) {
      Serial.println("Set BPM delay");   
      Serial.println(bpmVals[3]);    
      setBpmDelay(bpmVals[3]);
      //processBPMTimeOffset(bpmVals[3]);
    }    
  }

//  if (equalizerLongChar.written()) {
//    setLedAnimationPattern(ANIMATION_OFF);
//    // Equalizer vals need bitwise operations to unpack
//    // This is a quick operations < 100 MICRO seconds
//    equalizerLongChar.readValue(unpackedEqVals, 9);    
//    showUnpackedEqValues(unpackedEqVals);
//  } 

  if (beatSequenceChar.written()) {
    setLedAnimationPattern(ANIMATION_BPM_TEST);
    // This is a quick operations < 100 MICRO seconds
    beatSequenceChar.readValue(beatSequenceVals, 20);   

    // Decode and attach to specified animation
    if (beatSequenceVals[1] == 0) {  
      decodeAndAppendBeatSequence(beatSequenceVals, 20, getSimpleFadeBeatSequence());           
    } else if (beatSequenceVals[1] == 1) {  
      decodeAndAppendBeatSequence(beatSequenceVals, 20, getRainbowBeatSequence());           
    } else if (beatSequenceVals[1] == 2) {  
      decodeAndAppendBeatSequence(beatSequenceVals, 20, getBeatBc());
    }
  
    Serial.println("Beat Sequence Recieved");
    for (int i = 0; i < 20; i++) {
      Serial.print(beatSequenceVals[i]); 
      Serial.print(", "); 
    }    
    Serial.println("");
  } 
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
