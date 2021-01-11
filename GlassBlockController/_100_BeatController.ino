/*
  LED animation pattern functions and variables   
*/

#include <FastLED.h>

#define BEAT_CONTROLLER_DEBUG 1

/**
 * Holds a pointer to an array of animation objects
 */
uint8_t beatAnimationCount = 0;
struct Animation* beatAnimations = NULL;

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

  #ifdef BEAT_CONTROLLER_DEBUG  
    Serial.print("New start time ");
    Serial.print(bpmStartTimeMicros);
    Serial.print(", New offset micros ");
    Serial.print(bpmStartTimeOffsetMicros);
    Serial.println();
  #endif  
}
void setBpmDelay(uint8_t bleDelay) {
  bleDelayMicros = bleDelay * 1000;
  bpmStartTimeOffsetMicros = bpmStartTimeMicros - bleDelayMicros;

  #ifdef BEAT_CONTROLLER_DEBUG 
    Serial.print("New offset micros ");
    Serial.print(bpmStartTimeOffsetMicros);
    Serial.println();
  #endif
}
void addFastLEDShowDelay(uint16_t delayMicros) {
  bpmStartTimeOffsetMicros -= delayMicros; 
}

uint16_t last24thBeat = 0;
uint16_t current24thBeat = 0;
unsigned long lastBeatTime = 0;
unsigned long nowLoop = micros();
uint16_t beatCtr = 0;

void initController_Beat() {
  if (beatAnimationCount > 0) {
    return; // Already initialized
  }

  #ifdef BEAT_CONTROLLER_DEBUG
    Serial.print(F("init beat ctrlr"));
  #endif

  // Create animation size of 1
  beatAnimations = malloc(sizeof(Animation));
  beatAnimationCount = 1;

  // Default animation
  setAnimFunc_RainbowRow(&beatAnimations[0]);
//  setAnimFunc_Tetris(&beatAnimations[0]);

  beatAnimations[0].init();
}

void destroyController_Beat() {
  for (int i = 0; i < beatAnimationCount; i++) {
    beatAnimations[i].destroy();
  }  
  free(beatAnimations);
  beatAnimations = NULL;  
  beatAnimationCount = 0;
}

void runLoopController_Beat() {
  // No-op needed until I get RTC module for keeping time
  // For now, BLE sends each beat
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

void processBeatSequence(byte sequenceMsg[]) {
  
  byte animIdx = sequenceMsg[1];
  // Decode and attach to specified animation
  if (animIdx < beatAnimationCount) {  // bounds check
    decodeAndAppendBeatSequence(sequenceMsg, 20, beatAnimations[animIdx].beat());

    #ifdef BEAT_CONTROLLER_DEBUG
      Serial.print("Decoded new beat seq ");
      Serial.print(animIdx);
      Serial.println();
    #endif
  }
}

void processBeatNum(uint16_t beatNumInMeasure) {
  for (int i = 0; i < beatAnimationCount; i++) {
    beatAnimations[i].draw(beatNumInMeasure);
  }
  FastLED.show();
}

/**
 * @return the the iterator step that matches the current BPM
 */
double stepForBpm() {
  // 20 is max iterator, steps in 0.25 between 0 and 200
  return (20.0 * (animBpm / 200.0));
}

/**
 * @return true if the beat controller is running, false otherwise
 */
boolean isBeatControllerRunning() {
  return beatAnimationCount > 0;
}

/**
 * @return true if the current beat num has a beat in a sequence
 */
boolean isABeat(uint16_t beatNumInMeasure, struct BeatSequence* beats) {
  for (int i = 0; i < beats->sequenceSize; i++) {
    if (beatNumInMeasure == beats->sequence[i]) {
      #ifdef BEAT_CONTROLLER_DEBUG
        Serial.print(i);        
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
  if (beats.sequenceSize == 0) {
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
  // TODO: switch 96 to beats in time signature
  return (96 - beatNumInMeasure) + beats.sequence[0];
}
