/*
  LED animation pattern functions and variables   
*/
#include <FastLED.h>

#define ANIMATION_OFF 0
#define ANIMATION_RAINBOW 1
#define ANIMATION_RAINBOW_ROW 2

byte rainbowRed = 33;
byte rainbowGreen = 99;
byte rainbowBlue = 133;

byte rainbowRowRed = 33;
byte rainbowRowGreen = 99;
byte rainbowRowBlue = 134;

// The current hue value for the rainbow animation
float rainbowAnimHue = 0.0;
float rainbowAnimHueSpeed = 2.0; 

// The current hue value for the rainbow per row animation
// Another good starting one is { 30.0, 36.0, 42.0, 36.0, 30.0 };
float rainbowRowAnimHue[] = { 0.0, 50.0, 100.0, 150.0, 200.0 };
float rainbowRowAnimHueSpeed[] = { 2.0, 2.0, 2.0, 2.0, 2.0 };

CRGB colorNautical = CRGB(0, 255, 198);
CRGB colorRaspberrySorbet = CRGB(255, 0, 127);
CRGB colorWhite = CRGB(255, 255, 255);
CRGB colorLavender = CRGB(158, 0, 255);
CRGB colorYellow = CRGB(255, 194, 9);

byte animationAlpha = 1;

// The average Lo
int* avgLowMidHigh = new int[3];

byte ledledAnimationVal = 0;

void animationLoop() {
  if (ledledAnimationVal == ANIMATION_RAINBOW) {
    fastLedRainbow();
  } else if (ledledAnimationVal == ANIMATION_RAINBOW_ROW) {
    rainbowRow();
  }   
}

// Process ARGB animation command
void processArgb(byte alpha, byte r, byte g, byte b) {   

  if (animationAlpha == alpha &&
     rainbowRed == r &&
     rainbowGreen == g &&
     rainbowBlue == b) {

    ledledAnimationVal = ANIMATION_RAINBOW;        
    return;      
  }

  if (animationAlpha == alpha &&
     rainbowRowRed == r &&
     rainbowRowGreen == g &&
     rainbowRowBlue == b) {

    ledledAnimationVal = ANIMATION_RAINBOW_ROW;        
    return;      
  }

  // Hue 
  if (1 == alpha && 0 == r && 0 == g) {  
    rainbowAnimHueSpeed = (float)((uint8_t)b) * 0.25;

    for (int i = 0; i < blocksRowCount; i++) {
      rainbowRowAnimHueSpeed[i] = rainbowAnimHueSpeed;
    }
    return;
  }

  // Make sure the brightness is relative to the max in this code
  setGlobalBrightness(alpha);  

  // Sending black will just control the global brightness 
  if (0 == r && 0 == g && 0 == b) {            
      return;      
  }

  ledledAnimationVal = ANIMATION_OFF;
  
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
           lightBlockRGB(rowAdjusted, col, 158, 0, 255);
        } else if (rowAdjusted == 3) {
           lightBlockRGB(rowAdjusted, col, 0, 255, 198);
        } else if (rowAdjusted == 2) {
           lightBlockRGB(rowAdjusted, col, 255, 194, 9);
        } else if (rowAdjusted == 1) {
           lightBlockRGB(rowAdjusted, col, 255, 0, 127);
        } else if (rowAdjusted == 0) {
           lightBlockRGB(rowAdjusted, col, 255, 255, 255);
        }        
      } else {
        lightBlockRGB(rowAdjusted, col, 50, 50, 50);
      }
    }
  }

  for (int col = 3; col < blocksPerRow; col++) {
    for (int row = 0; row < blocksRowCount; row++) {
      uint8_t rowAdjusted = (ledsRowCount - 1) - row;
      if (unpackedEqVals[col - 3] > row) {
        if (rowAdjusted == 4) {
           lightBlockRGB(rowAdjusted, col, 158, 0, 255);
        } else if (rowAdjusted == 3) {
           lightBlockRGB(rowAdjusted, col, 0, 255, 198);
        } else if (rowAdjusted == 2) {
           lightBlockRGB(rowAdjusted, col, 255, 194, 9);
        } else if (rowAdjusted == 1) {
           lightBlockRGB(rowAdjusted, col, 255, 0, 127);
        } else if (rowAdjusted == 0) {
           lightBlockRGB(rowAdjusted, col, 255, 255, 255);
        }       
      } else {
        lightBlockRGB(rowAdjusted, col, 50, 50, 50);
      }
    }
  }
  
  FastLED.show(); 
}

