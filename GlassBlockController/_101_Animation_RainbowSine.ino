/*
  LED animation pattern functions and variables   
*/

#include <FastLED.h>

#define DEBUG_RAINBOW_SINE 1;

/**
 * Type of rainbow sine animation pattern
 */
enum RainbowSinePattern {
  RS_Pattern_Block      = 0,
  RS_Pattern_Block_Gap1 = 1,
  RS_Pattern_Block_Gap2 = 2,
  RS_Pattern_Block_Gap3 = 3,
  RS_Pattern_Led        = 4
};

struct BeatSequence* beats_RainbowSine = NULL;

struct RainbowSineParams {
  // Current Rainbow Row pattern
  enum RainbowSinePattern pattern;
  // Current Rainbow Row beat style
  enum BeatStyle style;
  
  // To make the rainbow look best, we make
  // each LED 7 diff from the last one
  uint8_t colorsInRainbow;

  byte hue;
  byte hueStep;
  boolean reverseDirection = false;

  // The step width when drawing the rainbow
  double rainbowWidth;
  double rainbowStepWidth;
  double targetRainbowStepWidth;
};
struct RainbowSineParams* params_RSStruct = NULL;

/**
 * Initialize memory required to run this animation
 */
void init_RainbowSine() {  
  params_RSStruct = malloc(sizeof(struct RainbowSineParams));
  initParams_RainbowSine();
  
  beats_RainbowSine = malloc(sizeof(struct BeatSequence));   
  beats_RainbowSine->sequenceSize = 0;
  beats_RainbowSine->sequence = NULL;
}

void initParams_RainbowSine() {
  // Current Rainbow Row pattern
  params_RSStruct->pattern = RS_Pattern_Block;
  // Current Rainbow Row beat style
  params_RSStruct->style = Beat_Style_Random;

  // To make the rainbow look best, we make
  // each LED 7 diff from the last one
  params_RSStruct->colorsInRainbow = 7;

  // Rainbow width is step in iterator loop
  params_RSStruct->rainbowWidth = 7.0;
  params_RSStruct->rainbowStepWidth = 2.0;
  params_RSStruct->targetRainbowStepWidth = 0.0;

  params_RSStruct->hue = 0;
  params_RSStruct->hueStep = 0;
  params_RSStruct->reverseDirection = false;
}

/**
 * Free up memory used by this animation
 */
void free_RainbowSine() {
  free(beats_RainbowSine->sequence);
  free(beats_RainbowSine);
  beats_RainbowSine = NULL;

  free(params_RSStruct);
  params_RSStruct = NULL;
}

/**
 * @return the rainbow animation type
 */
enum AnimType type_RainbowSine();
enum AnimType type_RainbowSine() {
  return AnimType_RainbowSine;
}

void setupRainbowSinePattern(enum RainbowSinePattern newPattern);
void setupRainbowSinePattern(enum RainbowSinePattern newPattern) {
  if (params_RSStruct == NULL) {
    #ifdef DEBUG_RAINBOW_SINE
      Serial.println("Params not yet intialized");
    #endif
    return;
  }
  params_RSStruct->pattern = newPattern;
}

/**
 * Sets the default parameters for this animation
 */
