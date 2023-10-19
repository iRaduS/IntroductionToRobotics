#include "Potentiometer.h"
#include <Arduino.h>

/**
 * Class constructor that populates the pin of the current potentiometer object we are creating
 */
Potentiometer::Potentiometer(unsigned int potentiometerPin): potentiometerPin(potentiometerPin) {
  pinMode(this->potentiometerPin, INPUT);
}

/**
 * updateValue is a method that updates the current value of the potentiometer by reading it through
 * the analog interface
 */
Potentiometer& Potentiometer::updateValue() {
  this->currentValue = analogRead(this->potentiometerPin);
  
  return *this;
}

/**
 * getCurrentValue is a getter method that retrevies the currentValue property for the object
 */
unsigned int Potentiometer::getCurrentValue() const {
  return this->currentValue;
}
