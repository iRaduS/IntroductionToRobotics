/*
 * RGBLed.h
 * RGBLed class which keeps the informations such as
 * pin and current state and other methods
 */
#ifndef RGBLed_h
#define RGBLed_h

#include <Arduino.h>

class RGBLed {
  private:
    unsigned int redChannelPin, blueChannelPin, greenChannelPin;
  public:
    RGBLed(unsigned int, unsigned int, unsigned int);
    unsigned int getRedChannelPin() const;
    unsigned int getBlueChannelPin() const;
    unsigned int getGreenChannelPin() const;
};

#endif
