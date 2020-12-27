/*
  LED animation pattern functions and variables   
*/
#include <FastLED.h>

#define ANIMATION_DEBUG = 1

#define ANIMATION_OFF 0
#define ANIMATION_RAINBOW 1
#define ANIMATION_RAINBOW_ROW 2
#define ANIMATION_BPM_TEST 6

byte rainbowRed = 33;
byte rainbowGreen = 99;
byte rainbowBlue = 133;

byte rainbowRowRed = 33;
byte rainbowRowGreen = 99;
byte rainbowRowBlue = 134;

// The current hue value for the rainbow animation
float rainbowAnimHue = 0.0;
float rainbowAnimHueSpeed = 2.0; 
boolean reverseDirection = false;
byte hsvBrightness = 255;
byte bpmAnimation = 0;

// The current hue value for the rainbow per row animation
// Other good starting ones are is float 
float rainbowRowAnimEqualSpace[] = { 0.0, 50.0, 100.0, 150.0, 200.0 };
float rainbowRowAnimArrow[] = { 30.0, 42.0, 54.0, 42.0, 30.0 };
float rainbowRowAnimHueLine[] = { 30.0, 42.0, 54.0, 66.0, 78.0 };
float rainbowRowAnimHue[] = { rainbowRowAnimHueLine[0], rainbowRowAnimHueLine[1], rainbowRowAnimHueLine[2], rainbowRowAnimHueLine[3], rainbowRowAnimHueLine[4] };
float rainbowRowAnimHueSpeed[] = { 2.0, 2.0, 2.0, 2.0, 2.0 };

// Current pattern
int currentPattern = 0;
int rainbowPatternSine = 3;
int rainbowPatternSineBlock = 4;
int rainbowPatternCircleBlock = 5;

CRGB colorNautical = CRGB(0, 255, 198);
CRGB colorRaspberrySorbet = CRGB(255, 0, 127);
CRGB colorWhite = CRGB(255, 255, 255);
CRGB colorLavender = CRGB(158, 0, 255);
CRGB colorYellow = CRGB(255, 194, 9);

// Stores the current BPM and start time of the tempo 
unsigned long bpmStartTime = 0; 
uint8_t bpm = 0;
int bpmOffset = 0;
float bpmStartAnimSpeed = 0.0;
int bpmFrameIdx = 0;

// The command for starting an animation
byte animationAlpha = 1;

// The average Lo
int* avgLowMidHigh = new int[3];

byte ledAnimationVal = 0;
void setLedAnimationPattern(byte animationIdx) {
  ledAnimationVal = animationIdx;
}

void animationLoop() {
  if (ledAnimationVal == ANIMATION_RAINBOW) {
    fastLedRainbow();
  } else if (ledAnimationVal == ANIMATION_RAINBOW_ROW) {
    rainbowRow();
  } else if (ledAnimationVal == ANIMATION_BPM_TEST) {
    doBpmAnimation();
  }  
}

