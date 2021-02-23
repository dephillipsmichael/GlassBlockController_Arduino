/*
  LED animation pattern functions and variables   
*/

#include <FastLED.h>

#define DEBUG_SIMPLE_FADE 1

struct SimpleFadeStruct {
  byte* rainbow;
  byte rainbowSize;

  double hue;
  double fadeSpeed;

  // The index within the color set
  byte hueSetIdx;
  byte* hueSet;
  byte hueSetSize;
};

struct SimpleFadeStruct* params_SF;
struct BeatSequence* beats_SimpleFade;

void init_simpleFadeStruct() {
  params_SF->rainbowSize = 14;
  params_SF->rainbow = new byte[14] { 0, 18, 36, 54, 72, 90, 108, 126, 144, 162, 180, 198, 216, 234 };

  params_SF->hueSet = malloc(sizeof(byte) * params_SF->rainbowSize);
  for (int i = 0; i < params_SF->rainbowSize; i++) {
    params_SF->hueSet[i] = params_SF->rainbow[i];
  }
  params_SF->hueSetIdx = 0;
  params_SF->hueSetSize = params_SF->rainbowSize;
  params_SF->hue = params_SF->hueSet[0]; 
  params_SF->fadeSpeed = 24.0;  
}

/**
 * Initialize memory required to run this animation
 */
void init_SimpleFade() {  
  #ifdef DEBUG_SIMPLE_FADE
    Serial.println("init simple fade");
  #endif
  
  params_SF = malloc(sizeof(struct SimpleFadeStruct));
  // This mallocs both rainbow and hueSet vars
  init_simpleFadeStruct();
  
  beats_SimpleFade = malloc(sizeof(struct BeatSequence));   
  beats_SimpleFade->sequenceSize = 0;
  beats_SimpleFade->sequence = NULL;
}

/**
 * Free up memory used by this animation
 */
void free_SimpleFade() {
  free(beats_SimpleFade->sequence);
  free(beats_SimpleFade);
  beats_SimpleFade = NULL;

  free(params_SF->hueSet);
  free(params_SF->rainbow);
  free(params_SF);
  params_SF = NULL;
}

/**
 * @return the rainbow animation type
 */
enum AnimType type_SimpleFade();
enum AnimType type_SimpleFade() {
  return AnimType_SimpleFade;
}

/**
 * Sets the default parameters for this animation
 */
void params_SimpleFade(byte params[]) {
  if (params_SF == NULL) {
    #ifdef DEBUG_SIMPLE_FADE
      Serial.println("params_SF not yet intialized");
    #endif
    return;
  }

  params_SF->fadeSpeed = (((49.0 - (double)params[3]) * 2.0) + 16.0) / 4.0;

  #ifdef DEBUG_SIMPLE_FADE
    Serial.print("New speed ");
    Serial.println(params_SF->fadeSpeed);
  #endif
}


/**
 * Return the beat sequence stored for this animation
 */
BeatSequence* beatSequence_SimpleFade() {
  return beats_SimpleFade;
}

/**
 * Sets the anim struct with the correct function pointers
 */
void setAnimFunc_SimpleFade(struct Animation* anim) {
  anim->type = &type_SimpleFade;
  anim->draw = &draw_SimpleFade;
  anim->params = &params_SimpleFade;
  anim->init = &init_SimpleFade;
  anim->destroy = &free_SimpleFade;
  anim->beat = &beatSequence_SimpleFade;
}

void draw_SimpleFade(uint16_t beatNumInMeasure) {

  boolean shouldFadeToNextIdx = false;
  
  if (isBeatControllerRunning() && isABeat(beatNumInMeasure, beats_SimpleFade)) {  
    shouldFadeToNextIdx = true;
  }

  if (!isBeatControllerRunning() &&   
      (beatNumInMeasure % ((int)(49.0 - params_SF->fadeSpeed)) == 0)) {
    shouldFadeToNextIdx = true;
  }

  if (shouldFadeToNextIdx) {
    
    // Set the hue to the previous index
    // To avoid a sceario where it wasn't quite done animating
    params_SF->hue = params_SF->hueSet[params_SF->hueSetIdx];
    
    // Fade to next beat
    params_SF->hueSetIdx++;
    if (params_SF->hueSetIdx >= params_SF->hueSetSize) {
      params_SF->hueSetIdx = 0;       
    }  

    #ifdef DEBUG_SIMPLE_FADE
      Serial.print("shouldFadeToNextIdx ");
      Serial.println(params_SF->hueSetIdx);
    #endif    
  }

  // Check for if we are fading to the next color
  if (params_SF->hue != params_SF->hueSet[params_SF->hueSetIdx]) {
    byte lastIndex = params_SF->hueSetIdx - 1;
    if (params_SF->hueSetIdx == 0) {
       lastIndex = params_SF->hueSetSize - 1;
    }    
    byte hueDirection = params_SF->hueSet[params_SF->hueSetIdx] > params_SF->hueSet[lastIndex];
    byte hueDiff = abs((int)params_SF->hueSet[params_SF->hueSetIdx] - (int)params_SF->hueSet[lastIndex]);
    if (hueDiff > 128) {  // It would be faster to fade hue the other direction
      hueDiff = 255 - hueDiff;
      hueDirection = !hueDirection;
    }
    double hueStep = (double)hueDiff / params_SF->fadeSpeed;

    // Check for near ending conditions, in which case we finish the fade
    if (hueDirection) { // check for fade increasing hue
      params_SF->hue += hueStep;
      if (params_SF->hue > 255.0) {
        params_SF->hue -= 255.0;
      }
    } else { // check for fade to left, decreasing hue
      params_SF->hue -= hueStep;
      if (params_SF->hue < 0.0) {
        params_SF->hue += 255.0;
      }
    }    

    if (abs(params_SF->hue - params_SF->hueSet[params_SF->hueSetIdx]) < hueStep) {
      params_SF->hue = params_SF->hueSet[params_SF->hueSetIdx];
    }       
  }
  
  FastLED_FillSolidHSV((byte)params_SF->hue, 255, 255);  
}
