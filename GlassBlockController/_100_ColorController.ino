
#include <FastLED.h>

#define COLOR_CONTROLLER_DEBUG = 1

/**
 * Current color RGB and Alpha
 */
byte alpha = 255;
byte red = 40;
byte green = 40;
byte blue = 40;

// Holds the state of the signal to write the frame again
boolean needsWritten = true;

void initController_Color() {
  needsWritten = true;
}

void destroyController_Color() {
  // Taking up negligable memory, no-op needed
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
void processArgbCommand(byte a, byte r, byte g, byte b) {   

  // Make sure the brightness is relative to the max in this code
  alpha = a;  
  setGlobalBrightness(alpha);     

  // Sending black will just control the global brightness 
  if (0 == r && 0 == g && 0 == b) {  

    #ifdef COLOR_CONTROLLER_DEBUG
      Serial.print("New global alpha ");
      Serial.println(alpha);           
    #endif 
    
    return;      
  }

  #ifdef COLOR_CONTROLLER_DEBUG
    Serial.print(alpha);
    Serial.print(", ");
    Serial.print(r);
    Serial.print(", ");
    Serial.print(g);
    Serial.print(", ");
    Serial.print(b);
    Serial.print(" - new a, r, g, b cmd");
    Serial.println(alpha);             
  #endif  

  red = r;
  green = g;
  blue = b;  

   // New command, need to signal to write the frame again
  needsWritten = true;
}
