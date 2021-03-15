#include "Arduino.h"
#include "Matrix.h"
#include <Wire.h>

Matrix::Matrix(byte rows[], byte rowCount, byte cols[], byte colCount, byte (*onKeyPressed)(byte number)) {
  _rows = rows;
  _rowCount = rowCount;

  _cols = cols;
  _colCount = colCount;

  _onKeyPressed = onKeyPressed;

  _keys = new byte[_rowCount];
  _candidate = new byte[_rowCount];
  _reading = new byte[_rowCount];

  _startTime = millis();
}

  void Matrix::init() {
    mcp.begin();
    Wire.setClock(400000);
    setupPins();
  }

  void Matrix::setupPins() {
    for (int x = 0; x < _rowCount; x++) {
      mcp.pinMode(_rows[x], INPUT);
      mcp.pullUp(_cols[x], LOW);
    }

    for (int x = 0; x < _colCount; x++) {
      mcp.pinMode(_cols[x], INPUT);
      mcp.pullUp(_cols[x], HIGH);
    }
  }


void Matrix::scan() {
  if (millis() - _startTime >= scanDelay) {
    _startTime = millis();
    readMatrix();
    detectKeyChanges();
  }
}

void Matrix::readMatrix() {
  for (int rowIndex = 0; rowIndex < _rowCount; rowIndex++) {

    byte curRow = _rows[rowIndex];
    mcp.pinMode(curRow, OUTPUT);
    mcp.digitalWrite(curRow, LOW);

    byte tempRow = 0;
    for (int colIndex = 0; colIndex < _colCount; colIndex++) {
      byte curCol = _cols[colIndex];
      mcp.pullUp(curCol, HIGH);
      tempRow = tempRow << 1;
      tempRow = tempRow | (~mcp.digitalRead(curCol) & 0b1);
      mcp.pullUp(curCol, LOW);
    }
    _reading[rowIndex] = tempRow;

    mcp.pinMode(curRow, INPUT);
  }
}

void Matrix::detectKeyChanges() {
  if (!_candidateMode) {
    // no candidate mode
    if (sameKeys(_reading, _keys)) return;
    else {
      _candidateMode = true;
      memcpy(_candidate, _reading, _rowCount);
      _consecutiveCandidateCounter++;
      return;
    }
  }

  // candidate mode
  if (sameKeys(_reading, _candidate)) {
    // same reading
    _consecutiveCandidateCounter++;
    if (_consecutiveCandidateCounter > debounceThreshold) {
      onKeysChanged(_candidate);
      memcpy(_keys, _candidate, _rowCount);
      _consecutiveCandidateCounter = 0;
      _candidateMode = false;
    }
  } else {
    // different reading
    _consecutiveCandidateCounter--;
    if (_consecutiveCandidateCounter == 0) {
      _candidateMode = false;
    }
  }
}

bool Matrix::sameKeys(byte first[], byte second[]) {
  if (sizeof(first) != sizeof(second)) return false;

  return memcmp(first, second, _rowCount) == 0;
}

void Matrix::onKeysChanged(byte changedKeys[]) {
  for (int i = 0; i < _rowCount; i++) {
    for (int j = 0; j < _colCount; j++) {
      byte oldKeyState = (_keys[i] & powersOfTwo[_colCount - j - 1]) >> (_colCount - j - 1);
      byte newKeyState = (changedKeys[i] & powersOfTwo[_colCount - j - 1]) >> (_colCount - j - 1);
      if (oldKeyState < newKeyState) {
        _onKeyPressed(i * _colCount + j);
      }
    }
  }
}
