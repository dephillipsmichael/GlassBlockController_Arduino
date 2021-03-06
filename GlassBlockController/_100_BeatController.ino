/*
  LED animation pattern functions and variables   
*/

#include <Wire.h>
#include <FastLED.h>

#define BEAT_CONTROLLER_DEBUG 1

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

struct BeatControllerVars {
  /**
   * Holds a pointer to an array of animation objects
   */
  struct Animation* animations;
  /**
   * The number of animations
   */
  uint8_t animationCount;

  /**
   * BPM (beats per minute) controls the refresh rate of the LEDs
   * so that the screen can be reactive as possible to musical beats.
   */
  uint16_t bpm;

  /**
   * The time in micros when the BPM was set
   */
  unsigned long bpmStartTimeMicros;

  /**
   * In order to account for the latency of BLE transmission
   * This can be set to fine tune the BPM start time
   */
  unsigned long bleDelayMicros;

  /**
   * The number of frames drawn within a beat
   * This reflects the beat sequence encoding values
   */
  uint8_t framesPerBeat;

  /**
   * This is analogous with a musical time signature,
   * and the beat number will wrap back to 0 at this number
   */
  uint8_t beatsInMeasure;
};

struct BeatControllerVars* beatCtrl = NULL;

/**
 * Set the beat in meausre var
 */
void setBeatsInMeasure(uint16_t newBeatsPerMeasure) {
  if (beatCtrl == NULL) {
    #ifdef BEAT_CONTROLLER_DEBUG  
      Serial.println(F("Beat controller not initialized"));
    #endif
    return;
  }
  
  beatCtrl->beatsInMeasure = newBeatsPerMeasure;
}

/**
 * Set the BPM and its assignment time
 */
void setBpm(uint16_t newBpm, unsigned long startTimeMicros) {  
  beatCtrl->bpm = newBpm;
  beatCtrl->bpmStartTimeMicros = startTimeMicros;

  #ifdef BEAT_CONTROLLER_DEBUG  
    Serial.print("New BPM ");
    Serial.print(beatCtrl->bpm);
    Serial.print(", and start time ");
    Serial.print(beatCtrl->bpmStartTimeMicros);    
    Serial.println();
  #endif  
}

/**
 * Set the BLE delay
 */
void setBpmDelay(uint8_t bleDelay) {
  beatCtrl->bleDelayMicros = bleDelay * 1000;

  #ifdef BEAT_CONTROLLER_DEBUG 
    Serial.print("New ble delay micros ");
    Serial.print(beatCtrl->bleDelayMicros);
    Serial.println();
  #endif
}

void setBpmBeatsInMeasure(uint16_t beatsInMeasure) {
  beatCtrl->beatsInMeasure = beatsInMeasure;

  #ifdef BEAT_CONTROLLER_DEBUG 
    Serial.print("New beats in measure ");
    Serial.println(beatCtrl->beatsInMeasure);
  #endif
}

void initController_Beat() {
  if (beatCtrl != NULL) {
    #ifdef BEAT_CONTROLLER_DEBUG
      Serial.println(F("init already called for beat ctrlr"));
    #endif    
    return; // Already initialized
  }

  #ifdef BEAT_CONTROLLER_DEBUG
    Serial.println(F("init beat ctrlr"));
  #endif

  // join i2c bus for timestamp millis (address optional for master)
  Wire.begin(); 
  delay(500);

  #ifdef BEAT_CONTROLLER_DEBUG
    Serial.println(F("Wire begin"));
  #endif  

  beatCtrl = malloc(sizeof(BeatControllerVars));
  initBeatControllerVars();

  // Create animation size of 1
  beatCtrl->animations = malloc(sizeof(Animation));
  beatCtrl->animationCount = 1;

  // Default animation
  //setAnimFunc_RainbowRow(&(beatCtrl->animations[0]));  
  //setAnimFunc_Tetris(&beatAnimations[0]);
  //setAnimFunc_RainbowSine(&(beatCtrl->animations[0]));
  setAnimFunc_SimpleFade(&(beatCtrl->animations[0]));

  (beatCtrl->animations[0]).init();
}

void destroyController_Beat() {
  for (int i = 0; i < beatCtrl->animationCount; i++) {
    beatCtrl->animations[i].destroy();
  }  
  free(beatCtrl->animations);
  free(beatCtrl);
  beatCtrl = NULL;

  // End wire monitoring
  Wire.end();
}

uint16_t lastBeat = 0;
uint16_t newBeat = 0;

unsigned long debugTimeDuration = 0;
int debugTimeCtr = 0;

