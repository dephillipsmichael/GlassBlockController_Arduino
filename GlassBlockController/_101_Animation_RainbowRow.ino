/*
  LED animation Pattern functions and variables
*/

#include <FastLED.h>

#define ANIMATION_RAINBOW_ROW_DEBUG = 1

struct DynamicLinearInterpolation* interp_RainbowRow = NULL;
struct BeatSequence* beats_RainbowRow = NULL;

struct RainbowRowParams {
  // Current Rainbow Row pattern
  enum RainbowRowPattern pattern = Pattern_None;
  // Current Rainbow Row beat style
  enum BeatStyle style = Beat_Style_Random;

  // To make the rainbow look best, we make
  // each LED 7 diff from the last one
  uint8_t colorsInRainbow = 7;
};

struct RainbowRowParams* params_RRStruct = NULL;

/**
 * Forward declare enum param functions
 */
void setupRainbowRowPattern(enum RainbowRowPattern newPattern);

/**
 * Initialize memory required to run this animation
 */
void init_RainbowRow() {
  interp_RainbowRow = malloc(sizeof(struct DynamicLinearInterpolation) * ledsRowCount);
  for (int row = 0; row < ledsRowCount; row++) {
    initDynamicLinearInterpolation(&interp_RainbowRow[row]);
    interp_RainbowRow[row].ogStep = 12.0;
    interp_RainbowRow[row].curStep = 12.0;
  }

  params_RRStruct = malloc(sizeof(struct RainbowRowParams));
  initParams_RainbowRow();

  // Start with a random pattern
  setupRainbowRowPattern(Pattern_Equal);

  beats_RainbowRow = malloc(sizeof(struct BeatSequence));
  beats_RainbowRow->sequenceSize = 0;
  beats_RainbowRow->sequence = NULL;
}

void initParams_RainbowRow() {
    // Current Rainbow Row pattern
  params_RRStruct->pattern = Pattern_None;
  // Current Rainbow Row beat style
  params_RRStruct->style = Beat_Style_Random;
  //params_RRStruct->style = Beat_Style_Metronome;

  // To make the rainbow look best, we make
  // each LED 7 diff from the last one
  params_RRStruct->colorsInRainbow = 7;
}

/**
 * Sets the default parameters for this animation
 */
void params_RainbowRow(byte params[]) {
  if (interp_RainbowRow == NULL) {
    #ifdef ANIMATION_RAINBOW_ROW_DEBUG
      Serial.println(F("Rainbow Row Anim not initialized"));
    #endif
    return;
  }

  // Set the new or existing pattern
  setupRainbowRowPattern(params[2]);

  // Assign the new speed value to all rows
  double newCoeff = (params[3] * 0.5) - 12.0;
  for (int i = 0; i < ledsRowCount; i++) {
    // Keep moving left or right parity
    interp_RainbowRow[i].ogStep = newCoeff;  
    if (params[2] == Pattern_EqualOpp &&
        i % 2 == 0) {
      Serial.print(F(" Opp "));
      interp_RainbowRow[i].ogStep = -newCoeff;      
    }
  }

  #ifdef ANIMATION_RAINBOW_ROW_DEBUG
    Serial.print(F("New speed "));
    Serial.print(newCoeff);
    Serial.print(F(" New pattern "));
    Serial.print(params[2]);
    Serial.println();
  #endif
}

/**
 * Set the intial spacing of the pattern
 * to create the visual effect in the name of the pattern
 */
