#ifndef BUTTON_H
#define BUTTON_H

#include "Arduino.h"

class Button {
  const byte candidateThreshold = 2;

  public:
    Button(byte pin, void (*onButtonPressed)());
    void init();
    void scan();

  private:
    void setupPin();

    byte _pin;
    void (*_onButtonPressed)();

    uint8_t _consecutiveCandidateCounter;
    boolean _candidateMode;
    boolean _buttonPressed;
    boolean _candidate;
};

#endif // BUTTON_H
