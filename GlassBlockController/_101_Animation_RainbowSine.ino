/*
  LED animation pattern functions and variables   
*/

#include <FastLED.h>

struct LinearInterpolation* interp_RainbowSine = NULL;
struct BeatSequence* beats_RainbowSine = NULL;

/**
 * Initialize memory required to run this animation
 */
void init_RainbowSine() {
  interp_RainbowSine = malloc(sizeof(struct LinearInterpolation));
  interp_RainbowSine->start = 0.0;
  interp_RainbowSine->coeff = 1.0;  // delta hue
  
  beats_RainbowSine = malloc(sizeof(struct BeatSequence));   
}

/**
 * Free up memory used by this animation
 */
void free_RainbowSine() {
  free(interp_RainbowSine);
  interp_RainbowSine = NULL;
  
  free(beats_RainbowSine->sequence);
  free(beats_RainbowSine);
  beats_RainbowSine = NULL;
}

/**
 * @return the rainbow animation type
 */
enum AnimType type_RainbowSine();
enum AnimType type_RainbowSine() {
  return AnimType_RainbowSine;
}

/**
 * Sets the default parameters for this animation
 */
void params_RainbowSine(byte params[]) {
  if (interp_RainbowSine == NULL) {
    return;
  }

  byte newCoeff = params[0] * 0.25;
  interp_RainbowSine->coeff = newCoeff;
}


/**
 * Return the beat sequence stored for this animation
 */
BeatSequence* beatSequence_RainbowSine() {
  return beats_RainbowSine;
}

/**
 * Sets the anim struct with the correct function pointers
 */
void setAnimFunc_RainbowSine(struct Animation* anim) {
  anim->type = &type_RainbowSine;
  anim->draw = &draw_RainbowSine;
  anim->params = &params_RainbowSine;
  anim->init = &init_RainbowSine;
  anim->destroy = &free_RainbowSine;
  anim->beat = &beatSequence_RainbowSine;
}

/**
 * Draw the next rainbow frame
 */
void draw_RainbowSine(uint16_t beatNum) { 
  // fill_rainbow(leds, num_leds, starting hue, delta hue)  
  // fill_rainbow(leds, NUM_LEDS, linearInterp255(iterp_RainbowSine, beatNum), (byte)interp_RainbowSine->coeff);
}

//void rainbowSine() {
//  // The Rainbow Sine pattern moves hue by up the rows, then down the next row, etc.
//
//  float hue = rainbowAnimHue;
//  float sat = 255;
//  int colLoops = ledsPerRow;
//  if (currentPattern == rainbowPatternSineBlock) {
//    colLoops = blocksPerRow;
//  }
//  for (int col = 0; col < colLoops; col++) {    
//    // Even rows flow up
//    if ((col % 2) == 0) {
//      for (int row = 0; row < blocksRowCount; row++) {
//        sat = 255;
//        hue += rainbowRowAnimHueSpeed[0];
//        if (hue > 255) {
//          hue = hue - 255;
//        }
//        if (currentPattern == rainbowPatternSineBlock) {          
//          if ((hue >= 0 && hue < 10) ||
//              (hue >= 100 && hue < 110) ||
//              (hue >= 200 && hue < 210))  {
//            sat = 200;
//          }
//          FastLED_lightBlockHueSat(row, col, hue, sat);
//        } else {
//          FastLED_SetHueVal(to_led_idx(row, col), hue, hsValueScaled());
//        }
//      }  
//    } else { // Odd rows flow down
//      for (int row = (blocksRowCount - 1); row >= 0; row--) {
//        sat = 255;
//        hue += rainbowRowAnimHueSpeed[0];
//        if (hue > 255) {
//          hue = hue - 255;
//        }
//        if (currentPattern == rainbowPatternSineBlock) {
//          if ((hue >= 0 && hue < 10) ||
//              (hue >= 100 && hue < 110) ||
//              (hue >= 200 && hue < 210)) {
//            sat = 200;
//          }
//          FastLED_lightBlockHueSatVal(row, col, hue, sat, hsValueScaled());
//        } else {
//          FastLED_SetHueVal(to_led_idx(row, col), hue, hsValueScaled());
//        }
//      }  
//    }
//  }
//}