// Process ARGB animation command
void processArgb(byte alpha, byte r, byte g, byte b) {   

  if (animationAlpha == alpha &&
     rainbowRed == r &&
     rainbowGreen == g &&
     rainbowBlue == b) {

    #ifdef ANIMATION_DEBUG
      Serial.println("RA start");            
    #endif      

    ledAnimationVal = ANIMATION_RAINBOW;        
    return;      
  }

  if (animationAlpha == alpha &&
     rainbowRowRed == r &&
     rainbowRowGreen == g &&
     rainbowRowBlue == b) {

    #ifdef ANIMATION_DEBUG
      Serial.println("RR start");            
    #endif
    ledAnimationVal = ANIMATION_RAINBOW_ROW;        
    return;      
  }

  // Rainbow Speed 
  if (1 == alpha && 0 == r && g == 0) {  
    rainbowAnimHueSpeed = (float)((uint8_t)b) * 0.25;

    for (int i = 0; i < blocksRowCount; i++) {
      rainbowRowAnimHueSpeed[i] = rainbowAnimHueSpeed;
    }

    #ifdef ANIMATION_DEBUG
      Serial.print("RA new speed ");
      Serial.println(b);           
    #endif
    
    return;
  }
  
  // Rainbow Row Speeds
  if (1 == alpha && 1 == r && (g >= 0 && g < 2)) {   
    if (g == 0) {
      rainbowAnimHueSpeed = (float)((uint8_t)b) * 0.25;
      for (int i = 0; i < blocksRowCount; i++) {
        rainbowRowAnimHueSpeed[i] = rainbowAnimHueSpeed;
      }  
       
      #ifdef ANIMATION_DEBUG
        Serial.print("RR new speed ");
        Serial.print(b);      
        Serial.print(" for idx: ");
        Serial.println(g);             
      #endif   
    } else if (g == 1) {
      float* hues = rainbowRowAnimEqualSpace;
      if (b == 0) { 
        hues = rainbowRowAnimHueLine;
      } else if (b == 1) {  
        hues = rainbowRowAnimArrow;
      } else if (b == 2) {  
        hues = rainbowRowAnimEqualSpace;
      }
      currentPattern = b;

      for (int i = 0; i < blocksRowCount; i++) {
        rainbowRowAnimHue[i] = hues[i];
      }
      
      #ifdef ANIMATION_DEBUG
        Serial.print("RR new pattern ");
        Serial.println(b);            
      #endif   
    }
 
    return;
  }

  // Make sure the brightness is relative to the max in this code
  setGlobalBrightness(alpha);    

  // Sending black will just control the global brightness 
  if (0 == r && 0 == g && 0 == b) {  

    #ifdef ANIMATION_DEBUG
      Serial.print("New global alpha ");
      Serial.println(alpha);           
    #endif    

     FastLED.show();
     return;      
  }

  #ifdef ANIMATION_DEBUG
    Serial.print(alpha);
    Serial.print(", ");
    Serial.print(r);
    Serial.print(", ");
    Serial.print(g);
    Serial.print(", ");
    Serial.print(b);
    Serial.print(" - new a, r, g, b cmd");
    Serial.println(alpha);             
  #endif  

  ledAnimationVal = ANIMATION_OFF;
  
  FastLED_FillSolid(r, g, b);
  FastLED.show();
}

// Do animation for 9 frequency band equalizer values
void showUnpackedEqValues(byte unpackedEqVals[]) {  
  
  avgLowMidHigh[0] = max(max(unpackedEqVals[0], unpackedEqVals[1]), unpackedEqVals[2]);
  avgLowMidHigh[1] = max(max(unpackedEqVals[3], unpackedEqVals[4]), unpackedEqVals[5]);
  avgLowMidHigh[2] = max(max(unpackedEqVals[6], unpackedEqVals[7]), unpackedEqVals[8]);
  
  for (int col = 0; col < 3; col++) {   
    for (int row = 0; row < blocksRowCount; row++) {
      uint8_t rowAdjusted = (ledsRowCount - 1) - row;
      if (avgLowMidHigh[col] > row) {
        if (rowAdjusted == 4) {
           FastLED_lightBlockRGB(rowAdjusted, col, 158, 0, 255);
        } else if (rowAdjusted == 3) {
           FastLED_lightBlockRGB(rowAdjusted, col, 0, 255, 198);
        } else if (rowAdjusted == 2) {
           FastLED_lightBlockRGB(rowAdjusted, col, 255, 194, 9);
        } else if (rowAdjusted == 1) {
           FastLED_lightBlockRGB(rowAdjusted, col, 255, 0, 127);
        } else if (rowAdjusted == 0) {
           FastLED_lightBlockRGB(rowAdjusted, col, 255, 255, 255);
        }        
      } else {
        FastLED_lightBlockRGB(rowAdjusted, col, 50, 50, 50);
      }
    }
  }

  for (int col = 3; col < blocksPerRow; col++) {
    for (int row = 0; row < blocksRowCount; row++) {
      uint8_t rowAdjusted = (ledsRowCount - 1) - row;
      if (unpackedEqVals[col - 3] > row) {
        if (rowAdjusted == 4) {
           FastLED_lightBlockRGB(rowAdjusted, col, 158, 0, 255);
        } else if (rowAdjusted == 3) {
           FastLED_lightBlockRGB(rowAdjusted, col, 0, 255, 198);
        } else if (rowAdjusted == 2) {
           FastLED_lightBlockRGB(rowAdjusted, col, 255, 194, 9);
        } else if (rowAdjusted == 1) {
           FastLED_lightBlockRGB(rowAdjusted, col, 255, 0, 127);
        } else if (rowAdjusted == 0) {
           FastLED_lightBlockRGB(rowAdjusted, col, 255, 255, 255);
        }       
      } else {
        FastLED_lightBlockRGB(rowAdjusted, col, 50, 50, 50);
      }
    }
  }
  
  FastLED.show(); 
}

