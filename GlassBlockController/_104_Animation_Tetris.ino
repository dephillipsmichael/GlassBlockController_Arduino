/*
 * LED Animation to check if BPM and beat counting is correct 
 */
 
#include <FastLED.h>

//#define TETRIS_DEBUG 1

// These are for reference on building shapes:
// Each shape is formatted like...
// blockRowSize, blockColSize, 
// [Next N = blockRowSize * blockColSize]...] are
// [1 if block is filled in], [0 if block is blank] }
// 
//uint8_t iBlck0[6] = { 1, 4, 1, 1, 1, 1 }; // I-block
//
//uint8_t tBlck0[8] = { 2, 3, 0, 1, 0, 1, 1, 1 }, // T-block upside down
//uint8_t tBlck1[8] = { 3, 2, 1, 0, 1, 1, 1, 0 }; // T-block rotated -90
//uint8_t tBlck2[8] = { 3, 2, 0, 1, 1, 1, 0, 1 }; T-block rotated -180
//uint8_t tBlck4[8] = { 2, 3, 1, 1, 1, 0, 1, 0 };
//
//uint8_t lBlck0[8] = { 3, 2, 1, 0, 1, 0, 1, 1 }; // L-block normal
//uint8_t lBlck1[8] = { 3, 2, 0, 1, 0, 1, 1, 1 }; // L-block hori flipped
//uint8_t lBlck2[8] = { 2, 3, 1, 0, 0, 1, 1, 1 }; // L-block rotated -90 and hori flipped
//
//uint8_t sBlck0[8] = { 2, 3, 0, 1, 1, 1, 1, 0 }; // S-block normal
//uint8_t sBlck1[8] = { 3, 2, 1, 0, 1, 1, 0, 1 }; // S-block rotated -90
//uint8_t sBlck2[8] = { 2, 3, 1, 1, 0, 0, 1, 1 };
//
//uint8_t oBlck1[6] = { 2, 2, 1, 1, 1, 1 }; // O-block
//
//uint8_t fillerBlck0[6] = { 2, 2, 1, 1, 1, 0 }; // Filler-block, shaped like "r"
//uint8_t fillerBlck1[6] = { 1, 1, 1 }; // Filler-block single block

// Hue values of specified colors
byte blue   = 240;
byte purple = 213;  
byte orange = 20;
byte green  = 78;    // Neon-green
byte red    = 0;
byte yellow = 43;
byte cyan   = 128;

byte lightWhiteRgb = 20;

uint8_t blockSize = 11;
uint8_t tetrisSize = 18;

