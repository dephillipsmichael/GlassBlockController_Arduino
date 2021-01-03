// One minute in microseconds
const double perMinuteMicros = 60000000.00;

/**
 * Beat division is a way to divide beats by factors common in musical notes
 * 24 is used as the transmit protocol for BLE, for the same reasons that
 * MIDI protocol chose 24 as the beat division.
 * Its the minimal division that works with all time signatures and triplet notes.
 *
 * The rest of them are used to quantize music in more common (4/4) patterns.
 */
extern const uint16_t FourthBeat = 4;
extern const uint16_t EighthBeat = 8;
extern const uint16_t SixteenthBeat = 16;
extern const uint16_t TwentyFourthBeat = 24;

/**
 * Time signature beats per measure
 */
extern const uint16_t FOUR_FOUR = 4;
extern const uint16_t EIGHT_EIGHT = 8;
extern const uint16_t SIXTEEN_SIXTEEN = 16;

// Beat measure characteristic data length
const uint8_t beatMeasueDataLength = 20;
// Start command byte
const byte startSequenceByte = 1;
// Append part to previous sequence messages
const byte appendSequenceByte = 2;

/**
 * A beat sequence represents quantized 24th beat impulses
 */
struct BeatSequence {
  uint16_t* sequence = NULL;
  uint16_t sequenceSize = 0;
  uint16_t lastBeatValue = 0;
};

/**
 * @return 1 unit of the division per the parameter
 */
double oneBeat(uint16_t perThBeat) {
  return 1.0 / (double)(perThBeat);
}

/**
 * @return true if number of 24th beats is divisible by whole beat
 */
boolean isWholeBeat(uint16_t beatNumIn24th) {
  return (beatNumIn24th % TwentyFourthBeat) == 0;
}

/**
 * @return true if number of 24th beats is divisible by half beat
 */
boolean isHalfBeat24th(uint16_t beatNumIn24th) {
  return (beatNumIn24th % (TwentyFourthBeat / 2)) == 0;
}

/**
 * @return true if number of 24th beats is divisible by quarter beat
 */
boolean isQuarterBeat24th(uint16_t beatNumIn24th) {
  return (beatNumIn24th % (TwentyFourthBeat / 4)) == 0;
}

/**
 * @param timeInMicros of current beat
 * @param bpm beats per minute
 * @param bpmStartTimeMicros start time of bpm counting in microseconds
 * @return the whole and fractional number of beats in double format
 */
double beatsElapsed(unsigned long timeInMicros, uint16_t bpm, unsigned long bpmStartTimeMicros) {
  return (timeInMicros - bpmStartTimeMicros) / microsInBeat(bpm);
}

/**
 * @param timeSignature with valid beats per measure
 * @param beatsElapsed results of beatsElapsed function
 * @return the number of beats through the current measure
 */
double beatsThroughMeasure(uint16_t beatsPerMeasure, double beatsElapsed) {
  // beatsElapsed % beatsPerMeasure
  double beatsElapsedDivision = beatsElapsed / (double)beatsPerMeasure; 
  return beatsElapsed - ((uint16_t)beatsElapsedDivision * beatsPerMeasure);
}

/**
 * @param bpm beats per minute
 * @return number of milliseconds in a beat in double precision
 */
double microsInBeat(uint16_t bpm) {
  return perMinuteMicros / bpm;
}

/**
 * @return the bpm value rounded to a whole number
 */
uint16_t bpm(double microsInBeat) {
    return (uint16_t)round(perMinuteMicros / microsInBeat);
}

/**
 * @param division the divisional unit of the beat
 * @param timeInMicros of current beat
 * @param bpm beats per minute
 * @param bpmStartTimeMicros start time of bpm counting in microseconds
 * @return the number of 24th beats in long format
 */
unsigned long quantizeBeatTo(uint16_t division, unsigned long timeInMicros, uint16_t bpm, unsigned long bpmStartTimeMicros) {

// Kotlin code:
//    val numberOfBeats = beatsElapsed(timeInMicros, bpm, bpmStartTimeMicros)
//    val numberOfDivisionalBeats = numberOfBeats / division.one
//    return round(numberOfDivisionalBeats).toLong()

    return (unsigned long)round(beatsElapsed(timeInMicros, bpm, bpmStartTimeMicros) / oneBeat(division));
}