void fastLedRainbow () {
  nextRainbowFrame();

  // FastLED's built-in rainbow generator
  fill_rainbow(leds, NUM_LEDS, (byte)rainbowAnimHue, 7);
  
  // send the 'leds' array out to the actual LED strip
  FastLED.show(); 
}

void nextRainbowFrame() {
  if (reverseDirection) {
    rainbowAnimHue -= rainbowAnimHueSpeed;
  } else {
    rainbowAnimHue += rainbowAnimHueSpeed; 
  }  
  if (rainbowAnimHue > 255) {
    rainbowAnimHue = rainbowAnimHue - 255;
  } else if (rainbowAnimHue < 0) {
    rainbowAnimHue = rainbowAnimHue + 255;
  }

  // Incrament all the row's hue values
  for (int i = 0; i < blocksRowCount; i++) {
    if (reverseDirection) {
      rainbowRowAnimHue[i] -= rainbowRowAnimHueSpeed[i];
    } else {
      rainbowRowAnimHue[i] += rainbowRowAnimHueSpeed[i];
    }  
    
    if (rainbowRowAnimHue[i] > 255) {
      rainbowRowAnimHue[i] = rainbowRowAnimHue[i] - 255;
    } else if (rainbowRowAnimHue[i] < 0) {
      rainbowRowAnimHue[i] = rainbowRowAnimHue[i] + 255;
    }
  }
}

// Used by function rainbowRow
float iteratorHues[] = { 0.0, 0.0, 0.0, 0.0, 0.0 };
void rainbowRow() {

  nextRainbowFrame();

  if (currentPattern == rainbowPatternSine ||
    currentPattern == rainbowPatternSineBlock) {
    rainbowSine();
    return;
  }

  if (currentPattern == rainbowPatternCircleBlock) {
    rainbowCircle();
    return;
  }

  for (int i = 0; i < blocksRowCount; i++) {
    // Copy over current hue values to iterator
    iteratorHues[i] = rainbowRowAnimHue[i];
  }
  
  // This loop runs the LEDS a pixel at a time all in the same direction a row at a time
  // Starts at the bottom row and end on the top row
  // TODO: mdephillips 12/20/20 move this to a function pointer to remove dupe code
  int rowIdx = 0;
  for (int i = 0; i < LED_ROW_INDEX_SIZE; i += LED_ROW_INDEX_COUNT) {
    rowIdx = (i / LED_ROW_INDEX_COUNT);
    if (ledRowIndexes[i+2] < 0) {
      for (int ledIdx = ledRowIndexes[i]; ledIdx >= ledRowIndexes[i+1]; ledIdx += ledRowIndexes[i+2]) {
        iteratorHues[rowIdx] += rainbowRowAnimHueSpeed[rowIdx];
        FastLED_SetHueVal(ledIdx, (byte)iteratorHues[rowIdx], hsValueScaled());
      }
    } else {
      for (int ledIdx = ledRowIndexes[i]; ledIdx < ledRowIndexes[i+1]; ledIdx += ledRowIndexes[i+2]) {
        iteratorHues[rowIdx] += rainbowRowAnimHueSpeed[rowIdx];
        FastLED_SetHueVal(ledIdx, (byte)iteratorHues[rowIdx], hsValueScaled());
      }
    }
  }  

  // send the 'leds' array out to the actual LED strip
  FastLED.show(); 
}

