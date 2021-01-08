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
      // TODO: implement the rest
      // setAnimFunc_RainbowRow(anim);
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
