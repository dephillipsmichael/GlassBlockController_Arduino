// The glass block bar is an L-shaped design
// The front facing plane is 9 x 5 blocks (w x h),
// The left facing plane is 3 x 5 blocks
//
// Each glass block has 5-6 LEDs embedded in the top of 
// them facing down. 
//
// Here is a beautiful ASCII art rendering of the design:
//
//  Overhead:
//       _          |
//      | | BAR TOP |    
// LEFT | |_________|
//      |___________|
//          FRONT
//

// The LED matrix is 70 LEDs wide by 5 LEDs high.
// The first LED is in the bottom right corner,
// and they are connected in this flow pattern.
//
//     349 <... 282 < 281 < 280
//                           |
//                           .
//                           |
//     210 > 211 > 212 >... 279
//     |
//     .
//     |
//     209 <... 142 < 141 < 140
//                           |
//                           .
//                           |
//     70 > 71 > 72 >...   139
//     |
//     .
//     |
//     69 <... 3 < 2 < 1  <  0
//
//

#include <FastLED.h>

#define DATA_PIN    5
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB
#define NUM_LEDS    350
CRGB leds[NUM_LEDS];

// LEDs per a row
const uint16_t ledsPerRow = 70;
// Number of rows in the wall
const uint16_t ledsRowCount = 5;

const uint16_t ledCount = ledsPerRow * ledsRowCount;

// This array represents a row of blocks and how many LEDs are in each block
const uint8_t maxLedsPerBlock = 6;
const uint8_t blocksPerRow = 12;
const uint8_t blocksRowCount = ledsRowCount;
const uint8_t ledsPerBlock[blocksPerRow] = { 5, 6, 5, 6, 6, 6, 6, 6, 6, 6, 6, 6 };

// The front plane is the side facing out
const uint8_t frontPlaneBlocksPerRow = 9;
const uint8_t sidePlaneBlocksPerRow = 3;

// About 8x8 inches by 3.25 inches thick is the size of each glass block
const uint8_t blockSizeInInches = 8; 

// The LED index ranges - see comments in class header
// These are the constants to iterate oer leds a row at a time
// in the same direction.  Whereas if you iterate normally,
// the animations would snake like in the class header wiring.
// When running this code:
// for (int i = 0; i < 70; i++) {     // Bottom row of glass blocks
// for (int i = 139; i >= 70; i--) { 
// for (int i = 140; i < 210; i++) {
// for (int i = 279; i >= 210; i--) {
// for (int i = 280; i < 350; i++) {  // Top row of glass blocks
const uint8_t LED_ROW_INDEX_COUNT = 3;
const uint8_t LED_ROW_INDEX_SIZE = LED_ROW_INDEX_COUNT * blocksRowCount;
const int ledRowIndexes[] = { 0, 70, 1, 139, 70, -1, 140, 210, 1, 279, 210, -1, 280, 350, 1 };

// The current global brightness, always < MAX_BRIGHTNESS
uint8_t globalBrightness = 100;
void setGlobalBrightness(uint8_t brightness) {
  globalBrightness = brightness;
  FastLED.setBrightness(globalBrightnessScaled());
}

// This is a limiter to not draw too much power from the system
#define MAX_BRIGHTNESS 200

// Used to communicate to to_led_idx_set function caller not to use array index
const int IGNORE_LED = -1;

int globalBrightnessScaled() {
  return ((float)globalBrightness / 255.0) * MAX_BRIGHTNESS;
}

// Sets up the data pins for controlling the LEDs
void setupFastLed() {
  // tell FastLED about the LED strip configuration
  FastLED.addLeds<LED_TYPE,DATA_PIN,COLOR_ORDER>(leds, NUM_LEDS);

  // Set master brightness control
  FastLED.setBrightness(globalBrightnessScaled());
  
  FastLED.clear();
  FastLED.show(); 
}

