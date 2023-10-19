/*
 * Potentiometer.h
 * Potentiometer class which keeps the informations such as
 * pin and current state and other methods
 */
#ifndef Potentiometer_h
#define Potentiometer_h

#include <Arduino.h>

class Potentiometer {
  private:
    const unsigned int potentiometerPin;
    unsigned int currentValue;
  public:
    Potentiometer(unsigned int);
    Potentiometer& updateValue();
    unsigned int getCurrentValue() const;
};

#endif
