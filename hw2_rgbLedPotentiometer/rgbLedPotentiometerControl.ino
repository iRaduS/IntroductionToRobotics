#include "Potentiometer.h"
#include "RGBLed.h"

Potentiometer redLEDPotentiometer(A0), blueLEDPotentiometer(A1), greenLEDPotentiometer(A2);
RGBLed rgbLed(11, 10, 9);
const unsigned int maximumInputValue = (1 << 10), maximumOutputValue = (1 << 8);

/**
 * computeRGBComponentValue is a function that compute from a potentiometer the state and transform it to the optimal value
 * distribution that it needs to output
 */
unsigned int computeRGBComponentValue(Potentiometer& potentiometer) {
  return map(potentiometer.updateValue().getCurrentValue(), 0, maximumInputValue, 0, maximumOutputValue);
}

void setup() {
  // empty method due to initalization inside the class constructor of Potentiometer and RGBLed
}

void loop() {
  analogWrite(rgbLed.getRedChannelPin(), computeRGBComponentValue(redLEDPotentiometer));
  analogWrite(rgbLed.getGreenChannelPin(), computeRGBComponentValue(greenLEDPotentiometer));
  analogWrite(rgbLed.getBlueChannelPin(), computeRGBComponentValue(blueLEDPotentiometer));
}
