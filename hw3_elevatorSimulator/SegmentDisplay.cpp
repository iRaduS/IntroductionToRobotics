#include "SegmentDisplay.h"
#include <Arduino.h>

/**
 * Grid representation of the following symbols
 * P for the ground floor
 * 1 for the first floor
 * 2 for the second floor
 */
const bool SegmentDisplay::displayConfiguration[3][8] = {
  {1, 1, 1, 1, 1, 0, 0, 1},
  {1, 0, 0, 0, 0, 0, 1, 1},
  {1, 1, 0, 1, 1, 1, 0, 1}
};

// Number of connected pins to display informations.
const unsigned int SegmentDisplay::numberOfPins = 8;

/**
 * Class constructor for initialization of the segment display.
 */
SegmentDisplay::SegmentDisplay(unsigned int *displayPins) 
  : displayPins(new unsigned int[SegmentDisplay::numberOfPins]) {
  for (unsigned int i = 0; i < SegmentDisplay::numberOfPins; i++) {
    this->displayPins[i] = displayPins[i];

    pinMode(this->displayPins[i], OUTPUT);
  }
}

/**
 * Class destructor used to free build up memory from the pins array.
 */
SegmentDisplay::~SegmentDisplay() {
  if (this->displayPins != NULL) {
    delete[] this->displayPins;
  }
}

/**
 * Set on each segment the corresponding state to draw the symbol.
 */
void SegmentDisplay::updateDisplayWithFloorNumber(unsigned int floorNo) const {
  for (unsigned int i = 0; i < SegmentDisplay::numberOfPins; i++) {
    digitalWrite(this->displayPins[i], SegmentDisplay::displayConfiguration[floorNo][i]);
  }
}