void fastLedRainbow () {
  rainbowAnimHue += rainbowAnimHueSpeed;
  if (rainbowAnimHue > 255) {
    rainbowAnimHue = rainbowAnimHue - 255;
  }
  // FastLED's built-in rainbow generator
  fill_rainbow( leds, NUM_LEDS, (byte)rainbowAnimHue, 7);
  
  // send the 'leds' array out to the actual LED strip
  FastLED.show(); 
}

// Used by function rainbowRow
float iteratorHues[] = { 0.0, 0.0, 0.0, 0.0, 0.0 };
void rainbowRow() {

  // Incrament all the row's hue values
  for (int i = 0; i < blocksRowCount; i++) {
    rainbowRowAnimHue[i] += rainbowRowAnimHueSpeed[i];
    if (rainbowRowAnimHue[i] > 255) {
      rainbowRowAnimHue[i] = rainbowRowAnimHue[i] - 255;
    }
    // Copy over current hue values to iterator
    iteratorHues[i] = rainbowRowAnimHue[i];
  }
  
  // This loop runs the LEDS a pixel at a time all in the same direction a row at a time
  // Starts at the bottom row and end on the top row
  // TODO: mdephillips 12/20/20 move this to a function pointer to remove dupe code
  int rowIdx = 0;
  for (int i = 0; i < LED_ROW_INDEX_SIZE; i+=LED_ROW_INDEX_COUNT) {
    rowIdx = (i / LED_ROW_INDEX_COUNT);
    if (ledRowIndexes[i+2] < 0) {
      for (int led = ledRowIndexes[i]; led >= ledRowIndexes[i+1]; led+=ledRowIndexes[i+2]) {
        iteratorHues[rowIdx] += rainbowRowAnimHueSpeed[rowIdx];
        FastLED_SetHue(led, iteratorHues[rowIdx]);
      }
    } else {
      for (int led = ledRowIndexes[i]; led < ledRowIndexes[i+1]; led+=ledRowIndexes[i+2]) {
        iteratorHues[rowIdx] += rainbowRowAnimHueSpeed[rowIdx];
        FastLED_SetHue(led, iteratorHues[rowIdx]);
      }
    }
  }

  //

  // send the 'leds' array out to the actual LED strip
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
      leds[i] = colorRaspberrySorbet;
    } else {
      leds[i] = CRGB::Black;
    }
  }

  for (int i = 210; i < 350; i++) {      
    if (high > 0) {
      leds[i] = colorWhite;
    } else {
      leds[i] = CRGB::Black;
    }
  }
  
  FastLED.show();
}

// Glass block buffer
int* blockLeds = new int[maxLedsPerBlock];
void lightBlock(uint8_t row, uint8_t col, uint8_t hue, uint8_t val) {
  to_led_idx_set(row, col, blockLeds);
  for (int led = 0; led < maxLedsPerBlock; led++) {
    if (blockLeds[led] != IGNORE_LED) {  // Black out LED in block
      leds[blockLeds[led]] = CHSV(hue, 255, val);
    }
  }
}

// Glass block buffer
void lightBlockRGB(uint8_t row, uint8_t col, uint8_t red, uint8_t green, uint8_t blue) {
  to_led_idx_set(row, col, blockLeds);
  for (int led = 0; led < maxLedsPerBlock; led++) {
    if (blockLeds[led] != IGNORE_LED) {  // Black out LED in block
      leds[blockLeds[led]] = CRGB(red, green, blue);
    }
  }
}
