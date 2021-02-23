/*
 * LED Animation to check if BPM and beat counting is correct 
 */
 
#include <FastLED.h>

byte row = 0;  // Start at the top row
//byte col = 11; // Start 
double col = 0.0;
double rowLedCount = 9.0 * 6.0;
byte lastRow = 0;
byte lastCol = 0;

struct BeatSequence beatBc;
BeatSequence* getBeatBc() {
  return &beatBc;
}

void anim_BeatCheck(uint16_t beatNumInMeasure) {
  uint16_t beatNumScaled = beatNumInMeasure;
  if (beatNumScaled >= (16 * 4)) {
    beatNumScaled = beatNumScaled / 4;
  }
  
  FastLED_SetRGB(to_led_idx(lastRow, lastCol), 0, 0, 0);
  double beatsThroughMeasure = ((double)beatNumScaled / 16.0);
  row = (byte)beatsThroughMeasure;
  col = round((beatsThroughMeasure - row) * rowLedCount) + 17;
  FastLED_SetRGB(to_led_idx(row, (byte)col), 132, 222, 2);

  lastRow = (byte)row;
  lastCol = (byte)col;
//  
//  col = 11 - (beatNumInMeasure / 24);
//  for (int i = 11; i >= (11 - 3); i--) {
//    if (i >= col) {
//      FastLED_lightBlockRGB(row, i, 132, 222, 2);
//    } else {
//      FastLED_lightBlockRGB(row, i, 0, 0, 0);
//    }
//  }  
}
