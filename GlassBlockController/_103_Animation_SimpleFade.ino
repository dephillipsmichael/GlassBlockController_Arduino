/*
  LED animation pattern functions and variables   
*/

#include <FastLED.h>

byte rainbow[14] = { 0, 18, 36, 54, 72, 90, 108, 126, 144, 162, 180, 198, 216, 234 };
//byte rainbow[14] =   { 0, 54, 108, 162, 216, 18, 72, 126, 180, 234, 36, 90, 144, 198 };
float iteratorStep = ((float)rainbow[1] / (float)3);
boolean isHigher = false;

struct BeatSequence beatSf;
BeatSequence* getSimpleFadeBeatSequence() {
  return &beatSf;
}

struct AnimationBasics animSf;

void runSimpleFadeAnimation(uint16_t beatNumInMeasure) {
  
  if (isABeat(beatNumInMeasure, beatSf)) {   
    Serial.println("Beat for SF");
  
    // Fade to next beat
    animSf.idx++;
    if (animSf.idx >= 14) {
      animSf.idx = 0;       
      isHigher = true;
    } else {
      isHigher = false;
    }
    animSf.iterator = iteratorStep;  
  }

  animSf.hue += animSf.iterator;  

  // Check for the end of the fade to next color animation
  if ((animSf.hue > rainbow[animSf.idx]) && !isHigher) {

    // Set the ending target value and stop animation
    animSf.hue = rainbow[animSf.idx];
    animSf.iterator = 0.0;
  }

  if (animSf.hue > 255.0) {
    animSf.hue -= 255.0;
    isHigher = false;
  }
  
  FastLED_FillSolidHSV((byte)animSf.hue, 255, 255); 
}
