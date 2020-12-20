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
//       _
//      | |
// LEFT | |_________
//      |___________|
//          FRONT
//
// The LED matrix is 70 LEDs wide by 5 LEDs high.
// The first LED is in the bottom right corner,
// and they are connected in this flow pattern.
//
//     349 <... 282 < 281 < 280
//                          |
//                          .
//                          |
//     210 > 211 > 212 >... 279
//     |
//     .
//     |
//     209 <... 142 < 141 < 140
//                         |
//                         .
//                         |
//     70 > 71 > 72 >... 139
//     |
//     .
//     |
//     69 <... 3 < 2 < 1 < 0
//
//

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

const uint8_t blockSizeInInches = 8; // About 8 inches is the size of each block

// Used to communicate to to_led_idx_set function caller about unused array result index
const int IGNORE_LED = -1;

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
