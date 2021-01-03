void runUnitTests() {
  runMicrosInBeat_Test();
  syncToNearest24thBeat_Test();
  syncToNearest24thBeatMicros_Test();
  syncToNearest24thBeatThroughMeasure4_4_Test();
  syncToNearest24thBeatThroughMeasure4_4_Truncate_Real_Data_Test();
  encodeMeasureOfQuantizedBeats_4_4_Beats_Test();
  encodeMeasureOfQuantizedBeats_16_16_15thBeat_Test();
  encodeMeasureOfQuantizedBeats_16_16_All16thBeats_Test();
  encodeMeasureOfQuantizedBeats_CarryOver_Beats_Test();
  encodeMeasureOfQuantizedBeats_CarryOver_Beats2_Test();
}

void runMicrosInBeat_Test() {  
  uint8_t failedCount = 0;  
  // Check within 9 decimal places
  double delta9Dec = .000000001;
  
  for (int i = 0; i < bpmSize; i++) {
    uint16_t actualBpm = bpm(microsInBeatTests[i][1]);
    if (!testEquality((uint16_t)microsInBeatTests[i][0], actualBpm)) {
      Serial.print("Failed runMicrosInBeat_Test test at index "); Serial.println(i);
      failedCount++;
    }

    double actualBeatMicros = microsInBeat((uint16_t)microsInBeatTests[i][0]);
    if (!testEqualityDouble(microsInBeatTests[i][1], actualBeatMicros, delta9Dec)) {
      Serial.print("Failed runMicrosInBeat_Test test at index "); Serial.println(i);
      failedCount++;
    }
  }

  if (failedCount == 0) {
    Serial.println("All runMicrosInBeat_Test completed successfully.");
  }
}

void syncToNearest24thBeat_Test() {
  uint8_t failedCount = 0;
  unsigned long bpmStartMicros = 0;

  for (int i = 0; i < bpmSize; i++) {
    uint16_t bpm = (uint16_t)syncTo24thTests[i][0];
    unsigned long microsOfHalfQuarterBeat = (unsigned long)syncTo24thTests[i][3];
    unsigned long actual24thBeatRoundDown = quantizeBeatTo(TwentyFourthBeat, microsOfHalfQuarterBeat - 1, bpm, bpmStartMicros);
    if (!testEqualityULong(0, actual24thBeatRoundDown)) {
      Serial.print("Failed syncToNearest24thBeat_Test test at index "); Serial.println(i);
      failedCount++;
    }
    unsigned long actual24thBeatRoundUp = quantizeBeatTo(TwentyFourthBeat, microsOfHalfQuarterBeat + 1, bpm, bpmStartMicros);
    if (!testEqualityULong(1, actual24thBeatRoundUp)) {
      Serial.print("Failed syncToNearest24thBeat_Test test at index "); Serial.println(i);
      failedCount++;
    }
  }

  if (failedCount == 0) {
    Serial.println("All syncToNearest24thBeat_Test completed successfully.");
  }
}

void syncToNearest24thBeatMicros_Test() {
  uint8_t failedCount = 0;
  // Check within 9 decimal places
  double delta9Dec = .000000001;
  unsigned long bpmStartMicros = 0;

  for (int i = 0; i < bpmSize; i++) {
    uint16_t bpm = (uint16_t)syncTo24thTests[i][0];
    unsigned long microsOfHalfQuarterBeat = (unsigned long)syncTo24thTests[i][3];
    double actual24thBeatRoundDownMicros = quantizeBeatToInMicros(TwentyFourthBeat, microsOfHalfQuarterBeat - 1, bpm, bpmStartMicros);
    if (!testEqualityDouble(0.0, actual24thBeatRoundDownMicros, delta9Dec)) {
      Serial.print("Failed syncToNearest24thBeatMicros_Test test at index "); Serial.println(i);
      failedCount++;
    }
    double actual24thBeatRoundUpMicros = quantizeBeatToInMicros(TwentyFourthBeat, microsOfHalfQuarterBeat + 1, bpm, bpmStartMicros);
    if (!testEqualityDouble(syncTo24thTests[i][2], actual24thBeatRoundUpMicros, delta9Dec)) {      
      Serial.print(" Failed syncToNearest24thBeatMicros_Test test at index "); Serial.println(i);
      failedCount++;
    }
  }

  if (failedCount == 0) {
    Serial.println("All syncToNearest24thBeatMicros_Test completed successfully.");
  }
}

