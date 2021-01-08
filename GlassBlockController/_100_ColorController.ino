
#include <FastLED.h>

// #define COLOR_CONTROLLER_DEBUG = 1

/**
 * Current color RGB and Alpha
 */
byte white40 = 40;
byte red = white40;
byte green = white40;
byte blue = white40;

// Holds the state of the signal to write the frame again
boolean needsWritten = true;

void initController_Color() {
  needsWritten = true;
}

void destroyController_Color() {
  red = white40;
  green = white40;
  blue = white40;
}

void runLoopController_Color() {
  if (needsWritten) {    
    needsWritten = false;
    FastLED_FillSolid(red, green, blue);
    FastLED.show();
  }
}

enum ControllerType controllerType_Color();
enum ControllerType controllerType_Color() {
  return ControllerType_Color;
}

void assignController_Color(struct Controller* controller) {
  controller->runLoop = &runLoopController_Color;
  controller->init = &initController_Color;
  controller->destroy = &destroyController_Color;
  controller->type = &controllerType_Color;
}

/**
 * Process an ARGB command from BLE
 * 
 * @param alpha is used to control global brightness to set an "alpha"
 * @param r red color value
 * @param g green color value
 * @param b blue color value
 * @return true if RGB command was executed, false if global brightness command was executed
 */
void processRgbCommand(byte r, byte g, byte b) {   

  #ifdef COLOR_CONTROLLER_DEBUG
    Serial.print(r);
    Serial.print(", ");
    Serial.print(g);
    Serial.print(", ");
    Serial.println(b);
  #endif  

  red = r;
  green = g;
  blue = b;  

   // New command, need to signal to write the frame again
  needsWritten = true;
}
