/*
  LED animation pattern functions and variables   
*/

#include <FastLED.h>

#define ANIMATION_DEBUG = 1

/**
 * Holds a pointer to an array of animation objects
 */
struct Animation* animations = NULL;
uint8_t animationCount = 0;

// Keeps track of running animation idx
const uint16_t animationControllerStartBeat = 255 + 255 + 1;
uint16_t animationFrameIdx = animationControllerStartBeat;

/**
 * Spin up resources used by this controller
 */
void initController_Animation() {
  if (animationCount > 0) {
    #ifdef ANIMATION_DEBUG
      Serial.println("Anim controller init() already called");
    #endif
    return; // Already initialized
  }

  // Start at the beginning of the animation
  animationFrameIdx = animationControllerStartBeat;

  // Create animation size of 1
  animations = malloc(sizeof(Animation) * 1);
  animationCount = 1;
  
  // Default to SimpleFade animation
  setAnimFunc_SimpleFade(&animations[0]);
  animations[0].init();

  #ifdef ANIMATION_DEBUG
    Serial.println("Initialized animation controller");
  #endif
}

/**
 * Frees up all resources used by this controller
 */
void destroyController_Animation() {    
  for (int i = 0; i < animationCount; i++) {
    animations[i].destroy();
  }  
  free(animations);
  animations = NULL;  
  animationCount = 0;
  animationFrameIdx = 0;
}

enum ControllerType controllerType_Animation();
enum ControllerType controllerType_Animation() {
  return ControllerType_Animation;
}

void assignController_Animation(struct Controller* controller) {
  controller->runLoop = &runLoopController_Animation;
  controller->init = &initController_Animation;
  controller->destroy = &destroyController_Animation;
  controller->type = &controllerType_Animation;
}

void setAnimationType(enum AnimType newType);
/**
 * Set the animation type
 */
void setAnimationType(enum AnimType newType) {
  if (animationCount == 0) {
    #ifdef ANIMATION_DEBUG
      Serial.println("Anim controller not initialized.");
    #endif
    return;
  }

  // If we switched types, destroy the old and set up the new anim
  if (animations[0].type() != newType) {

    #ifdef ANIMATION_DEBUG
      Serial.print("Anim 0 change from ");
      Serial.print(animations[0].type());
      Serial.print(" to ");
      Serial.println(newType);
    #endif

    // Destroy the old animation
    animations[0].destroy();

    // Initialize the new animation
    assignAnimationType(newType, &animations[0]);
    animations[0].init();
  }
}

/**
 * Process the animation parameters,
 * these can be different per each animation,
 * so let each on deal with them.
 */
void processAnimParams(byte params[]) {
  if (animationCount == 0) {
    #ifdef ANIMATION_DEBUG
      Serial.println("Anim controller not initialized.");
    #endif
    return;
  }

  #ifdef ANIMATION_DEBUG
    debugPrintBleParams(params, 4); 
  #endif

  setAnimationType(params[1]);
  animations[0].params(params);
}

/**
 * Should be called by the main loop() function
 * Calls draw on all the animations in the stack
 * And then writes the frame to FastLED
 */
void runLoopController_Animation() {
  animationFrameIdx++;

  for (int i = 0; i < animationCount; i++) {
    animations[i].draw(animationFrameIdx);
  }

  FastLED.show();
}
