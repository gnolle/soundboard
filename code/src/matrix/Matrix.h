#ifndef MATRIX_H
#define MATRIX_H

#include <Adafruit_MCP23017.h>
#include "Arduino.h"

const byte powersOfTwo[8] = {1, 2, 4, 8, 16, 32, 64, 128};
const byte debounceThreshold = 3;
const unsigned long scanDelay = 1;

class Matrix {
public:
  Matrix(byte rows[], byte rowCount, byte cols[], byte colCount, byte (*onKeyPressed)(byte number));
  void init();
  void scan();

private:
  void setupPins();
  void readMatrix();
  void detectKeyChanges();
  bool sameKeys(byte *first, byte *second);
  void onKeysChanged(byte *changedKeys);

  byte *_rows;
  byte *_cols;

  int _rowCount;
  int _colCount;

  byte *_keys;
  byte *_candidate;
  byte *_reading;

  byte _consecutiveCandidateCounter;

  bool _candidateMode;

  unsigned long _startTime;

  byte (*_onKeyPressed)(byte number);

  Adafruit_MCP23017 mcp;

};

#endif // MATRIX_H