// See google spreadsheet for visual of this data:
// https://docs.google.com/spreadsheets/d/1-_Ugj6EqNiLKDq4XKn1Xmqe9Q-mdsAmwi_rJ1mabieA/edit#gid=807635637
uint8_t tetris[18][11] = {
  // Format is:
  // 
  // { hueColor, colDrop, rowDrop, blockRowSize, blockColSize, 
  // [Next N = blockRowSize * blockColSize]...] are
  // [1 if block is filled in], [0 if block is blank] }
  // 
  // colDrop = column index where the block will drop
  // rowDrop = rows to iterate over in the drop
  {    blue,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, // black frame 
  {  purple,  6, 4, 2, 3, 0, 1, 0, 1, 1, 1 }, // T-block upside down
  {  orange,  0, 4, 3, 2, 1, 0, 1, 0, 1, 1 }, // L-block normal
  {    blue, 10, 4, 3, 2, 0, 1, 0, 1, 1, 1 }, // L-block hori flipped
  {   green,  4, 4, 2, 3, 0, 1, 1, 1, 1, 0 }, // S-block normal
  {   green,  8, 4, 3, 2, 1, 0, 1, 1, 0, 1 }, // S-block rotated -90
  {     red,  9, 3, 3, 2, 1, 0, 1, 1, 0, 1 }, // S-block rotated -90
  {   green,  1, 4, 3, 2, 1, 0, 1, 1, 0, 1 }, // S-block rotated -90
  {  purple,  3, 4, 3, 2, 1, 0, 1, 1, 1, 0 }, // T-block rotated -90
  {  yellow, 10, 1, 2, 2, 1, 1, 1, 1, 0, 0 }, // O-block
  {    cyan,  4, 2, 1, 4, 1, 1, 1, 1, 0, 0 }, // I-block
  {  purple,  1, 2, 3, 2, 0, 1, 1, 1, 0, 1 }, // T-block rotated 90
  {    blue,  3, 1, 2, 3, 1, 0, 0, 1, 1, 1 }, // L-block rotated -90 and hori flipped
  {     red,  5, 1, 2, 3, 1, 1, 0, 0, 1, 1 }, // L-block hori flipped  
  {  purple,  7, 1, 2, 3, 1, 1, 1, 0, 1, 0 }, // T-block
  {  yellow,  0, 1, 2, 2, 1, 1, 1, 0, 0, 0 }, // Filler-block, shaped like "r"
  {    cyan,  4, 0, 1, 1, 1, 0, 0, 0, 0, 0 }, // Filler-block single block
  {    cyan,  0, 0, 0, 1, 1, 0, 0, 0, 0, 0 }  // Flash the all row
}; 

// Used to track which piece we are on
uint8_t tetrisIdx = 0;
// Used to track which row we are at in the drop animation
uint8_t unAssignedDropIdx = 255;
uint8_t rowDropIdx = unAssignedDropIdx;

// State of the ending animation
uint8_t endingStateFlashCtr = 0;

// TODO: make this based on time signature
uint8_t beatsInMeausre9624ths = 96; // we only send half in our case

// Keeps track of timing of animation triggers on assigned beats
struct BeatSequence beatTetris;
BeatSequence* getBeatTetris() {
  return &beatTetris;
}

/**
 * Draw the current tetris animation at the specified beat 
 */
void draw_Tetris(uint16_t beatNumInMeasure) {

  // For test purposes, just drop a piece every whole beat
  if (beatTetris.sequenceSize == 0) {
    beatTetris.sequence = new uint16_t[4] { 0, 24, 48, 72 };
    beatTetris.sequenceSize = 4;
//    beatTetris.sequence = new uint16_t[8] { 0, 12, 24, 36, 48, 60, 72, 84 };
//    beatTetris.sequenceSize = 8;
  }

  // Because we want to time the beats to the 
  // tetris piece hitting the bottom row.
  // We look into the future at the next piece to drop
  // and check how many rows it will have to drop
  uint8_t nextTetrisPieceRows = (tetris[(tetrisIdx + 1) % tetrisSize][2] * 2); // always make it even
  uint8_t nextBeatsInAdv = (beatNumInMeasure + nextTetrisPieceRows) % beatsInMeausre9624ths;

  // Refresh screen with off white
  FastLED_FillSolid(lightWhiteRgb, lightWhiteRgb, lightWhiteRgb);
  
  // Check if we should drop the next piece
  if ((rowDropIdx == unAssignedDropIdx) && 
        isABeat(nextBeatsInAdv, beatTetris)) {

    #ifdef TETRIS_DEBUG
      Serial.print(" Nxt Tetris.");
    #endif       

    // Go to next tetris piece
    tetrisIdx++;

    // Check for ending state
    if (tetrisIdx >= tetrisSize) {
      #ifdef TETRIS_DEBUG
        Serial.print(" Tetris strt end anim.");
      #endif          
      // Start the animation
      endingStateFlashCtr = 1; 
      rowDropIdx = 0;         
    } else {     
      // Do next tetris beat
      rowDropIdx = tetris[tetrisIdx][2];
    }
  } 

  // We are in the ending flash animation
  if (endingStateFlashCtr > 0) { 
    drawTetrisBlink();
    endingStateFlashCtr++;

    // Process ending state
    if (endingStateFlashCtr > 12) {
      endingStateFlashCtr = 0;
      rowDropIdx = unAssignedDropIdx;
      tetrisIdx = 0; 
    }  
  } // We are animating a piece drop
  else if (rowDropIdx != unAssignedDropIdx) {
    // Draw all the current pieces
    drawAllTetrisPiecesUntil(tetrisIdx);
    // Draw the animating piece
    #ifdef TETRIS_DEBUG
      Serial.print(" Draw drop ");
      Serial.print(rowDropIdx);
      Serial.print(".");
    #endif 
    drawTetrisPiece(tetris[tetrisIdx], rowDropIdx);
    rowDropIdx--;
    if (rowDropIdx == 0) {
      rowDropIdx = unAssignedDropIdx;
    }
  } else {
    // Draw the current tetris board
    drawAllTetrisPiecesUntil(tetrisIdx + 1);
  }

  #ifdef TETRIS_DEBUG
      Serial.println();
  #endif    
}

/**
 * Blink the whole tetris board
 */
void drawTetrisBlink() {
  if ((endingStateFlashCtr % 6) % 2 == 0) {
      #ifdef TETRIS_DEBUG
        Serial.print(" Ending State ON.");
      #endif  
      // Draw the tetris board filled
      drawAllTetrisPieces();
    } else {
      #ifdef TETRIS_DEBUG
        Serial.print(" Ending State OFF.");
      #endif  
      // Make all pieces black
      FastLED_FillSolid(lightWhiteRgb, lightWhiteRgb, lightWhiteRgb);
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
         #ifdef TETRIS_DEBUG
          Serial.print(" (");
          Serial.print(row);
          Serial.print(",");
          Serial.print(col);
          Serial.print(")");
        #endif 
        FastLED_lightBlockHue(row, col, tetrisPiece[0]);        
      }
      i++;
    }
  }
}

/*
 * Draw all the tetrisPieces
 */
void drawAllTetrisPieces() {
  drawAllTetrisPiecesUntil(tetrisSize);
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
    drawTetrisPiece(tetris[i], 0);
  }
}
