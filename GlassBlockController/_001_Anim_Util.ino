/**
 * Type of animation
 */
enum AnimType {
  AnimType_RainbowRow  = 0,
  AnimType_RainbowSine = 1,
  AnimType_Squares     = 2,
  AnimType_Tetris      = 3
};

/**
 * Type of rainbow row animation pattern
 */
enum RainbowRowPattern {
  Pattern_Equal        = 0,
  Pattern_EqualOpp     = 1,
  Pattern_SlantedLeft  = 2,
  Pattern_SlantedRight = 3,
  Pattern_ArrowLeft    = 4,
  Pattern_ArrowRight   = 5,
  Pattern_None         = 255
};

/**
 * Type of Rainbow beat animation style
 */
enum BeatStyle {
  Beat_Style_Random = 0,
  Beat_Style_Metronome = 1
};

enum RainbowRowPattern rainbowRowPatternForIdx(uint8_t idx);
enum RainbowRowPattern rainbowRowPatternForIdx(uint8_t idx) {
  switch(idx) {
    case Pattern_Equal:
      return Pattern_Equal;
    case Pattern_EqualOpp:
      return Pattern_EqualOpp;
    case Pattern_SlantedLeft:
      return Pattern_SlantedLeft;
    case Pattern_SlantedRight:
      return Pattern_SlantedRight;
    case Pattern_ArrowLeft:
      return Pattern_ArrowLeft;
    case Pattern_ArrowRight:
      return Pattern_ArrowRight;
    default:
      return Pattern_None;          
  }
}

/**
 * Animation function pointers to implement
 * inheritence for each animation.
 * This allows the animation controller
 * to manage the animations generically
 */
typedef void (*DrawFunc)(uint16_t);
typedef void (*ParamsFunc)(byte[]);
typedef enum AnimType (*TypeFunc)();
typedef BeatSequence* (*BeatSequenceFunc)();
/**
 * Defined in _000_ControllerManager.ino
 * typedef void (*InitFunc)();
 * typedef void (*DestroyFunc)();
 */

/**
 * Animation State basics for linear hue
 */
struct LinearInterpolation {
  double start = 0.0;
  double coeff = 0.0;
};

/**
 * Animation State basics for linear hue.
 * However, a dynamic version contains
 * the ability to track it's original (og) values.
 * That way you can animate by changing speed
 * and come back to the original.
 */
struct DynamicLinearInterpolation {

  // Stores the og interpolation values
  double ogStart;
  double ogStep;

  // The current start and coef 
  double curVal;
  // Stores the target value to end on
  // Stores the step for iterating to target
  double curStep;

  // Target that the curStep is working towards
  // By adding speedStep
  double targetSpeedStep;
  // Use this to speed up step to make 
  // quadratic animation changes
  double speedStep;
};

void initDynamicLinearInterpolation(struct DynamicLinearInterpolation* interp) {
  interp->ogStart = 0.0;
  interp->ogStep = 2.0;
  
  // The current start and coef 
  interp->curVal = 0.0;
  // Stores the target value to end on
  // Stores the step for iterating to target
  interp->curStep = 2.0;
  
  // Target that the curStep is working towards
  // By adding speedStep
  interp->targetSpeedStep = 0.0;  
  // Use this to speed up step to make 
  // quadratic animation changes
  interp->speedStep = 0.0;
}

/**
 * Animation State basics for quadratic hue
 */
struct QuadraticInterpolation {
  double start = 0.0;
  double coeffA = 0.0;
  double coeffB = 0.0;
};

/**
 * An animation will always have these functions
 * This is how the animation controller uses
 * "inheritence" to handle generically calling animation funcs.
 */
struct Animation {
  InitFunc init = NULL;
  DestroyFunc destroy = NULL;
  TypeFunc type = NULL;
  DrawFunc draw = NULL;  
  ParamsFunc params = NULL;
  BeatSequenceFunc beat = NULL;
};

/**
 * Due to the way Arduino IDE compiles,
 * we need to forward declare the assignAnimation_ Funcs
 * for each animation type
 */
void setAnimFunc_RainbowRow(struct Animation* anim);

void assignAnimationType(enum AnimType type, struct Animation* anim);  
/**
 * Assign the func ptrs in anim to their corresponding type
 */
void assignAnimationType(enum AnimType type, struct Animation* anim) {
  switch (type) {
    case AnimType_RainbowRow:
      setAnimFunc_RainbowRow(anim);
      break;
    case AnimType_RainbowSine:
      setAnimFunc_RainbowSine(anim);
      break;
  }
}

/**
 * Applies a linear step to the animation parameters
 * And returns the result scaled to 255
 * 
 * @param interp linear interpolation values to be used
 * @param t is time this can be scaled however 
 * 
 * @return value between 0 and 255
 */
byte linearInterp255(struct LinearInterpolation interp, double t) {
  return interpTo255(linearInterp(interp, t));
}
double dynamicLinearInterp255(struct DynamicLinearInterpolation interp, double t) {
  return interpTo255(dynamicLinearInterp(interp, t));
}

/**
 * Applies a linear step to the animation parameters
 *
 * @param interp linear interpolation values to be used
 * @param t is time this can be scaled however 
 * 
 * @return linear interpolated value
 */
double linearInterp(struct LinearInterpolation interp, double t) {
  return interp.start + (interp.coeff * t);
}
double dynamicLinearInterp(struct DynamicLinearInterpolation interp, double t) {
  return interp.ogStart + (interp.ogStep * t);
}

/**
 * Calculate the difference between the target and the og 
 */
void dynamicLinearInterpStep(struct DynamicLinearInterpolation* interp) {
  
  // Check for ending bounds in changing speed
  if ((interp->speedStep > 0 && interp->curStep >= interp->targetSpeedStep) ||
      (interp->curStep < 0 && interp->curStep <= interp->targetSpeedStep)) {

    // Reset speed change vars
    interp->speedStep = 0.0;
    interp->curStep = interp->targetSpeedStep;
    interp->targetSpeedStep = 0.0;
  }

  // Iterate speed that curVal is going to target
  interp->curStep += interp->speedStep;

  // Step through the 
  interp->curVal += interp->curStep;
}

double calcDynamicLinearStep(struct DynamicLinearInterpolation* interp, double targetSpeed, double framesToDoItIn) {
  double diff = targetSpeed - interp->curStep;  
  interp->speedStep = diff / framesToDoItIn;
  interp->targetSpeedStep = targetSpeed;
}

double calcDynamicLinearExponentialStep(struct DynamicLinearInterpolation* interp, double target, double framesUntilBeat, double speedStep) {
  calcDynamicLinearStep(interp, target, framesUntilBeat);
  // TODO: mdephillips 1/7/2021 do we need to re-evaluate step to land on beat
  interp->speedStep = speedStep;
}

/**
 * @param interpValue value computed by an interpolation
 * @return interpolation value rounded and modulated to fit a value between 0-255
 */
byte interpTo255(double interpValue) {  
  if (interpValue < 0) {
    // Negative values treat as 255 -> 0
    return 255 - (((int)round(abs(interpValue))) % 255);
  }
  // Postive values treat as 0 -> 255
  return ((int)round(interpValue)) % 255;
}

/**
 * Applies a quadratic step to the animation parameters
 * And returns the result scaled to 255
 * 
 * @param interp quadratic interpolation values to be used
 * @param t is time this can be scaled however 
 * 
 * @return value between 0 and 255
 */
byte quadInterp255(struct QuadraticInterpolation interp, double t) {
  return abs(round((interp.coeffA * (t * t)) + (interp.coeffB * t) + interp.start)) % 255;
}