void setupRainbowRowPattern(enum RainbowRowPattern newPattern) {

  if (newPattern == params_RRStruct->pattern) {
    return;  // No change necessary
  }
  params_RRStruct->pattern = newPattern;

  // block step is the number of leds to skip
  byte blockStep = 255 / params_RRStruct->colorsInRainbow;
  int midRow = ledsRowCount / 2;
  byte twoBlocks = 2 * maxLedsPerBlock;

  for (int i = 0; i < ledsRowCount; i++) {

     // Set delta hue the same on all of them
    interp_RainbowRow[i].ogStep = 1.0;

    // Equal spacing rainbow across each row
    if (params_RRStruct->pattern == Pattern_Equal) {
      interp_RainbowRow[i].ogStart = (blockStep * i);
    }
    else if (params_RRStruct->pattern == Pattern_EqualOpp) {
      // Here we want the animation to perform like FastLED_Rainbow
      interp_RainbowRow[i].ogStart = twoBlocks * i;
      if (i % 2 != 0) {  // reverse direction of each row
        interp_RainbowRow[i].ogStep = -interp_RainbowRow[i].ogStep;
      }
    }
    // "/" looking design
    else if (params_RRStruct->pattern == Pattern_SlantedLeft) {
      interp_RainbowRow[i].ogStart = twoBlocks * (ledsRowCount - i - 1);
    }
    // "\" looking design
    else if (params_RRStruct->pattern == Pattern_SlantedRight) {
      interp_RainbowRow[i].ogStart = twoBlocks * i;
    }
    // "<" looking design
    else if (params_RRStruct->pattern == Pattern_ArrowLeft) {
      interp_RainbowRow[i].ogStart = blockStep * ((midRow) - abs(midRow - i));
    }
    // ">" looking design
    else { // if (== Pattern_ArrowRight)
       interp_RainbowRow[i].ogStart = blockStep * abs(midRow - i);
    }

    #ifdef ANIMATION_RAINBOW_ROW_DEBUG
      Serial.print(interp_RainbowRow[i].ogStart);
      Serial.print(", ");
    #endif

    interp_RainbowRow[i].curVal = interp_RainbowRow[i].ogStart;
    interp_RainbowRow[i].curStep = interp_RainbowRow[i].ogStep;
  }
  #ifdef ANIMATION_RAINBOW_ROW_DEBUG
    Serial.println();
    for (int i = 0; i < ledsRowCount; i++) {
      Serial.print(interp_RainbowRow[i].ogStep);
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

  free(params_RRStruct);
  params_RRStruct = NULL;
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

  // WARNING: 
  //
  // The drawBeats_RainbowRow code takes about
  // 50 milliseconds to perform.
  // That means that beat detection,
  // These won't be very accurate, and BLE messages will be,
  // delayed significantly when sending a BPM info command
  if (isBeatControllerRunning()) {
    drawBeats_RainbowRow(beatNumInMeasure);
    return;
  }

  drawFrameCols_rainbowRowAll(beatNumInMeasure);
}

/**
 * Draw all columns in all rows for basic step
 */
void drawFrameCols_rainbowRowAll(uint16_t beatNumInMeasure) {
  // Draw rainbow rows
  for (int row = 0; row < ledsRowCount; row++) {
    byte hue = dynamicLinearInterp255(interp_RainbowRow[row], beatNumInMeasure);
    byte hueStep = abs(interp_RainbowRow[row].ogStep);
    drawFrameCols_rainbowRow(row, (double)hue, (double)hueStep);
  }
}

/**
 * Calculate next frame and draw to LEDs synced to BPM
 */
void drawBeats_RainbowRow(uint16_t beatNumInMeasure) {

  // On beats, find a new target animation val
  if (isABeat(beatNumInMeasure, beats_RainbowRow)) {

    #ifdef ANIMATION_RAINBOW_ROW_DEBUG
      Serial.print("RainbowRow beat");
    #endif

    double newSpeedTarget = stepForBpm() * ((double)random(50, 200) / 100.0);
      if (params_RRStruct->style == Beat_Style_Random) {
        #ifdef ANIMATION_RAINBOW_ROW_DEBUG
        Serial.print("RainbowRow new rand speed = ");
        Serial.println(newSpeedTarget);
      #endif

      for (int row = 0; row < ledsRowCount; row++) {
        calcDynamicLinearStep(&interp_RainbowRow[row], newSpeedTarget, 6.0);
      }
    } else if (params_RRStruct->style == Beat_Style_Metronome) {
      for (int row = 0; row < ledsRowCount; row++) {
        interp_RainbowRow[row].curStep = -interp_RainbowRow[row].curStep;
      }
    }

    #ifdef ANIMATION_RAINBOW_ROW_DEBUG
      Serial.println();
    #endif
  }

  for (int row = 0; row < ledsRowCount; row++) {
    // Go to next frame
    dynamicLinearInterpStep(&interp_RainbowRow[row]);

    double hue = interp_RainbowRow[row].curVal;
    drawFrameCols_rainbowRow(row, hue, interp_RainbowRow[row].curStep);
  }
}

/**
 * Draw all columns in a row stepHue per LED
 */
void drawFrameCols_rainbowRow(int row, double hue, double stepHue) {
  for (int col = 0; col < ledsPerRow; col++) {
    FastLED_SetHue(to_led_idx(row, col), interpTo255(hue + (col * stepHue)));
  }
}