void syncToNearest24thBeatThroughMeasure4_4_Test() {
  uint8_t failedCount = 0;
  unsigned long bpmStartMicros = 0;

  for (int i = 0; i < bpmSize; i++) {
    uint16_t bpm = (uint16_t)syncTo24thTests[i][0];
    uint8_t beatsPerMeasure = 4;
    double endOfMeasure1 = syncTo24thTests[i][1] * beatsPerMeasure;    

    // Micros of the first measure
    unsigned long microsOfHalfQuarterBeat = (unsigned long)(syncTo24thTests[i][3] + endOfMeasure1);
    
    uint16_t actual24thBeatRoundDown = quantizeBeatWithinMeasureTo(TwentyFourthBeat, beatsPerMeasure, microsOfHalfQuarterBeat - 1, bpm, bpmStartMicros);
    if (!testEquality(0, actual24thBeatRoundDown)) {
      Serial.print("Failed syncToNearest24thBeatThroughMeasure4_4_Test test at index "); Serial.println(i);
      failedCount++;
    }
    uint16_t actual24thBeatRoundUp = quantizeBeatWithinMeasureTo(TwentyFourthBeat, beatsPerMeasure, microsOfHalfQuarterBeat + 1, bpm, bpmStartMicros);
    if (!testEquality(1, actual24thBeatRoundUp)) {      
      Serial.print(" Failed syncToNearest24thBeatThroughMeasure4_4_Test test at index "); Serial.println(i);
      failedCount++;
    }
  }

  if (failedCount == 0) {
    Serial.println("All syncToNearest24thBeatThroughMeasure4_4_Test completed successfully.");
  }
}

void syncToNearest24thBeatThroughMeasure4_4_Truncate_Real_Data_Test() {
  uint8_t failedCount = 0;
  unsigned long bpmStartTimeMicros = a24thTruncateStarTimeMicros;
  uint16_t division = 24;
  uint16_t beatsPerMeasure = 4;
  uint16_t bpm = a24thTruncateBpm;
  for(int i = 0; i < 96; i++) {
    uint16_t actual = quantizeBeatWithinMeasureTruncate(division, beatsPerMeasure, testa24thTruncateMicros[i], bpm, bpmStartTimeMicros);
    if (!testEquality(testa24thTruncateBeats[i], actual)) {
      Serial.print(" Failed syncToNearest24thBeatThroughMeasure4_4_Truncate_Real_Data_Test test at index "); Serial.println(i);
      failedCount++;
    }
  }

  if (failedCount == 0) {
    Serial.println("All syncToNearest24thBeatThroughMeasure4_4_Truncate_Real_Data_Test completed successfully.");
  }
}

//    fun syncToNearest24thBeatThroughMeasure4_4_Truncate_Real_Data_Test() {
//        val bpmStartTimeMicros = a24thTruncateStarTimeMicros
//        val timeSig = TimeSignatureEnum.FOUR_FOUR.value
//        val bpm = a24thTruncateBpm
//        for (i in 0 until testa24thTruncateMicros.size) {
//            val actual = Utils.quantizeBeatWithinMeasureToTruncate(
//                    Utils.BeatDivision.TWENTY_FOURTH, timeSig,
//                    testa24thTruncateMicros[i], bpm, bpmStartTimeMicros)
//            assertEquals(testa24thTruncateBeats[i], actual)
//        }
//    }

void encodeMeasureOfQuantizedBeats_4_4_Beats_Test() {
  struct BeatSequence beats;
  decodeAndAppendBeatSequence(fourFourBeatTapsMsg, 20, &beats);

  if (!testEqualityUInt16Array(4, beats.sequenceSize, fourFourBeatTaps, beats.sequence)) {
    Serial.println("encodeMeasureOfQuantizedBeats_4_4_Beats_Test test failed.");
  } else {
    Serial.println("All encodeMeasureOfQuantizedBeats_4_4_Beats_Test completed successfully.");
  }
}

