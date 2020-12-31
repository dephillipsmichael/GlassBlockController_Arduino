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
// The LED flow stay on the same row when they go from 
// the front plane to left plane, so in the 349-280
// instance, left plane is LEDs 359-331 and
// front plane starts around LED 330-280.
// See function to_led_idx_set for more details.
//

#include <FastLED.h>

#define DATA_PIN    5
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB
#define NUM_LEDS    350
CRGB leds[NUM_LEDS];

// This is a limiter to not draw too much power from the system
#define MAX_BRIGHTNESS 200

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
// These are the constants to iterate over LEDs a row at a time
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

void FastLED_SetHueVal(int ledIdx, uint8_t hue, uint8_t val) {  
  leds[ledIdx] = CHSV(hue, 255, val);
}

// Sets an LED index to a hue val at full saturation, and global brightness
void FastLED_SetRGB(int ledIdx, uint8_t r, uint8_t g, uint8_t b) {
  leds[ledIdx] = CRGB(r, g, b);
}


// Glass block buffer
int* blockLeds = new int[maxLedsPerBlock];
void FastLED_lightBlockHue(uint8_t row, uint8_t col, uint8_t hue) {
  to_led_idx_set(row, col, blockLeds);
  for (int led = 0; led < maxLedsPerBlock; led++) {
    if (blockLeds[led] != IGNORE_LED) {  // Black out LED in block
      leds[blockLeds[led]] = CHSV(hue, 255, 255);
    }
  }
}

void FastLED_lightBlockHueVal(uint8_t row, uint8_t col, uint8_t hue, uint8_t val) {
  to_led_idx_set(row, col, blockLeds);
  for (int led = 0; led < maxLedsPerBlock; led++) {
    if (blockLeds[led] != IGNORE_LED) {  // Black out LED in block
      leds[blockLeds[led]] = CHSV(hue, 255, val);
    }
  }
}

// Glass block buffer
void FastLED_lightBlockHueSat(uint8_t row, uint8_t col, uint8_t hue, uint8_t sat) {
  to_led_idx_set(row, col, blockLeds);
  for (int led = 0; led < maxLedsPerBlock; led++) {
    if (blockLeds[led] != IGNORE_LED) {  // Black out LED in block
      leds[blockLeds[led]] = CHSV(hue, sat, 255);
    }
  }
}

void FastLED_lightBlockHueSatVal(uint8_t row, uint8_t col, uint8_t hue, uint8_t sat, uint8_t val) {
  to_led_idx_set(row, col, blockLeds);
  for (int led = 0; led < maxLedsPerBlock; led++) {
    if (blockLeds[led] != IGNORE_LED) {  // Black out LED in block
      leds[blockLeds[led]] = CHSV(hue, sat, val);
    }
  }
}

void FastLED_lightBlockRGB(uint8_t row, uint8_t col, uint8_t red, uint8_t green, uint8_t blue) {
  to_led_idx_set(row, col, blockLeds);
  for (int led = 0; led < maxLedsPerBlock; led++) {
    if (blockLeds[led] != IGNORE_LED) {  // Black out LED in block
      leds[blockLeds[led]] = CRGB(red, green, blue);
    }
  }
}