// Each glass block has 5-6 LEDs embedded in them
// When you specify a glass block to light up, 
// this function will return a set of LED indexes to light up.
//
// (0, 0) is upper left for left facing plane, (0, 3) is upper left for front facing plane
//
// param: block_row row index for glass block
// param: block_col column index for glass block
// param: result pre-allocated array (needs size of 6) where the results will be stored
void to_led_idx_set(uint16_t block_row, uint16_t block_col, int* result) {

  // Bounds check bad param data
  if (block_row >= blocksRowCount || block_col >= blocksPerRow) {
    for (int i = 0; i < maxLedsPerBlock; i++) {
      result[i] = IGNORE_LED;
    }
    return result; 
  }
  
  // Compute the column of the left-most pixel in the block
  // The first 3 blocks have LED count of 5, 6, and 5, but then the rest are 6.
  uint8_t ledCol = 0;
  for (int i = 0; i < block_col; i++) {
    ledCol = ledCol + ledsPerBlock[i];
  }
  uint16_t ledIndex = to_led_idx(block_row, ledCol);
  
  uint8_t ledsInThisBlock = ledsPerBlock[block_col];
  for (int i = 0; i < ledsInThisBlock; i++) {
    if (block_row % 2 == 0) { // Even rows (0, 2, 4) run backwards
      result[i] = ledIndex - i;
    } else {
      result[i] = ledIndex + i;
    }
  }
  for (int i = ledsInThisBlock; i < maxLedsPerBlock; i++) {
    result[i] = IGNORE_LED;
  }
}

// Due to the way the LEDs were wired, there is a 
// tranlsation that needs done to convert the usual row and col
// upper left coordinate (0, 0) system to an LED index
//
// param: led_row row index for LED
// param: led_col column index for LED
// return: the index used in the FastLED library to designate the LED at this row, column
uint16_t to_led_idx(uint16_t led_row, uint16_t led_col) {
  uint16_t result;
  if (led_row % 2 == 0) { // Even rows (0, 2, 4) run backwards
    result = (ledCount - 1) - (((led_row + 1) * ledsPerRow) + (led_col - ledsPerRow));
  } else {
    result = (ledCount) - (((led_row + 1) * ledsPerRow) - led_col);
  }
  return result;
}

void FastLED_FillSolid(byte r, byte g, byte b) {
  fill_solid(leds, NUM_LEDS, CRGB(r, g, b));
}

// Sets an LED index to a hue val at full saturation, and global brightness
void FastLED_SetHue(int ledIdx, uint8_t hue) {
  leds[ledIdx] = CHSV(hue, 255, globalBrightnessScaled());
}

// Sets an LED index to a hue val at full saturation, and global brightness
void FastLED_SetRGB(int ledIdx, uint8_t r, uint8_t g, uint8_t b) {
  FastLED.setBrightness(globalBrightnessScaled());
  leds[ledIdx] = CRGB(r, g, b);
}


// Glass block buffer
int* blockLeds = new int[maxLedsPerBlock];
void FastLED_lightBlockHue(uint8_t row, uint8_t col, uint8_t hue) {
  to_led_idx_set(row, col, blockLeds);
  for (int led = 0; led < maxLedsPerBlock; led++) {
    if (blockLeds[led] != IGNORE_LED) {  // Black out LED in block
      leds[blockLeds[led]] = CHSV(hue, 255, globalBrightnessScaled());
    }
  }
}

// Glass block buffer
void FastLED_lightBlockHueSat(uint8_t row, uint8_t col, uint8_t hue, uint8_t sat) {
  to_led_idx_set(row, col, blockLeds);
  for (int led = 0; led < maxLedsPerBlock; led++) {
    if (blockLeds[led] != IGNORE_LED) {  // Black out LED in block
      leds[blockLeds[led]] = CHSV(hue, sat, globalBrightnessScaled());
    }
  }
}

// Glass block buffer
void FastLED_lightBlockRGB(uint8_t row, uint8_t col, uint8_t red, uint8_t green, uint8_t blue) {
  to_led_idx_set(row, col, blockLeds);
  for (int led = 0; led < maxLedsPerBlock; led++) {
    if (blockLeds[led] != IGNORE_LED) {  // Black out LED in block
      leds[blockLeds[led]] = CRGB(red, green, blue);
    }
  }
}