void runLoopController_Beat() {

  if (beatCtrl == NULL) {
    #ifdef BEAT_CONTROLLER_DEBUG
      Serial.println(F("beat ctrlr not initialized"));
    #endif
    return;
  }

  if (beatCtrl->bpm == 0) {
    // BPM needs set first
    return;
  }

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
  
  // Calculate new divisional beat within a measure
  newBeat = quantizeBeatWithinMeasureTruncate(
    SixteenthBeat, beatCtrl->beatsInMeasure,
    lastMillis * 1000, beatCtrl->bpm, beatCtrl->bpmStartTimeMicros);

  // If we proceeded to the next divisional beat, draw an LED frame
  if (lastBeat != newBeat) {
    lastBeat = newBeat;
    processBeatNum(lastBeat);
  }  
}

enum ControllerType controllerType_Beat();
enum ControllerType controllerType_Beat() {
  return ControllerType_Beat;
}

void assignController_Beat(struct Controller* controller) {
  controller->runLoop = &runLoopController_Beat;
  controller->init = &initController_Beat;
  controller->destroy = &destroyController_Beat;
  controller->type = &controllerType_Beat;
}

void initBeatControllerVars() {
  beatCtrl->animations = NULL;
  beatCtrl->animationCount = 0;
  beatCtrl->bpm = 0;
  beatCtrl->bpmStartTimeMicros = 0;
  beatCtrl->bleDelayMicros = 0;
  beatCtrl->framesPerBeat = 16;
  beatCtrl->beatsInMeasure = 4;
}

/**
 * Called by BLE manager to send BPM info params through to the controller
 */
void processBpmInfo(byte bpmInfoParams[]) {
  refreshI2cTimeMillis();
  setBpmBeatsInMeasure(bpmInfoParams[1]);
  setBpm(bpmInfoParams[2], lastMillis * 1000);
  setBpmDelay(bpmInfoParams[3]);
}

void processBeatSequence(byte sequenceMsg[]) {
  byte animIdx = sequenceMsg[1];
  // Decode and attach to specified animation
  if (animIdx < beatCtrl->animationCount) {  // bounds check
    decodeAndAppendBeatSequence(sequenceMsg, 20, beatCtrl->animations[animIdx].beat());

    #ifdef BEAT_CONTROLLER_DEBUG
      Serial.print("Decoded new beat seq ");
      Serial.print(animIdx);
      Serial.println();
    #endif
  }
}

void processBeatNum(uint16_t beatNumInMeasure) {
  #ifdef BEAT_CONTROLLER_DEBUG     
    if (beatNumInMeasure == 0 || beatNumInMeasure == 16 || beatNumInMeasure == 32 || beatNumInMeasure == 48) {
      Serial.print("Beat ");   
      Serial.println(beatNumInMeasure);   
    }
  #endif  
  
  for (int i = 0; i < beatCtrl->animationCount; i++) {
    beatCtrl->animations[i].draw(beatNumInMeasure);
  }
  FastLED.show();
}

/**
 * @return the the iterator step that matches the current BPM
 */
double stepForBpm() {
  // 20 is max iterator, steps in 0.25 between 0 and 200
  return (20.0 * (beatCtrl->bpm / 200.0));
}

byte stepForBopmByte() {
  return (byte)(((double)beatCtrl->bpm / 200.0) * 20);
}

/**
 * @return true if the beat controller is running, false otherwise
 */
boolean isBeatControllerRunning() {
  if (beatCtrl == NULL) {
    return false;
  }
  return beatCtrl->animationCount > 0;
}

/**
 * @return true if the current beat num has a beat in a sequence
 */
boolean isABeat(uint16_t beatNumInMeasure, struct BeatSequence* beats) {

  for (int i = 0; i < beats->sequenceSize; i++) {
    if (beatNumInMeasure == beats->sequence[i]) {
      #ifdef BEAT_CONTROLLER_DEBUG
        Serial.print(beats->sequence[i]);        
        Serial.println(" beat found");       
      #endif
      return true;
    }
  } 
  return false;
}

/**
 * @return the number of beats distance to next beat in the sequence, -1 if no next beat
 */
int distanceToNextBeat(uint16_t beatNumInMeasure, struct BeatSequence beats) {
  if (beatCtrl == NULL || beats.sequenceSize == 0) {
    return -1;
  }

  // Beats are ordered lowest to highest, so loop til we pass one,
  // then we know where the next beat is
  for (int i = 0; i < beats.sequenceSize; i++) {
    if (beatNumInMeasure > beats.sequence[i] && i < (beats.sequenceSize - 1)) {
      return beats.sequence[i + 1] - beatNumInMeasure;
    }   
  } 
  
  // On last beat now, add distance to first beat
  int beatNumEnd = beatFramesInMeasure();
  return (beatNumEnd - beatNumInMeasure) + beats.sequence[0];
}

uint16_t beatFramesInMeasure() {
  return beatCtrl->beatsInMeasure * beatCtrl->framesPerBeat;
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