void rainbowCircle() {
  // The Rainbow circle patterns starts color in middle of wall and goes outwards equally
  // See Hypno-Toad eyes for reference

  float hue = rainbowAnimHue + 128;
  int circleDiameter = 1;
  int circleDiameterMax = 5;
  int midPointCol = 1;
  int midPointRow = 2;
  int row = midPointCol;
  int col = midPointRow;

  // Hard-coded 5 circles fit inside front facing plane
  while (circleDiameter <= circleDiameterMax) {

    row = midPointRow - (circleDiameter / 2);
    col = midPointCol - (circleDiameter / 2);    

    for (int i = 0; i < circleDiameter; i++) {
      // Top row of circle
      FastLED_lightBlockHue(row, col + i, hue);
      // Left column of circle
      FastLED_lightBlockHue(row + i, col, hue);
      // Bottom row of circle
      FastLED_lightBlockHue(row + (circleDiameter -1), col + i, hue);
      // Right column of circle
      FastLED_lightBlockHue(row + i, col + (circleDiameter - 1), hue);
    }
    
    circleDiameter += 2;
    hue += rainbowRowAnimHueSpeed[0];
    if (hue > 255) {
      hue = hue - 255;
    }
  }

  hue = rainbowAnimHue;
  circleDiameter = 1;
  circleDiameterMax = 5;
  midPointCol = 5;
  midPointRow = 2;
  row = midPointCol;
  col = midPointRow;

  // Hard-coded 5 circles fit inside front facing plane
  while (circleDiameter <= circleDiameterMax) {

    row = midPointRow - (circleDiameter / 2);
    col = midPointCol - (circleDiameter / 2);    

    for (int i = 0; i < circleDiameter; i++) {
      // Top row of circle
      FastLED_lightBlockHue(row, col + i, hue);
      // Left column of circle
      FastLED_lightBlockHue(row + i, col, hue);
      // Bottom row of circle
      FastLED_lightBlockHue(row + (circleDiameter -1), col + i, hue);
      // Right column of circle
      FastLED_lightBlockHue(row + i, col + (circleDiameter - 1), hue);
    }
    
    circleDiameter += 2;
    hue += rainbowRowAnimHueSpeed[0];
    if (hue > 255) {
      hue = hue - 255;
    }
  }

  hue = rainbowAnimHue;
  circleDiameter = 1;
  circleDiameterMax = 5;
  midPointCol = 9;
  midPointRow = 2;
  row = midPointCol;
  col = midPointRow;

  // Hard-coded 5 circles fit inside front facing plane
  while (circleDiameter <= circleDiameterMax) {

    row = midPointRow - (circleDiameter / 2);
    col = midPointCol - (circleDiameter / 2);    

    for (int i = 0; i < circleDiameter; i++) {
      // Top row of circle
      FastLED_lightBlockHue(row, col + i, hue);
      // Left column of circle
      FastLED_lightBlockHue(row + i, col, hue);
      // Bottom row of circle
      FastLED_lightBlockHue(row + (circleDiameter -1), col + i, hue);
      // Right column of circle
      FastLED_lightBlockHue(row + i, col + (circleDiameter - 1), hue);
    }
    
    circleDiameter += 2;
    hue += rainbowRowAnimHueSpeed[0];
    if (hue > 255) {
      hue = hue - 255;
    }
  }

  FastLED.show();
}

