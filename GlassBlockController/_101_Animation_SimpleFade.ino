/*
  LED animation pattern functions and variables   
*/

#include <FastLED.h>

byte rainbow[14] = { 0, 18, 36, 54, 72, 90, 108, 126, 144, 162, 180, 198, 216, 234 };
float iteratorStep = ((float)rainbow[1] / (float)6);

struct BeatSequence beatSf;
BeatSequence* getSimpleFadeBeatSequence() {
  return &beatSf;
}

struct AnimationBasics animSf;

void runSimpleFadeAnimation(uint16_t beatNumInMeasure) {
  
  if (isABeat(beatNumInMeasure, beatSf)) {   
    //Serial.println("Beat for SF");
  
    // Fade to next beat
    animSf.idx++;
    if (animSf.idx >= 14) {
      animSf.idx = 0;        
    }
    animSf.idx = (animSf.idx + 1) % 14;
    animSf.iterator = iteratorStep;   
  }

  animSf.hue += animSf.iterator;  

  // Check for the end of the fade to next color animation
  if ((animSf.hue > rainbow[animSf.idx]) ||
      (animSf.idx == 0 && animSf.hue >= 255.0)) {

    // Set the ending target value and stop animation
    animSf.hue = rainbow[animSf.idx];
    animSf.iterator = 0.0;
  }

  if (animSf.hue > 255.0) {
    animSf.hue -= 255.0;
  }
  
  FastLED_FillSolidHSV((byte)animSf.hue, 255, 255); 
}
