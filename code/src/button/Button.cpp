#include "Arduino.h"
#include "Button.h"

Button::Button(byte pin, void (*onButtonPressed)()) {
  _pin = pin;
  _onButtonPressed = onButtonPressed;
}

void Button::init() {
  setupPin();
}

void Button::setupPin() {
  pinMode(_pin, INPUT);
  digitalWrite(_pin, HIGH);
}

void Button::scan() {
  boolean reading = !digitalRead(_pin);

  if (!_candidateMode) {
    if (reading == _buttonPressed) return;
    else {
      _candidateMode = true;
      _candidate = reading;
      _consecutiveCandidateCounter++;
      return;
    }
  }

  if (reading == _candidate) {
    _consecutiveCandidateCounter++;
    if (_consecutiveCandidateCounter > candidateThreshold) {
      _buttonPressed = _candidate;
      if (_buttonPressed) {
        _onButtonPressed();
      }
      _consecutiveCandidateCounter = 0;
      _candidateMode = false;
    }
  } else {
    _consecutiveCandidateCounter--;
    if (_consecutiveCandidateCounter == 0) {
      _candidateMode = false;
    }
  }
}
