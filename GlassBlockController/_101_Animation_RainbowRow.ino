/*
  LED animation pattern functions and variables   
*/

#include <FastLED.h>

// #define ANIMATION_RAINBOW_ROW_DEBUG = 1

struct LinearInterpolation* interp_RainbowRow = NULL;
struct BeatSequence* beats_RainbowRow = NULL;
enum RainbowRowPattern pattern = Pattern_None;

// To make the rainbow look best, we make
// each LED 7 diff from the last one
uint8_t colorsInRainbow = 7;

/**
 * Forward declare enum param functions
 */
void setupRainbowRowPattern(enum RainbowRowPattern newPattern);

/**
 * Initialize memory required to run this animation
 */
void init_RainbowRow() {
  interp_RainbowRow = malloc(sizeof(struct LinearInterpolation) * ledsRowCount);  
  beats_RainbowRow = malloc(sizeof(struct BeatSequence));

  // Start with Equal Opp by default
  setupRainbowRowPattern(Pattern_EqualOpp);  
}

/**
 * Sets the default parameters for this animation
 */
void params_RainbowRow(byte params[]) {
  if (interp_RainbowRow == NULL) {
    return;
  }

  // Set the new or existing pattern
  setupRainbowRowPattern(params[2]);

  // Assign the new speed value to all rows
  byte newCoeff = params[3] * 0.25;
  for (int i = 0; i < ledsRowCount; i++) {
    // Keep moving left or right parity
    if (interp_RainbowRow[i].coeff < 0) {
      interp_RainbowRow[i].coeff = -newCoeff;
    } else {
      interp_RainbowRow[i].coeff = newCoeff;
    }
  }
}

/**
 * Set the intial spacing of the pattern
 * to create the visual effect in the name of the pattern
 */
void setupRainbowRowPattern(enum RainbowRowPattern newPattern) {

  if (newPattern == pattern) {
    return;  // No change necessary
  }
  pattern = newPattern;

  // block step is the number of leds to skip
  byte blockStep = 255 / colorsInRainbow;  
  int midRow = ledsRowCount / 2;
  byte twoBlocks = 2 * maxLedsPerBlock;

  for (int i = 0; i < ledsRowCount; i++) { 

     // Set delta hue the same on all of them
    interp_RainbowRow[i].coeff = 1.0;   
    
    // Equal spacing rainbow across each row
    if (pattern == Pattern_Equal) {
      interp_RainbowRow[i].start = (blockStep * i);
    } 
    else if (pattern == Pattern_EqualOpp) {      
      // Here we want the animation to perform like FastLED_Rainbow
      interp_RainbowRow[i].start = twoBlocks * i; 
      if (i % 2 != 0) {  // reverse direction of each row
        interp_RainbowRow[i].coeff = -interp_RainbowRow[i].coeff;
      }      
    } 
    // "/" looking design
    else if (pattern == Pattern_SlantedLeft) {
      interp_RainbowRow[i].start = twoBlocks * (ledsRowCount - i - 1);
    }
    // "\" looking design
    else if (pattern == Pattern_SlantedRight) {
      interp_RainbowRow[i].start = twoBlocks * i;
    }
    // "<" looking design
    else if (pattern == Pattern_ArrowLeft) {
      interp_RainbowRow[i].start = blockStep * ((midRow) - abs(midRow - i)); 
    } 
    // ">" looking design
    else { // if (pattern == Pattern_ArrowRight)
       interp_RainbowRow[i].start = blockStep * abs(midRow - i); 
    }

    #ifdef ANIMATION_RAINBOW_ROW_DEBUG
      Serial.print(interp_RainbowRow[i].start);
      Serial.print(", ");
    #endif
  }
  #ifdef ANIMATION_RAINBOW_ROW_DEBUG
    Serial.println();
    for (int i = 0; i < ledsRowCount; i++) {
      Serial.print(interp_RainbowRow[i].coeff);    
      Serial.print(", ");  
    }    
    Serial.println();
  #endif
}

/**
 * Free up memory used by this animation
 */
void free_RainbowRow() {
  free(interp_RainbowRow);
  interp_RainbowRow = NULL;
  
  free(beats_RainbowRow->sequence);
  free(beats_RainbowRow);
  beats_RainbowRow = NULL;
}

/**
 * @return the rainbow animation type
 */
enum AnimType type_RainbowRow();
enum AnimType type_RainbowRow() {
  return AnimType_RainbowRow;
}

/**
 * Return the beat sequence stored for this animation
 */
BeatSequence* beatSequence_RainbowRow() {
  return beats_RainbowRow;
}

/**
 * Sets the anim struct with the correct function pointers
 */
void setAnimFunc_RainbowRow(struct Animation* anim) {
  anim->type = &type_RainbowRow;
  anim->draw = &draw_RainbowRow;
  anim->params = &params_RainbowRow;
  anim->init = &init_RainbowRow;
  anim->destroy = &free_RainbowRow;
  anim->beat = &beatSequence_RainbowRow;
}

/**
 * Draw the FastLED frame by row
 */
void draw_RainbowRow(uint16_t beatNumInMeasure) {
  for (int row = 0; row < ledsRowCount; row++) {
    int hue = linearInterp255(interp_RainbowRow[row], beatNumInMeasure);
    for (int col = 0; col < ledsPerRow; col++) {
      if (pattern == Pattern_EqualOpp) {
        hue = (hue + colorsInRainbow) % 255;    
      } else {
        hue = (hue + abs(interp_RainbowRow[row].coeff)) % 255;
      }
      FastLED_SetHue(to_led_idx(row, col), hue);
    }
  }
}