void rainbowSine() {
  // The Rainbow Sine pattern moves hue by up the rows, then down the next row, etc.

  float hue = rainbowAnimHue;
  float sat = 255;
  int colLoops = ledsPerRow;
  if (currentPattern == rainbowPatternSineBlock) {
    colLoops = blocksPerRow;
  }
  for (int col = 0; col < colLoops; col++) {    
    // Even rows flow up
    if ((col % 2) == 0) {
      for (int row = 0; row < blocksRowCount; row++) {
        sat = 255;
        hue += rainbowRowAnimHueSpeed[0];
        if (hue > 255) {
          hue = hue - 255;
        }
        if (currentPattern == rainbowPatternSineBlock) {          
          if ((hue >= 0 && hue < 10) ||
              (hue >= 100 && hue < 110) ||
              (hue >= 200 && hue < 210))  {
            sat = 200;
          }
          FastLED_lightBlockHueSat(row, col, hue, sat);
        } else {
          FastLED_SetHueVal(to_led_idx(row, col), hue, hsValueScaled());
        }
      }  
    } else { // Odd rows flow down
      for (int row = (blocksRowCount - 1); row >= 0; row--) {
        sat = 255;
        hue += rainbowRowAnimHueSpeed[0];
        if (hue > 255) {
          hue = hue - 255;
        }
        if (currentPattern == rainbowPatternSineBlock) {
          if ((hue >= 0 && hue < 10) ||
              (hue >= 100 && hue < 110) ||
              (hue >= 200 && hue < 210)) {
            sat = 200;
          }
          FastLED_lightBlockHueSatVal(row, col, hue, sat, hsValueScaled());
        } else {
          FastLED_SetHueVal(to_led_idx(row, col), hue, hsValueScaled());
        }
      }  
    }
  }

  FastLED.show();
}

void processLowMidHigh(byte low, byte mid, byte high) {
  // Too frequent to always log
//    Serial.print(low);
//    Serial.print(", ");
//    Serial.print(mid);
//    Serial.print(", ");
//    Serial.print(high);  
//    Serial.println();

  for (int i = 0; i < 140; i++) {
    if (low > 0) {
      leds[i] = colorNautical;
    } else {
      leds[i] = CRGB::Black;
    }
  }

  for (int i = 140; i < 210; i++) {
    if (mid > 0) {
      leds[i] = colorWhite;
    } else {
      leds[i] = CRGB::Black;
    }
  }

  for (int i = 210; i < 350; i++) {      
    if (high > 0) {
      leds[i] = colorRaspberrySorbet;
    } else {
      leds[i] = CRGB::Black;
    }
  }
  
  FastLED.show();
}

const unsigned long perMinuteMillis = 60000;
unsigned long nextBeatMillis = 0;
float millisBetweenBeats = 0.0;