/**
 * @param division the divisional unit of the beat
 * @param timeInMicros of current beat
 * @param bpm beats per minute
 * @param bpmStartTimeMicros start time of bpm counting in microseconds
 * @return the timestamp in microseconds of the time synced to closest 24th beat
 */
double quantizeBeatToInMicros(uint16_t division, unsigned long timeInMicros, uint16_t bpm, unsigned long bpmStartTimeMicros) {
  
// Kotlin code:  
//    val roundedNumberOfDivisionalBeats =
//            quantizeBeatTo(division, timeInMicros, bpm, bpmStartTimeMicros)
//    val roundedNumberOfBeats =
//            roundedNumberOfDivisionalBeats.toDouble() / division.divisor.toDouble()
//    return roundedNumberOfBeats * microsInBeat(bpm)

  return ((double)quantizeBeatTo(division, timeInMicros, bpm, bpmStartTimeMicros) / division) * microsInBeat(bpm);
}

/**
 * @param division the divisional unit of the beat
 * @param timeInMicros of current beat
 * @param bpm beats per minute
 * @param bpmStartTimeMicros start time of bpm counting in microseconds
 * @return the number of 24th beats through the current measure that the time is rounded to
 */
uint16_t quantizeBeatWithinMeasureTo(uint16_t division, uint16_t beatsPerMeasure,
                                     unsigned long timeInMicros, uint16_t bpm, 
                                     unsigned long bpmStartTimeMicros) {

// Kotlin code:
//    val beatsElapsed = beatsElapsed(timeInMicros, bpm, bpmStartTimeMicros)
//    val beatsThroughMeasure = beatsThroughMeasure(beatsPerMeasure, beatsElapsed)
//    val numberOfDivisionalBeats = beatsThroughMeasure / division.one
//    return round(numberOfDivisionalBeats).toInt()

  return (uint16_t)round(beatsThroughMeasure(beatsPerMeasure, beatsElapsed(timeInMicros, bpm, bpmStartTimeMicros)) / oneBeat(division));
}

/**
 * Same as function above, but truncates the remainder of the beat division instead of rounding it
 */
uint16_t quantizeBeatWithinMeasureTruncate(uint16_t division, uint8_t beatsPerMeasure,
                                           unsigned long timeInMicros, uint16_t bpm, 
                                           unsigned long bpmStartTimeMicros) {
                                            
  return (uint16_t)(beatsThroughMeasure(beatsPerMeasure, beatsElapsed(timeInMicros, bpm, bpmStartTimeMicros)) / oneBeat(division));
}


/**
 * To efficiently send quantized beat sequences through BLE,
 * we quantize all user taps to within a 24th beat in the loop (like MIDI).
 * A loop is a musical measure in the specified time signature.
 *
 * Each byte value between the start and end commands
 * represents a 24th beat, and the actual
 * time in milliseconds these represent is computed by the receiver
 * using BPM, BPM time offset, and time signature.
 *
 * The beat sequence message format is as follows:
 * [startSequenceByte][animation_index = AI in these examples]
 * [0 if starting on a beat, or 24th beats to first beat]
 * [distance in 24th beats from first beat to second beat]
 * [distance in 24th beats from first beat to second beat...
 * but 255 if distance > 255 (10x 24th beats), and next byte is added to 255]
 * [endSequenceByte]
 *
 * Take a simple example of 4/4 with the user tapping on the beat, the sequence would be:
 * [1][AI]][0][24][24][24]
 *
 * Take a more complicated example of 16/16 with the user tapping
 * on only the last beat (15/16) in the measure:
 * [1][AI][255][105]
 * Here we had to split the tap into 2 bytes as there is 360 24th beats
 * to the 15/16th beat in the measure, and a byte only has 255 values.
 *
 * Take an even more complicated example of when we need to split the message into
 * multiple ByteArrays, due to the limitation in BLE MTU size matching characteristic size.
 * Say we want to send a measure with beats every 16th note in a 16/16 time signature.
 * The byte sequence would look like this:
 * [1][AI][0][6][6][6][6][6][6][6][6][6][6][6][6][6][6][6][6][6]
 * [2][AI][6][6][6][6][6][6][6][6][6][6][6][6][6][6][6][6][6][6]
 * [2][AI][6][6][6][6][6][6][6][6][6][6][6][6][6][6][6][6][6][6]
 * [2][AI][6][6][6][6][6][6][6][6][6][6]
 *
 * Take an even EVEN more complicated example of when we need to split the message into
 * multiple ByteArrays, with the first array ending on 255.  17 32nd notes and then a long rest.
 * The byte sequence would look like this:
 * [1][AI][0][3][3][3][3][3][3][3][3][3][3][3][3][3][3][3][3][255]
 * [2][AI][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0][0]
 *
 * This takes 4 messages to send that sequence.
 * See these examples as Unit Tests in decodeMeasureOfQuantizedBeats_Test()
 *
 * This function decodes messages that are encoded like above in encodeMeasureOfQuantizedBeats.
 * @param lastBeatVal in the current encodedMessage
 * @param beatSequence the current beat sequence for the designated animation
 * @param startBeatValue this is the value at which we should start counting at
 *                       when decoding the next message
 *
 * @return the new beatSequence value to be used
 */
