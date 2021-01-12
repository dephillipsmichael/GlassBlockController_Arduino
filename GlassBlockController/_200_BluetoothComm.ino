/*
  This portion of the app has only been tested with the 
  
  * Arduino Wifi Rev 2

  It may work on the nRF Feather and other BLE enabled
  Arduinos microcontrollers.
*/
#include <ArduinoBLE.h>

// #define BLE_DEBUG 1;

// LED srevice for controlling the glass block wall
BLEService ledService("6e400001-b5a3-f393-e0a9-e50e24dcca9e"); // create service

// Receive bpm and timing info for syncing to music
BLECharacteristic communicationChar =  BLECharacteristic("6e400005-b5a3-f393-e0a9-e50e24dcca9e", BLEWriteWithoutResponse, 20);

// Beat measure characteristic data length
const uint8_t beatMeasueDataLength = 20;

/**
 * BleCommands switch the controller type
 * or provide additional info to a specific controller
 */
enum BleCommand {
  BLE_ARGB            = 0,
  BLE_ANIMATION       = 1,
  BLE_START_BEAT_SEQ  = 2,
  BLE_APPEND_BEAT_SEQ = 3,
  BLE_BEAT_TRACKING   = 4,
  BLE_BPM_INFO        = 5  
};

#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

void blePeripheralConnectHandler(BLEDevice central);
void blePeripheralDisconnectHandler(BLEDevice central);

// Buffer to retrieve the next part of the beat sequence
byte* beatSequenceVals = new byte[20];

void setupBluetooth() {
  // begin initialization
  if (!BLE.begin()) {
    Serial.println("starting BLE failed!");
    // TODO: mdephillips 12/15/20 flashing red LED instead?
    while (1);
  }

  // Set the desired connection interval to a strict 7.5ms - 15ms
  BLE.setConnectionInterval(0x0006, 0x0006);
  //BLE.setPhy(LE_2M, LE_2M);

  // Set the local name peripheral advertises
  BLE.setLocalName("Glass Block Bar Controller");  

  // Assign event handlers for connected, disconnected to peripheral
  BLE.setEventHandler(BLEConnected, blePeripheralConnectHandler);
  BLE.setEventHandler(BLEDisconnected, blePeripheralDisconnectHandler);
  
  // set the UUID for the service this peripheral advertises:
  BLE.setAdvertisedService(ledService);

  // add the characteristics to the service
  ledService.addCharacteristic(communicationChar);   

  // add the service
  BLE.addService(ledService);

  // start advertising
  BLE.advertise();

  Serial.println("Bluetooth device active, waiting for connections...");
}

unsigned long now = 0;
boolean loopBluetooth() {  
  
  now = micros();
   
  // This function on avg takes only
  // ~31 MICRO seconds to perform
  // However, if there is a characteristic
  // to read, like the 9 byte EQ,
  // This takes ~1000 MICRO secodns to perform.
  BLE.poll();

  // Check for a change in the communication characteristic
  if (communicationChar.written()) {

    // This is a quick operations < 100 MICRO seconds
    communicationChar.readValue(beatSequenceVals, 20); 

    #ifdef BLE_DEBUG
      Serial.println("BLE MSG ");
      for (int i = 0; i < 20; i++) {
        Serial.print(beatSequenceVals[i]); 
        if (i < 19) {
          Serial.print(", "); 
        }
      }    
      Serial.println();
    #endif   

    switch (beatSequenceVals[0]) {
      
      case BLE_ARGB:              
      
        // Sending black will just control the global brightness
        if (!(0 == beatSequenceVals[2] && 0 == beatSequenceVals[3] && 0 == beatSequenceVals[4])) {  
          setControllerType(ControllerType_Color);
          processRgbCommand(beatSequenceVals[2], beatSequenceVals[3], beatSequenceVals[4]);                   
        } else {
          #ifdef BLE_DEBUG
            Serial.print("New global alpha ");
            Serial.println(beatSequenceVals[1]);           
          #endif      
          // Make sure the brightness is relative to the max in this code
          setGlobalBrightness(beatSequenceVals[1]);  
        }        
        
        break;

      case BLE_ANIMATION:

        setControllerType(ControllerType_Animation);
        processAnimParams(beatSequenceVals);
      
        break;

      case BLE_START_BEAT_SEQ:
      case BLE_APPEND_BEAT_SEQ:        
        
        setControllerType(ControllerType_Beat);
        processBeatSequence(beatSequenceVals);

        break;

      case BLE_BEAT_TRACKING:

        setControllerType(ControllerType_Beat);
        processBeatNum((uint16_t)beatSequenceVals[1] + (uint16_t)beatSequenceVals[2]); 
  
        break;

      case BLE_BPM_INFO:  
    
        setControllerType(ControllerType_Beat);
        processBpmInfo(beatSequenceVals, now);
        
        break;
    }          
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
