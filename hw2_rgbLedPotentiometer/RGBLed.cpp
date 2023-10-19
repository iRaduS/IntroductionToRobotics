#include "RGBLed.h"
#include <Arduino.h>

/**
 * Class constructor that populates the pins of the current RGB Led object we are creating
 */
RGBLed::RGBLed(unsigned int redChannelPin, unsigned int blueChannelPin, unsigned int greenChannelPin) : 
  redChannelPin(redChannelPin), blueChannelPin(blueChannelPin), greenChannelPin(greenChannelPin) { 
    pinMode(this->redChannelPin, OUTPUT);
    pinMode(this->blueChannelPin, OUTPUT);
    pinMode(this->greenChannelPin, OUTPUT);
}

/**
 * getRedChannelPin getter method to retrieve the red channel pin value of the RGB led
 */
unsigned int RGBLed::getRedChannelPin() const {
  return this->redChannelPin;
}

/**
 * getBlueChannelPin getter method to retrieve the blue channel pin value of the RGB led
 */
unsigned int RGBLed::getBlueChannelPin() const {
  return this->blueChannelPin;
}

/**
 * getGreenChannelPin getter method to retrieve the green channel pin value of the RGB led
 */
unsigned int RGBLed::getGreenChannelPin() const {
  return this->greenChannelPin;
}