void encodeMeasureOfQuantizedBeats_16_16_15thBeat_Test() {
  struct BeatSequence beats;
  decodeAndAppendBeatSequence(sixteenSixteenLastBeatTapMsg, 20, &beats);

  if (!testEqualityUInt16Array(1, beats.sequenceSize, sixteenSixteenLastBeatTap, beats.sequence)) {
    Serial.println("encodeMeasureOfQuantizedBeats_16_16_15thBeat_Test test failed.");
  } else {
    Serial.println("All encodeMeasureOfQuantizedBeats_16_16_15thBeat_Test completed successfully.");
  }
}

void encodeMeasureOfQuantizedBeats_16_16_All16thBeats_Test() {
  struct BeatSequence beats;  

  for(int i = 0; i < 4; i++) {
    decodeAndAppendBeatSequence(sixteenSixteenSixteenthBeatsMsg[i], 20, &beats);    
  }

  if (!testEqualityUInt16Array(64, beats.sequenceSize, sixteenSixteenSixteenthBeats, beats.sequence)) {
    Serial.println("encodeMeasureOfQuantizedBeats_16_16_All16thBeats_Test test failed.");
  } else {
    Serial.println("All encodeMeasureOfQuantizedBeats_16_16_All16thBeats_Test completed successfully.");
  }
}

void encodeMeasureOfQuantizedBeats_CarryOver_Beats_Test() {
  struct BeatSequence beats;  

  for(int i = 0; i < 2; i++) {
    decodeAndAppendBeatSequence(carryOverBeatsMsg[i], 20, &beats);    
  }

  if (!testEqualityUInt16Array(18, beats.sequenceSize, carryOverBeats, beats.sequence)) {
    Serial.println("encodeMeasureOfQuantizedBeats_CarryOver_Beats_Test test failed.");
  } else {
    Serial.println("All encodeMeasureOfQuantizedBeats_CarryOver_Beats_Test completed successfully.");
  }
}

void encodeMeasureOfQuantizedBeats_CarryOver_Beats2_Test() {
  struct BeatSequence beats;  
  decodeAndAppendBeatSequence(carryOverBeats2Msg, 20, &beats);    

  if (!testEqualityUInt16Array(16, beats.sequenceSize, carryOverBeats2, beats.sequence)) {
    Serial.println("encodeMeasureOfQuantizedBeats_CarryOver_Beats_Test2 test failed.");
  } else {
    Serial.println("All encodeMeasureOfQuantizedBeats_CarryOver_Beats_Test2 completed successfully.");
  }
}

bool testEqualityDouble(double expected, double actual, double delta) {
  if (abs(expected - actual) > delta) {
    Serial.print("Test failed: expected ");
    Serial.print(expected);
    Serial.print(", actual ");
    Serial.println(actual);
    return false;
  }
  return true;
}

bool testEquality(uint16_t expected, uint16_t actual) {
  if (expected != actual) {
    Serial.print("Test failed: expected ");
    Serial.print(expected);
    Serial.print(", actual ");
    Serial.println(actual);
    return false;
  }
  return true;
}

bool testEqualityULong(unsigned long expected, unsigned long actual) {
  if (expected != actual) {
    Serial.print("Test failed: expected ");
    Serial.print(expected);
    Serial.print(", actual ");
    Serial.println(actual);
    return false;
  }
  return true;
}

bool testEqualityUInt16Array(int expectedSize, int actualSize, uint16_t* expected, uint16_t* actual) {
  if (expectedSize != actualSize) {
    Serial.print("Size test failed: expectedSize ");
    Serial.print(expectedSize);
    Serial.print(", actualSize ");
    Serial.println(actualSize);
    return false;
  } 
  for (int i = 0; i < expectedSize; i++) {
    if (expected[i] != actual[i]) {
      Serial.print("Test failed: expected ");
      Serial.print(expected[i]);
      Serial.print(", actual ");
      Serial.print(actual[i]);      
      Serial.print(", at index ");
      Serial.println(i);
      return false;
    } 
  }  
  return true;
}