void doBpmAnimation() {

  // Negative number means beat has past
  long millisToNextBeat = nextBeatMillis - millis();
  
  int beatsSoFar = round((float)(millis() - bpmStartTime) / (float)millisBetweenBeats);
  int currentBeat_4_4 = (beatsSoFar % 4);
  if (currentBeat_4_4 == 3) {
    // Exponentially slow speed and then kick it back up on the start of the measure
    if (!reverseDirection) {
      rainbowAnimHueSpeed -= 0.25;
      for (int i = 0; i < 5; i++) {
        rainbowRowAnimHueSpeed[i] -= 0.25; 
      }
    } else {
      rainbowAnimHueSpeed += 0.25;
      for (int i = 0; i < 5; i++) {
        rainbowRowAnimHueSpeed[i] += 0.25; 
      }
    }
    
  } else {
    rainbowAnimHueSpeed = bpmStartAnimSpeed;
    for (int i = 0; i < 5; i++) {
      rainbowRowAnimHueSpeed[i] = bpmStartAnimSpeed; 
    }    
  }

  // Brightness flash
  //hsvBrightness = max(0, min(((1.0 - ((float)(200.0 - abs(millisToNextBeat)) / 200.0)) * 100) + 155, 255)); 

//  Serial.print(millis());
//  Serial.print(" ");
//  Serial.println(millisToNextBeat);
  FastLED_FillSolid(0, 0, 0);
  // -12 ms in the past, 12 is in the future
  if (millisToNextBeat > -20 && millisToNextBeat < 20) { 
    #ifdef ANIMATION_DEBUG
    //Serial.println("Show beat");
  #endif    
    showBeat(millisToNextBeat + 8, currentBeat_4_4);
    recalculateNextBeat();
  } else if (millisToNextBeat < -100) { 
    #ifdef ANIMATION_DEBUG
      //Serial.println("Missed beat");
    #endif
    // 100 ms in the past, we missed the beat
//  #ifdef ANIMATION_DEBUG
//    Serial.println("Missed beat");
//  #endif
    recalculateNextBeat();
  }

  // Do the rest of the animation
  if (bpmAnimation == ANIMATION_RAINBOW) {
    fastLedRainbow();
  } else if (bpmAnimation == ANIMATION_RAINBOW_ROW) {
    rainbowRow();
  } 
}

void showBeat(int delayDuration, int idx) {

  // Switch animation directions
  //reverseDirection = !reverseDirection;

  // Simple flash
//  if (idx == 0) {
//    FastLED_FillSolid(255, 0, 0);
//  } else if (idx == 1) {
//    FastLED_FillSolid(0, 255, 0);
//  } else if (idx == 2) {
//    FastLED_FillSolid(255, 0, 255);
//  } else {
//    FastLED_FillSolid(0, 255, 255);
//  } 
//  FastLED.show();
}

uint8_t hsValueScaled() {
  // Only BPM will control hsv value overhead
  if (ledAnimationVal != ANIMATION_BPM_TEST) {
    return 255; // full brightness
  }
  return hsvBrightness;
}

void processBPM(uint8_t newBpm) {

  #ifdef ANIMATION_DEBUG
    Serial.print("BPM ");
    Serial.println(newBpm);
  #endif
  
  bpmStartTime = millis();
  bpm = newBpm;  
  recalculateNextBeat();

  // Make sure we are showing the BPM animation
  if (ledAnimationVal != ANIMATION_BPM_TEST) {
    bpmAnimation = ledAnimationVal;  
    bpmStartAnimSpeed = rainbowRowAnimHueSpeed[0];
  }
  ledAnimationVal = ANIMATION_BPM_TEST;  
}

void processBPMTimeOffset(uint8_t offset) {

  #ifdef ANIMATION_DEBUG
    Serial.print("BPM Offset ");
    Serial.println(offset);
  #endif
  
  bpmOffset = offset;
  recalculateNextBeat();  
  ledAnimationVal = ANIMATION_BPM_TEST;
}

void recalculateNextBeat() {
  // Get the number of milliseconds between beats 
  millisBetweenBeats = (float)perMinuteMillis / (float)bpm;
  float beatsFromStart = (float)(millis() - bpmStartTime - bpmOffset) / millisBetweenBeats;
  // Calculate next beat
  nextBeatMillis = bpmStartTime + round((millisBetweenBeats) * (float)(beatsFromStart + 1.0)); 

//  #ifdef ANIMATION_DEBUG
//     Serial.print("Now ");
//    Serial.print(millis());
//    Serial.print("Millis between beats ");
//    Serial.print(millisBetweenBeats);
//    Serial.print(" beatsFromStart ");
//    Serial.println(beatsFromStart);
//    Serial.print(" nextBeatMillis ");
//    Serial.println(nextBeatMillis);
//  #endif
}