const uint16_t LED_TEST_SET[] = {349, 348, 347, 346, 345, 344, 343, 342, 341, 340, 339, 338, 337, 336, 335, 334, 333, 332, 331, 330, 329, 328, 327, 326, 325, 324, 323, 322, 321, 320, 319, 318, 317, 316, 315, 314, 313, 312, 311, 310, 309, 308, 307, 306, 305, 304, 303, 302, 301, 300, 299, 298, 297, 296, 295, 294, 293, 292, 291, 290, 289, 288, 287, 286, 285, 284, 283, 282, 281, 280, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223, 224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239, 240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255, 256, 257, 258, 259, 260, 261, 262, 263, 264, 265, 266, 267, 268, 269, 270, 271, 272, 273, 274, 275, 276, 277, 278, 279, 209, 208, 207, 206, 205, 204, 203, 202, 201, 200, 199, 198, 197, 196, 195, 194, 193, 192, 191, 190, 189, 188, 187, 186, 185, 184, 183, 182, 181, 180, 179, 178, 177, 176, 175, 174, 173, 172, 171, 170, 169, 168, 167, 166, 165, 164, 163, 162, 161, 160, 159, 158, 157, 156, 155, 154, 153, 152, 151, 150, 149, 148, 147, 146, 145, 144, 143, 142, 141, 140, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 69, 68, 67, 66, 65, 64, 63, 62, 61, 60, 59, 58, 57, 56, 55, 54, 53, 52, 51, 50, 49, 48, 47, 46, 45, 44, 43, 42, 41, 40, 39, 38, 37, 36, 35, 34, 33, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0};
const uint16_t BLOCK_TEST_SETS[blocksPerRow * ledsRowCount][maxLedsPerBlock] = {{349, 348, 347, 346, 345, -1}, {344, 343, 342, 341, 340, 339}, {338, 337, 336, 335, 334, -1}, {333, 332, 331, 330, 329, 328}, {327, 326, 325, 324, 323, 322}, {321, 320, 319, 318, 317, 316}, {315, 314, 313, 312, 311, 310}, {309, 308, 307, 306, 305, 304}, {303, 302, 301, 300, 299, 298}, {297, 296, 295, 294, 293, 292}, {291, 290, 289, 288, 287, 286}, {285, 284, 283, 282, 281, 280}, {210, 211, 212, 213, 214, -1}, {215, 216, 217, 218, 219, 220}, {221, 222, 223, 224, 225, -1}, {226, 227, 228, 229, 230, 231}, {232, 233, 234, 235, 236, 237}, {238, 239, 240, 241, 242, 243}, {244, 245, 246, 247, 248, 249}, {250, 251, 252, 253, 254, 255}, {256, 257, 258, 259, 260, 261}, {262, 263, 264, 265, 266, 267}, {268, 269, 270, 271, 272, 273}, {274, 275, 276, 277, 278, 279}, {209, 208, 207, 206, 205, -1}, {204, 203, 202, 201, 200, 199}, {198, 197, 196, 195, 194, -1}, {193, 192, 191, 190, 189, 188}, {187, 186, 185, 184, 183, 182}, {181, 180, 179, 178, 177, 176}, {175, 174, 173, 172, 171, 170}, {169, 168, 167, 166, 165, 164}, {163, 162, 161, 160, 159, 158}, {157, 156, 155, 154, 153, 152}, {151, 150, 149, 148, 147, 146}, {145, 144, 143, 142, 141, 140}, {70, 71, 72, 73, 74, -1}, {75, 76, 77, 78, 79, 80}, {81, 82, 83, 84, 85, -1}, {86, 87, 88, 89, 90, 91}, {92, 93, 94, 95, 96, 97}, {98, 99, 100, 101, 102, 103}, {104, 105, 106, 107, 108, 109}, {110, 111, 112, 113, 114, 115}, {116, 117, 118, 119, 120, 121}, {122, 123, 124, 125, 126, 127}, {128, 129, 130, 131, 132, 133}, {134, 135, 136, 137, 138, 139}, {69, 68, 67, 66, 65, -1}, {64, 63, 62, 61, 60, 59}, {58, 57, 56, 55, 54, -1}, {53, 52, 51, 50, 49, 48}, {47, 46, 45, 44, 43, 42}, {41, 40, 39, 38, 37, 36}, {35, 34, 33, 32, 31, 30}, {29, 28, 27, 26, 25, 24}, {23, 22, 21, 20, 19, 18}, {17, 16, 15, 14, 13, 12}, {11, 10, 9, 8, 7, 6}, {5, 4, 3, 2, 1, 0}}; 

void setup() {
  Serial.begin(9600);
  while (!Serial);
  runUnitTests();
}

void loop() {
  
}

void runUnitTests() {
{
    // Test the individual LED index calculation
    uint16_t testIdx = 0;
    uint16_t ledIdx = 0; 
    uint8_t failedCount = 0;
    for (uint16_t row = 0; row < ledsRowCount; row++) {
      for (uint16_t col = 0; col < ledsPerRow; col++) {        
        ledIdx = to_led_idx(row, col);
        if (!testEquality(LED_TEST_SET[testIdx], ledIdx)) {
          Serial.print("for (");
          Serial.print(row);
          Serial.print(", ");
          Serial.print(col);
          Serial.print(")");
          Serial.println();
          failedCount = failedCount + 1;
        }
        testIdx = testIdx + 1;
      }  
    }
    if (failedCount == 0) {
      Serial.println("All LED tests completed successfully.");
    }
}

  {
  // Test the glass block LED sets
  uint16_t testIdx = 0;
  uint8_t failedCount = 0;
  int* ledSet = new int[maxLedsPerBlock];
  for (uint16_t row = 0; row < ledsRowCount; row++) {
    for (uint16_t col = 0; col < blocksPerRow; col++) {
      to_led_idx_set(row, col, ledSet);
      for (int i = 0; i < maxLedsPerBlock; i++) {
        if (!testEquality(BLOCK_TEST_SETS[testIdx][i], ledSet[i])) {
          Serial.print("for (");
          Serial.print(row);
          Serial.print(", ");
          Serial.print(col);
          Serial.print(", ");
          Serial.print(i);
          Serial.print(")");
          Serial.println();
          failedCount = failedCount + 1;
        }
      }
      testIdx = testIdx + 1;
    }      
  }
  if (failedCount == 0) {
    Serial.println("All Glass Block tests completed successfully.");
  }
  }  
}

bool testEquality(uint16_t expected, uint16_t actual) {
  if (expected != actual) {
    Serial.print("LED test failed: expected ");
    Serial.print(expected);
    Serial.print(", actual ");
    Serial.print(actual);
    return false;
  }
  return true;
}
