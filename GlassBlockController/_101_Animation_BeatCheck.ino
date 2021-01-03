/*
 * LED Animation to check if BPM and beat counting is correct 
 */
 
#include <FastLED.h>

byte row = 0;  // Start at the top row
byte col = 11; // Start 

struct BeatSequence beatBc;
BeatSequence* getBeatBc() {
  return &beatBc;
}

void anim_BeatCheck(uint16_t beatNumInMeasure) {
  col = 11 - (beatNumInMeasure / 24);
  for (int i = 11; i >= (11 - 3); i--) {
    if (i >= col) {
      FastLED_lightBlockRGB(row, i, 132, 222, 2);
    } else {
      FastLED_lightBlockRGB(row, i, 0, 0, 0);
    }
  }
}
