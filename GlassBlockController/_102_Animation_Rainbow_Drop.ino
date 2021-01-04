/*
 * LED animation pattern functions and variables   
 */
 
#include <FastLED.h>
//
//CRGB colorNautical = CRGB(0, 255, 198);
//CRGB colorRaspberrySorbet = CRGB(255, 0, 127);
//CRGB colorWhite = CRGB(255, 255, 255);
//CRGB colorLavender = CRGB(158, 0, 255);
//CRGB colorYellow = CRGB(255, 194, 9);

int x = 0;
int i = 0;
int randomRow = 0;
int randomColor = 0;
struct BeatSequence beatRd;

//int onScreen[32] = { 0 };

BeatSequence* getRainbowBeatSequence() {
  return &beatRd;
}

void runRainbowDropAnimation(uint16_t beatNumInMeasure) {
  if (beatRd.sequenceSize == 0) {
    return;
  }
  
  if (isABeat(beatNumInMeasure, beatRd)) {
    Serial.println("Beat for SF");
    randomRow = (randomRow + 1) % 5;
  }
  for (int col = 0; col < 12; col++) {
      FastLED_lightBlockHue(randomRow, col, 255 / (randomRow + 1));     
    } 
}