void params_RainbowSine(byte params[]) {
  if (params_RSStruct == NULL) {
    #ifdef DEBUG_RAINBOW_SINE
      Serial.println("params_RSStruct not yet intialized");
    #endif
    return;
  }

  // Set the new or existing pattern
  setupRainbowSinePattern(params[2]);

  // Assign the new speed value to all rows
  double newCoeff = ((double)params[3] * 0.25) - 6.0;
  
  // Keep moving left or right parity
  params_RSStruct->reverseDirection = (newCoeff < 0);
  params_RSStruct->hueStep = (byte)abs(round(newCoeff));
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
 * The Rainbow Sine pattern moves hue by up the rows, then down the next row, etc.
 * Draw the next rainbow frame
 */
void draw_RainbowSine(uint16_t beatNumInMeasure) { 

  // Draw the background
  if (params_RSStruct->pattern == RS_Pattern_Block_Gap1 ||
      params_RSStruct->pattern == RS_Pattern_Block_Gap2 ||
      params_RSStruct->pattern == RS_Pattern_Block_Gap3) {

    FastLED_FillSolid(40, 40, 40); 
  }

  // Incramenet animation
  if (params_RSStruct->reverseDirection) {
    params_RSStruct->hue += params_RSStruct->hueStep;
  } else {
    params_RSStruct->hue -= params_RSStruct->hueStep;
  }

  double hueStep = params_RSStruct->rainbowWidth;

  if (isBeatControllerRunning()) {
    checkBeat_rainbowSineBeat(beatNumInMeasure);
  }

  drawFrameCols_rainbowSineAll(beatNumInMeasure, params_RSStruct->hue, hueStep);
}

/**
 * Draws the beat rainbow sine and changes speed/direction on beat
 */
void checkBeat_rainbowSineBeat(uint16_t beatNumInMeasure) {
  if (isABeat(beatNumInMeasure, beats_RainbowSine) || params_RSStruct->hueStep == 0) {
      params_RSStruct->reverseDirection = !params_RSStruct->reverseDirection;
      if (params_RSStruct->reverseDirection) {
        params_RSStruct->hueStep = stepForBopmByte();
      } else {
        params_RSStruct->hueStep = 2 * stepForBopmByte();
      }
    }    
}

/**
 * Draw all columns in all rows for basic step
 */
void drawFrameCols_rainbowSineAll(uint16_t beatNumInMeasure, double hue, double hueStep) {  

  // Starting step values
  int sidewaysStep = getColumnStep_RainbowSine();  
  uint8_t movementDirection = 0; // 0 is down, 1 is sideways, 2 is up, 3 is sideways
  int col = 0;
  int row = 0;

  // End on either column led or block count
  int endingCol = ledsPerRow;
  if (isBlockPattern_RainbowSine()) {
    endingCol = blocksPerRow;
  }

  while (col < endingCol) {
    // Draw next LED or Block, depending on pattern
    incramentHueAndDraw(&hue, hueStep, row, col);
    
    if (movementDirection == 2) {  // UP      
      row--;
      if (row == 0) {
        movementDirection = (movementDirection + 1) % 4;
      }      
    } else if (movementDirection == 1 || movementDirection == 3) {  // SIDEWAYS
      col++;
      sidewaysStep--;
      if (sidewaysStep == 0) {
        sidewaysStep = getColumnStep_RainbowSine();
        movementDirection = (movementDirection + 1) % 4;
      }
    } else if (movementDirection == 0) {  // DOWN
      row++;
      if (row == (ledsRowCount - 1)) {
        movementDirection = (movementDirection + 1) % 4;
      }   
    }
  }
}

/**
 * Incrament the hue by hue step and draw the row, col value
 */
void incramentHueAndDraw(double *hue, double hueStep, int row, int col) {
  *hue = *hue + hueStep;
  if (*hue > 255.0) {
    *hue -= 255.0;
  }
  if (*hue < 0.0) {
    *hue += 255.0;
  }
  
  if (isBlockPattern_RainbowSine()) {
    FastLED_lightBlockHue(row, col, (int)*hue);
  } else {
    FastLED_SetHue(to_led_idx(row, col), (int)*hue);
  }
}

uint8_t getColumnStep_RainbowSine() {
  switch(params_RSStruct->pattern) {
    case RS_Pattern_Block:
      return 1;
    case RS_Pattern_Block_Gap1:
      return 2;
    case RS_Pattern_Block_Gap2:
      return 3;
    case RS_Pattern_Block_Gap3:
      return 4;
    case RS_Pattern_Led:
      return 1;
  }
  return 1;
}

boolean isBlockPattern_RainbowSine() {
  switch(params_RSStruct->pattern) {
    case RS_Pattern_Block:
    case RS_Pattern_Block_Gap1:
    case RS_Pattern_Block_Gap2:
    case RS_Pattern_Block_Gap3:
      return true;    
  }
  return false;
}
