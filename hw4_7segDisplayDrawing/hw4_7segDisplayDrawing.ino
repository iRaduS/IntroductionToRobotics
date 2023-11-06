// Declare all the joystick pins
const int pinSW = 2; // Digital pin connected to switch output
const int pinX = A0; // A0 - Analog pin connected to X output
const int pinY = A1; // A1 - Analog pin connected to Y output

// Declare all the segments pins
const int pinA = 12;
const int pinB = 10;
const int pinC = 9;
const int pinD = 8;
const int pinE = 7;
const int pinF = 6;
const int pinG = 5;
const int pinDP = 4;

// Declare an enum that will make the code a bit cleaner
enum SegmentDisplayPinEnum {
  pA = pinA,
  pB = pinB,
  pC = pinC,
  pD = pinD,
  pE = pinE,
  pF = pinF,
  pG = pinG,
  pDP = pinDP
};

// Declare the size of the array of pins from the segment display
const unsigned int segSize = 8;
unsigned int segmentsPin[segSize] = {
  pinA, pinB, pinC, pinD, pinE, pinF, pinG, pinDP
};

// Declare Joystick reading values of the X-axis and Y-axis
struct JoystickValue {
  unsigned int xAxis, yAxis;
};
// rawReading fluxuates between 0-1023
// normalizedReading will be a map that will reduce fluctuations in reading between 0-63
JoystickValue rawReading, normalizedReading;

const unsigned int maximumMappedValue = (1 << 4) - 1, normalPositionAxis = (maximumMappedValue >> 1);

// Declare for xAxis and yAxis of the joystick if it's still not in a normal position
// Declare aquiredLock if the joystick is moved diagonaly -> xAxis and yAxis are moved together
bool isXAxisNeutral = true, 
     isYAxisNeutral = true, 
     oldIsXAxisNeutral = true, 
     oldIsYAxisNeutral = true,
     aquiredLock = false;

// Declare the current position of our segment
unsigned int currentSegment = pDP;
unsigned long previousBlinkingTime;
byte currentSegmentState = LOW;
const unsigned int SEGMENT_ON_INTERVAL = 500;

// Declare the array of the segments that are light up or not
bool isActivatedSegment[segSize];
bool restartSequenceInitiated = false;
const unsigned int RESTART_TIME = 3000;

// Declare the joystick debounce variables
unsigned long lastDebounceTime = 0, restartTime = 0;
byte lastButtonState = HIGH, buttonState = HIGH;
const unsigned int DEBOUNCE_TIME = 50;

void setup() {
  Serial.begin(9600);
  for (unsigned int i = 0; i < segSize; i++) {
    pinMode(segmentsPin[i], OUTPUT);
  }
  pinMode(pinSW, INPUT_PULLUP);
}

void loop() {
  debounceButton();
  for (unsigned int i = 0; i < segSize; i++) {
    if (isActivatedSegment[i] && currentSegment != segmentsPin[i]) {
      digitalWrite(segmentsPin[i], HIGH);
      continue;
    }

    digitalWrite(segmentsPin[i], LOW);
  }
  if ((millis() - previousBlinkingTime) > SEGMENT_ON_INTERVAL) {
    currentSegmentState = !currentSegmentState;
    previousBlinkingTime = millis();
  }
  digitalWrite(currentSegment, currentSegmentState);
  
  
  rawReading.xAxis = analogRead(pinX);
  rawReading.yAxis = analogRead(pinY);

  normalizedReading.xAxis = map(rawReading.xAxis, 0, 1023, 0, maximumMappedValue);
  normalizedReading.yAxis = map(rawReading.yAxis, 0, 1023, 0, maximumMappedValue);

  isXAxisNeutral = normalizedReading.xAxis == normalPositionAxis;
  isYAxisNeutral = normalizedReading.yAxis == normalPositionAxis;
  if (isXAxisNeutral && isYAxisNeutral) {
    aquiredLock = false;
  }

  if (!isXAxisNeutral && oldIsXAxisNeutral != isXAxisNeutral && !aquiredLock) {
    Serial.println("X-axis was moved.");
    
    aquiredLock = true;
    moveToNeighbourXAxis(currentSegment, normalizedReading.xAxis);
  }

  if (!isYAxisNeutral && oldIsYAxisNeutral != isYAxisNeutral && !aquiredLock) {
    Serial.println("Y-axis was moved. This is the debug for Y-axis.");

    aquiredLock = true;
    moveToNeighbourYAxis(currentSegment, normalizedReading.yAxis);
  }

  oldIsXAxisNeutral = isXAxisNeutral;
  oldIsYAxisNeutral = isYAxisNeutral;
}

unsigned int returnSegment(unsigned int reading, SegmentDisplayPinEnum lowerBound, SegmentDisplayPinEnum upperBound) {
  return reading < normalPositionAxis ? lowerBound : upperBound;
}

void moveToNeighbourXAxis(unsigned int &currentSeg, unsigned int xAxisReading) {
  switch (currentSeg) {
    case pA: {
      currentSeg = returnSegment(xAxisReading, pG, pA);
      break;
    }
    case pB: {
      currentSeg = returnSegment(xAxisReading, pG, pA);
      break;
    }
    case pC: {
      currentSeg = returnSegment(xAxisReading, pD, pG);
      break;
    }
    case pD: {
      currentSeg = returnSegment(xAxisReading, pD, pG);
      break;
    }
    case pE: {
      currentSeg = returnSegment(xAxisReading, pD, pG);
      break;
    }
    case pF: {
      currentSeg = returnSegment(xAxisReading, pG, pA);
      break;
    }
    case pG: {
      currentSeg = returnSegment(xAxisReading, pD, pA);
      break;
    }
    default: {
      currentSeg = currentSeg;
    }
  }
}

void moveToNeighbourYAxis(unsigned int &currentSeg, unsigned int yAxisReading) {
  switch (currentSeg) {
    case pA: {
      currentSeg = returnSegment(yAxisReading, pF, pB);
      break;
    }
    case pB: {
      currentSeg = returnSegment(yAxisReading, pF, pB);
      break;
    }
    case pC: {
      currentSeg = returnSegment(yAxisReading, pE, pDP);
      break;
    }
    case pD: {
      currentSeg = returnSegment(yAxisReading, pE, pC);
      break;
    }
    case pE: {
      currentSeg = returnSegment(yAxisReading, pE, pC);
      break;
    }
    case pF: {
      currentSeg = returnSegment(yAxisReading, pF, pB);
      break;
    }
    case pDP: {
      currentSeg = returnSegment(yAxisReading, pC, pDP);
      break;
    }
    default: {
      currentSeg = currentSeg;
    }
  }
}

void debounceButton() {
  int reading = digitalRead(pinSW);

  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > DEBOUNCE_TIME) {
    if (reading != buttonState) {
      buttonState = reading;

      if (buttonState == HIGH && !restartSequenceInitiated) {
        Serial.println("Button is pressed.");
        for (unsigned int i = 0; i < segSize; i++) {
          if (currentSegment != segmentsPin[i]) {
            continue;
          }

          isActivatedSegment[i] = true;
        }
      } else if (buttonState == HIGH && restartSequenceInitiated) {
        restartSequenceInitiated = false;
      }

      if (buttonState == LOW) {
        restartTime = millis();
      }
    } else {
      if (reading == LOW && (millis() - restartTime) >= RESTART_TIME && !restartSequenceInitiated) {
        restartSequenceInitiated = true;
        currentSegment = pDP;

        for (unsigned int i = 0; i < segSize; i++) {
          isActivatedSegment[i] = false;
        }
      }
    }
  }

  lastButtonState = reading;
}
