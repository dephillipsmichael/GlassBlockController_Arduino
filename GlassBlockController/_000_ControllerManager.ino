/**
 * This class controls the state of the GlassBlock LED
 * and manages when states should destroy resources 
 * that are no longer needed by the controller
 */

// #define STATE_CONTROLLER_DEBUG = 1

/**
 * The type of controller to write to the LED wall
 */
enum ControllerType {
  ControllerType_Color     = 0,
  ControllerType_Animation = 1,
  ControllerType_Beat      = 2
};

/**
 * Used by the controller manager to maintain 
 * state of each controller
 */
typedef void (*InitFunc)();
typedef void (*DestroyFunc)();
typedef void (*RunLoopFunc)();
typedef enum ControllerType (*ControllerTypeFunc)();

struct Controller {
  ControllerTypeFunc type;
  InitFunc init;
  DestroyFunc destroy;
  RunLoopFunc runLoop;
};
struct Controller controller;

/**
 * Initialize the controller manager
 * Assigns default controller
 */
void setupControllerManager() {  
  // Default to color controller
  assignController_Color(&controller);
}

/**
 * Runs the selected controller loop
 */
void loopControllerManager() {
  controller.runLoop();  
}

void setControllerType(enum ControllerType type);
/**
 * @param the newState of the app
 */
void setControllerType(enum ControllerType type) {

  if (controller.type() == type) {
    #ifdef STATE_CONTROLLER_DEBUG
      Serial.print(F("Controller type already set to "));  
      Serial.print(type);  
      Serial.println();
    #endif  
    return;  // We are already using this controller
  } 

  #ifdef STATE_CONTROLLER_DEBUG
    Serial.print(F("Switching controller types from "));
    Serial.print(controller.type());
    Serial.print(" to ");
    Serial.print(type);
    Serial.println();
  #endif 

  controller.destroy();

  if (ControllerType_Color == type) {
    assignController_Color(&controller);
  } else if (ControllerType_Animation == type) {
    assignController_Animation(&controller);
  } else { // ControllerType.Beat:
    assignController_Beat(&controller);
  }
  
  controller.init();
}

/**
 * Prints the output of the BLE command params
 */
void debugPrintBleParams(byte msg[], int msgSize) {
  for (int i = 0; i < msgSize; i++) {
    Serial.print(msg[i]);
    if (i < (msgSize - 1)) {
      Serial.print(", ");  
    }
  }
  Serial.println();
}
