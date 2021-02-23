/*
  LED animation pattern functions and variables   
*/

#include <FastLED.h>

#define DEBUG_RAINBOW_ANIM 1

// Negligible memory usage, define globally
byte rainbowAnim_hue;

/**
 * Initialize memory required to run this animation
 */
void init_RainbowAnim() {  
  #ifdef DEBUG_RAINBOW_ANIM
    Serial.println("init rainbow anim");
  #endif
  rainbowAnim_hue = 0;
}

BeatSequence* beatSequence_RainbowAnim() {
  // This animation is not compatible with bpm timing,
  // because it needs to run a full speed non-stop to look best
  return NULL;
}

/**
 * Free up memory used by this animation
 */
void free_RainbowAnim() {
  rainbowAnim_hue = 0;
}

/**
 * @return the rainbow animation type
 */
enum AnimType type_RainbowAnim();
enum AnimType type_RainbowAnim() {
  return AnimType_Rainbow;
}

void params_RainbowAnim(byte params[]) {
  // no params for rainbow
}

/**
 * Sets the anim struct with the correct function pointers
 */
void setAnimFunc_RainbowAnim(struct Animation* anim) {
  anim->type = &type_RainbowAnim;
  anim->draw = &draw_RainbowAnim;
  anim->params = &params_RainbowAnim;
  anim->init = &init_RainbowAnim;
  anim->destroy = &free_RainbowAnim;
  anim->beat = &beatSequence_RainbowAnim;
}

void draw_RainbowAnim(uint16_t beatNumInMeasure) {
  FastLED_Rainbow(rainbowAnim_hue);
  rainbowAnim_hue++;
}
