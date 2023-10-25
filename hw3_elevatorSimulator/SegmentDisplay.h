#ifndef SegmentDisplay_h
#define SegmentDisplay_h

#include <Arduino.h>

class SegmentDisplay {
  private:
    static const bool displayConfiguration[3][8];
    static const unsigned int numberOfPins;
    unsigned int *displayPins;
  public:
    SegmentDisplay(unsigned int*);
    ~SegmentDisplay();
    void updateDisplayWithFloorNumber(unsigned int) const;
};

#endif
