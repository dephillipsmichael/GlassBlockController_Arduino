/*
  LED animation pattern functions and variables   
*/

#include <FastLED.h>

struct LinearInterpolation* interp_Squares = NULL;
struct BeatSequence* beats_Squares = NULL;

/**
 * Initialize memory required to run this animation
 */
void init_Squares() {
  interp_Squares = malloc(sizeof(struct LinearInterpolation));
  interp_Squares->start = 0.0;
  interp_Squares->coeff = 1.0;  // delta hue
  
  beats_Squares = malloc(sizeof(struct BeatSequence));   
}

/**
 * Free up memory used by this animation
 */
void free_Squares() {
  free(interp_Squares);
  interp_Squares = NULL;
  
  free(beats_Squares->sequence);
  free(beats_Squares);
  beats_Squares = NULL;
}

/**
 * @return the rainbow animation type
 */
 enum AnimType type_Squares();
enum AnimType type_Squares() {
  return AnimType_Squares;
}

/**
 * Return the beat sequence stored for this animation
 */
BeatSequence* beatSequence_Squares() {
  return beats_Squares;
}

/**
 * Sets the anim struct with the correct function pointers
 */
void setAnimFunc_Squares(struct Animation* anim) {
  anim->type = &type_Squares;
  anim->draw = &draw_Squares;
  anim->init = &init_Squares;
  anim->destroy = &free_Squares;
  anim->beat = &beatSequence_Squares;
}

/**
 * Draw the next rainbow frame
 */
void draw_Squares(uint16_t beatNum) { 
  // TODO translate algorithm below
}

//void rainbowCircle() {
//  // The Rainbow circle patterns starts color in middle of wall and goes outwards equally
//  // See Hypno-Toad eyes for reference
//
//  float hue = rainbowAnimHue + 128;
//  int circleDiameter = 1;
//  int circleDiameterMax = 5;
//  int midPointCol = 1;
//  int midPointRow = 2;
//  int row = midPointCol;
//  int col = midPointRow;
//
//  // Hard-coded 5 circles fit inside front facing plane
//  while (circleDiameter <= circleDiameterMax) {
//
//    row = midPointRow - (circleDiameter / 2);
//    col = midPointCol - (circleDiameter / 2);    
//
//    for (int i = 0; i < circleDiameter; i++) {
//      // Top row of circle
//      FastLED_lightBlockHue(row, col + i, hue);
//      // Left column of circle
//      FastLED_lightBlockHue(row + i, col, hue);
//      // Bottom row of circle
//      FastLED_lightBlockHue(row + (circleDiameter -1), col + i, hue);
//      // Right column of circle
//      FastLED_lightBlockHue(row + i, col + (circleDiameter - 1), hue);
//    }
//    
//    circleDiameter += 2;
//    hue += rainbowRowAnimHueSpeed[0];
//    if (hue > 255) {
//      hue = hue - 255;
//    }
//  }
//
//  hue = rainbowAnimHue;
//  circleDiameter = 1;
//  circleDiameterMax = 5;
//  midPointCol = 5;
//  midPointRow = 2;
//  row = midPointCol;
//  col = midPointRow;
//
//  // Hard-coded 5 circles fit inside front facing plane
//  while (circleDiameter <= circleDiameterMax) {
//
//    row = midPointRow - (circleDiameter / 2);
//    col = midPointCol - (circleDiameter / 2);    
//
//    for (int i = 0; i < circleDiameter; i++) {
//      // Top row of circle
//      FastLED_lightBlockHue(row, col + i, hue);
//      // Left column of circle
//      FastLED_lightBlockHue(row + i, col, hue);
//      // Bottom row of circle
//      FastLED_lightBlockHue(row + (circleDiameter -1), col + i, hue);
//      // Right column of circle
//      FastLED_lightBlockHue(row + i, col + (circleDiameter - 1), hue);
//    }
//    
//    circleDiameter += 2;
//    hue += rainbowRowAnimHueSpeed[0];
//    if (hue > 255) {
//      hue = hue - 255;
//    }
//  }
//
//  hue = rainbowAnimHue;
//  circleDiameter = 1;
//  circleDiameterMax = 5;
//  midPointCol = 9;
//  midPointRow = 2;
//  row = midPointCol;
//  col = midPointRow;
//
//  // Hard-coded 5 circles fit inside front facing plane
//  while (circleDiameter <= circleDiameterMax) {
//
//    row = midPointRow - (circleDiameter / 2);
//    col = midPointCol - (circleDiameter / 2);    
//
//    for (int i = 0; i < circleDiameter; i++) {
//      // Top row of circle
//      FastLED_lightBlockHue(row, col + i, hue);
//      // Left column of circle
//      FastLED_lightBlockHue(row + i, col, hue);
//      // Bottom row of circle
//      FastLED_lightBlockHue(row + (circleDiameter -1), col + i, hue);
//      // Right column of circle
//      FastLED_lightBlockHue(row + i, col + (circleDiameter - 1), hue);
//    }
//    
//    circleDiameter += 2;
//    hue += rainbowRowAnimHueSpeed[0];
//    if (hue > 255) {
//      hue = hue - 255;
//    }
//  }
//}