void decodeAndAppendBeatSequence(byte* encodedMessage, uint8_t encodedMessageSize, struct BeatSequence* beats) {
    // First two bytes always designate message type and the animation index
    byte messageType = encodedMessage[0];
    // The animation index has already been used to get the corresponding BeatSequence in BluetoothComm
    // byte animationIndex = encodedMessage[1]

    // Find the length of the beat sequence section of the message
    // The end is signaled by a zero after the 3rd byte
    uint16_t i = 3;
    uint8_t accurateMessageSize = 0;  // set to 1 to get it to run
    boolean found = false;
    uint8_t count255 = 0;             // Count the number of 255 values
    
    if (encodedMessage[2] == 255) {
      count255++;
    }
    while (!found && (i < encodedMessageSize)) {
        // Check for end of the message, if these conditions are met
        if (encodedMessage[i] == 0 &&
            (encodedMessage[i - 1] != 255)) {
            accurateMessageSize = i - 2 - count255;
            found = true;
        } else if (encodedMessage[i] == 255) {
            // Ignore 255 in the message count,
            // because it is a continuing sum
            count255++;
        }
        i++;
    }

    // The whole beat sequence in the message is valid
    if (accurateMessageSize == 0) {
        accurateMessageSize = encodedMessageSize - 2 - count255;
    }

    // If we are starting the sequence, erase all previous data
    uint16_t newBeatSequenceSize = beats->sequenceSize + accurateMessageSize;
    uint16_t newLastBeatValIn24th = 0;
    i = 0;
    if (messageType == startSequenceByte) {
        newBeatSequenceSize = accurateMessageSize;
        newLastBeatValIn24th = 0;
    }
    // Create a new array with the new size
    uint16_t* newBeatSequence = (uint16_t*)malloc(newBeatSequenceSize * sizeof(uint16_t));
    // Fill up the array, depending on the messageType
    if (messageType == appendSequenceByte) {
      for (int copyI = 0; copyI < beats->sequenceSize; copyI++) {
        newBeatSequence[copyI] = beats->sequence[copyI];
      }
      i = beats->sequenceSize;
      newLastBeatValIn24th = beats->lastBeatValue;
    }    

    // Now, fill in the rest of the translated beat sequence to full time signature beat scale
    // Skipping first 2 bytes for messageType and animationIdx
    for (int encodedIdx = 2; encodedIdx < (accurateMessageSize + 2 + count255); encodedIdx++) {
        // Always add to the running total
        newLastBeatValIn24th += encodedMessage[encodedIdx];
        // 255 is a case where it is considered "carried over"
        // to the next sum, and not an actual beat value
        if (encodedMessage[encodedIdx] != 255) {
            newBeatSequence[i] = newLastBeatValIn24th;
            i++;
        }
    }

    // In the translated arduino code, here we must...
    // Delete old memory here in c
    // De-reference passed in lastBeatVal here in C
    if (beats->sequenceSize > 0) {
      // Freeing a NULL pointer on Arduino can 
      // cause all sorts of problems
      free(beats->sequence);
    }
    beats->sequence = newBeatSequence;
    beats->sequenceSize = newBeatSequenceSize;
    beats->lastBeatValue = newLastBeatValIn24th;
}
