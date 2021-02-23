/*
 * LED Animation to check if BPM and beat counting is correct 
 */
 
#include <FastLED.h>

// #define TETRIS_DEBUG 1
// #define TETRIS_DEBUG_DRAWING 1

struct TetrisVars {
  byte lightWhiteRgb;
  boolean useHueOffsetCtr;
  byte hueOffsetCtr;
  byte hueOffsetItr;
  uint8_t tetrisIdx;
  uint8_t unAssignedDropIdx;
  uint8_t rowDropIdx;
  uint8_t endingStateFlashCtr;
  uint8_t blockDataSize; // = 11;
  uint8_t tetrisSize; // = 18;
  uint8_t blocks[18][11];
  uint8_t animSpeed;
  uint16_t animFrameIdx;
};

// Speed of the tetris animation
struct LinearInterpolation* interp_Tetris = NULL;

// Store as pointer so as not to take up memory until used
struct TetrisVars* tetris = NULL;

// Keeps track of timing of animation triggers on assigned beats
struct BeatSequence* beats_Tetris = NULL;
BeatSequence* getBeats_Tetris() {
  return beats_Tetris;
}

/**
 * Initialize memory required to run this animation
 */
void init_Tetris() {
  #ifdef TETRIS_DEBUG
    Serial.println("tetris init");
  #endif
  
  tetris = malloc(sizeof(struct TetrisVars));
  beats_Tetris = malloc(sizeof(struct BeatSequence));

  tetris->animFrameIdx = 0;
  tetris->animSpeed = 48;  

  // Hue values of specified colors
  tetris->lightWhiteRgb = 20;

  // The hue offsetCtr is a rotating offset
  // that can be used to cycle the tetris pieces colors
  // the hueOffsetItr is how quickly it changes
  tetris->useHueOffsetCtr = false;
  tetris->hueOffsetCtr = 0;
  tetris->hueOffsetItr = 8;   

  // Used to track which piece we are on
  tetris->tetrisIdx = 0;
  // Used to track which row we are at in the drop animation
  tetris->unAssignedDropIdx = 255;
  tetris->rowDropIdx = tetris->unAssignedDropIdx;
  
  // State of the ending animation
  tetris->endingStateFlashCtr = 0;

  tetris->blockDataSize = 11;
  tetris->tetrisSize = 18;
  // See google spreadsheet for visual of this data:
  // https://docs.google.com/spreadsheets/d/1-_Ugj6EqNiLKDq4XKn1Xmqe9Q-mdsAmwi_rJ1mabieA/edit#gid=807635637
  uint8_t tempBlocks[18][11] = {
    // Format is:
    // 
    // { hueColor, colDrop, rowDrop, blockRowSize, blockColSize, 
    // [Next N = blockRowSize * blockColSize]...] are
    // [1 if block is filled in], [0 if block is blank] }
    // 
    // colDrop = column index where the block will drop
    // rowDrop = rows to iterate over in the drop
    {    HUE_BLUE,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, // black frame 
    {  HUE_PURPLE,  6, 4, 2, 3, 0, 1, 0, 1, 1, 1 }, // T-block upside down
    {  HUE_ORANGE,  0, 4, 3, 2, 1, 0, 1, 0, 1, 1 }, // L-block normal
    {    HUE_BLUE, 10, 4, 3, 2, 0, 1, 0, 1, 1, 1 }, // L-block hori flipped
    {   HUE_GREEN,  4, 4, 2, 3, 0, 1, 1, 1, 1, 0 }, // S-block normal
    {   HUE_GREEN,  8, 4, 3, 2, 1, 0, 1, 1, 0, 1 }, // S-block rotated -90
    {     HUE_RED,  9, 3, 3, 2, 1, 0, 1, 1, 0, 1 }, // S-block rotated -90
    {   HUE_GREEN,  1, 4, 3, 2, 1, 0, 1, 1, 0, 1 }, // S-block rotated -90
    {  HUE_PURPLE,  3, 4, 3, 2, 1, 0, 1, 1, 1, 0 }, // T-block rotated -90
    {  HUE_YELLOW, 10, 1, 2, 2, 1, 1, 1, 1, 0, 0 }, // O-block
    {    HUE_AQUA,  4, 2, 1, 4, 1, 1, 1, 1, 0, 0 }, // I-block
    {  HUE_PURPLE,  1, 2, 3, 2, 0, 1, 1, 1, 0, 1 }, // T-block rotated 90
    {    HUE_BLUE,  3, 1, 2, 3, 1, 0, 0, 1, 1, 1 }, // L-block rotated -90 and hori flipped
    {     HUE_RED,  5, 1, 2, 3, 1, 1, 0, 0, 1, 1 }, // L-block hori flipped  
    {  HUE_PURPLE,  7, 1, 2, 3, 1, 1, 1, 0, 1, 0 }, // T-block
    {  HUE_YELLOW,  0, 1, 2, 2, 1, 1, 1, 0, 0, 0 }, // Filler-block, shaped like "r"
    {    HUE_AQUA,  4, 0, 1, 1, 1, 0, 0, 0, 0, 0 }, // Filler-block single block
    {    HUE_AQUA,  0, 0, 0, 1, 1, 0, 0, 0, 0, 0 }  // Flash the all row
  }; 
  for (int row = 0; row < tetris->tetrisSize; row++) {
    for (int col = 0; col < tetris->blockDataSize; col++) {
      tetris->blocks[row][col] = tempBlocks[row][col];
    }
  }
}

/**
 * Sets the default parameters for this animation
 */
void params_Tetris(byte params[]) {
  if (tetris == NULL) {
    #ifdef TETRIS_DEBUG
      Serial.println(F("Tetris Anim not initialized"));
    #endif
    return;
  }

  // Set whether each tetris piece should change color
  tetris->useHueOffsetCtr = (params[2] >= 1);
  
  // Assign the new speed of how quickly we drop new frames
  tetris->animSpeed = (((49 - params[3]) * 2) + 16) / 4;   
}

/**
 * Free up memory used by this animation
 */
void free_Tetris() {
  #ifdef TETRIS_DEBUG
    Serial.print("free tetris");
  #endif
  
  free(tetris);
  tetris = NULL;
  
  free(beats_Tetris->sequence);
  free(beats_Tetris);
  beats_Tetris = NULL;
}

/**
 * @return the rainbow animation type
 */
enum AnimType type_Tetris();
enum AnimType type_Tetris() {
  return AnimType_Tetris;
}

/**
 * Sets the anim struct with the correct function pointers
 */
void setAnimFunc_Tetris(struct Animation* anim) {
  anim->type = &type_Tetris;
  anim->draw = &draw_Tetris;
  anim->params = &params_Tetris;
  anim->init = &init_Tetris;
  anim->destroy = &free_Tetris;
  anim->beat = &getBeats_Tetris;
}

/**
 * Draw the current tetris animation at the specified beat 
 */
void draw_Tetris(uint16_t beatNumInMeasure) {

  if (tetris == NULL) {
    return;
  }

  // Check for next piece drop
  boolean startNextPieceDrop = false;
  
  if (isBeatControllerRunning()) {
    // Because we want to time the beats to the 
    // tetris piece hitting the bottom row.
    // We look into the future at the next piece to drop
    // and check how many rows it will have to drop
    uint8_t nextTetrisPieceRows = calcNextTetrisPieceRows();
    uint8_t nextBeatsInAdv = calcNextBeatsInAdv(beatNumInMeasure, nextTetrisPieceRows);
    startNextPieceDrop = (tetris->rowDropIdx == tetris->unAssignedDropIdx) && 
      isABeat(nextBeatsInAdv, beats_Tetris);
  } else {  // Use basic animation timing, every 40 frames  

    // Scale frame-rate with speed
    if (beatNumInMeasure % (tetris->animSpeed / 2) != 0) {      
      return;
    }
    
    tetris->animFrameIdx++;

    // Use speed to determine when to drop the next piece
    if ((tetris->animFrameIdx % tetris->animSpeed) == 0) {
      startNextPieceDrop = true;
    }
  }

  // Refresh screen with off white
  FastLED_FillSolid(tetris->lightWhiteRgb, tetris->lightWhiteRgb, tetris->lightWhiteRgb);
  
  // Check if we should drop the next piece
  if (startNextPieceDrop) {

    #ifdef TETRIS_DEBUG
      Serial.print(" Nxt Tetris.");
    #endif       

    // Go to next tetris piece
    tetris->tetrisIdx++;

    // Check for ending state
    if ((tetris->tetrisIdx) >= (tetris->tetrisSize)) {
      #ifdef TETRIS_DEBUG
        Serial.print(F(" Tetris strt end anim."));
      #endif          
      // Start the animation
      tetris->endingStateFlashCtr = 1; 
      tetris->rowDropIdx = 0;         
    } else {     
      // Do next tetris beat
      tetris->rowDropIdx = tetris->blocks[tetris->tetrisIdx][2];
    }
  } 

  // We are in the ending flash animation
  if (tetris->endingStateFlashCtr > 0) { 
    drawTetrisBlink();
    tetris->endingStateFlashCtr++;

    // Process ending state
    if (tetris->endingStateFlashCtr > 12) {
      #ifdef TETRIS_DEBUG
        Serial.print("End flash animation");
      #endif 
      tetris->endingStateFlashCtr = 0;
      tetris->rowDropIdx = tetris->unAssignedDropIdx;
      tetris->tetrisIdx = 0; 
    }  
  } // We are animating a piece drop
  else if (tetris->rowDropIdx != tetris->unAssignedDropIdx) {
    // Draw all the current pieces
    drawAllTetrisPiecesUntil(tetris->tetrisIdx);
    // Draw the animating piece
    #ifdef TETRIS_DEBUG
      Serial.print(" Draw drop ");
      Serial.print(tetris->rowDropIdx);
      Serial.print(".");
    #endif 
    drawTetrisPiece(tetris->blocks[tetris->tetrisIdx], tetris->rowDropIdx);
    tetris->rowDropIdx--;
    if (tetris->rowDropIdx == 0) {
      tetris->rowDropIdx = tetris->unAssignedDropIdx;
    }
  } else {
    // Draw the current tetris board
    drawAllTetrisPiecesUntil((tetris->tetrisIdx) + 1);
  }

  if (tetris->useHueOffsetCtr) {
    tetris->hueOffsetCtr += tetris->hueOffsetItr;
  }

  #ifdef TETRIS_DEBUG
      Serial.println();
  #endif    
}

uint8_t calcNextTetrisPieceRows() {
  return (tetris->blocks[(tetris->tetrisIdx + 1) % tetris->tetrisSize][2] * 2); // always make it even
}

uint8_t calcNextBeatsInAdv(uint16_t beatNumInMeasure, uint8_t nextTetrisPieceRows) {
  return (beatNumInMeasure + nextTetrisPieceRows) % beatFramesInMeasure(); 
}

/**
 * Blink the whole tetris board
 */
void drawTetrisBlink() {
  if ((tetris->endingStateFlashCtr % 6) % 2 == 0) {
      #ifdef TETRIS_DEBUG
        Serial.print(" Ending State ON.");
      #endif  
      // Draw the tetris board filled
      drawAllTetrisPieces();
    } else {
      #ifdef TETRIS_DEBUG
        Serial.print(" Ending State OFF.");
      #endif  
      // Make all pieces loook lik the background
      FastLED_FillSolid(tetris->lightWhiteRgb, tetris->lightWhiteRgb, tetris->lightWhiteRgb);
    }    
}

/**
 * Draw a tetris piece at the specified starting row and index
 * @param tetrisPiece holding all the info about the piece
 * @param atRowIdex, the starting row in the drop animation to drop the piece
 * 
 * Example 1: First piece idx 0, animRowIdx = 4
 * { purple,  6, 4, 2, 3, 1, 0, 1, 1, 1, 0 }, // T-block upside down
 * rowsToBottomOfBlock = 4 - 4 = 0
 * pieceHeight = 2
 * row = 0 - 2 = -2;  until including 0;
 * col = 4 until 4 + 3
 */
void drawTetrisPiece(uint8_t tetrisPiece[], uint8_t animRowIdx) {  
  int i = 0;
  int rowBottomOfBlock = (int)tetrisPiece[2] - (int)animRowIdx + 1;
  int pieceHeight = tetrisPiece[3];
  int pieceWidth = tetrisPiece[4];
  for (int row = rowBottomOfBlock - pieceHeight; row < rowBottomOfBlock; row++) {
    for (int col = tetrisPiece[1]; col < (tetrisPiece[1] + pieceWidth); col++) {
      if (row >= 0 && col >= 0 && 
          tetrisPiece[i + 5] == 1) {
         #ifdef TETRIS_DEBUG_DRAWING
          Serial.print(" (");
          Serial.print(row);
          Serial.print(",");
          Serial.print(col);
          Serial.print(")");
        #endif 
        FastLED_lightBlockHue(row, col, hueTranslated_Tetris(tetrisPiece[0]));        
      }
      i++;
    }
  }
}

/*
 * Draw all the tetrisPieces
 */
void drawAllTetrisPieces() {
  drawAllTetrisPiecesUntil(tetris->tetrisSize);
}

/*
 * Draw all the tetrisPieces
 * @param endingIdx until this ending index
 */
void drawAllTetrisPiecesUntil(uint8_t endingIdx) {
  #ifdef TETRIS_DEBUG
    Serial.print(" Draw all until ");
    Serial.print(endingIdx);
    Serial.print(".");
  #endif 
  for (int i = 0; i < endingIdx; i++) {
    drawTetrisPiece(tetris->blocks[i], 0);
  }
}

byte hueTranslated_Tetris(byte rawHue) {
  if (!tetris->useHueOffsetCtr) {
    return rawHue;
  }
  return rawHue + tetris->hueOffsetCtr;
}
