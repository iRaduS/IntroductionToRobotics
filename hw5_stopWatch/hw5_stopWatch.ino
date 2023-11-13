#define UNIT_DISPLAY_DIGIT 2
#define OVERFLOW_NUMBER_TRICK 1e4
const int latchPin = 11;
const int clockPin = 10;
const int dataPin = 12;
const int segD1 = 4;
const int segD2 = 5;
const int segD3 = 6;
const int segD4 = 7;
const int startPauseButtonPin = 3;
const int resetButtonPin = 2;
const int flagButtonPin = 8;
int displayDigits[] = {segD1, segD2, segD3, segD4};
const int displayCount = 4;
const int encodingsNumber = 10;
byte whatToWrite = B00000000;
byte decimalPointMask = B00000001;
byte byteEncodings[encodingsNumber] = {
  B11111100, // 0
  B01100000, // 1
  B11011010, // 2
  B11110010, // 3
  B01100110, // 4
  B10110110, // 5
  B10111110, // 6
  B11100000, // 7
  B11111110, // 8
  B11110110, // 9
};
unsigned long lastIncrement = 0;
unsigned long delayCount = 100;
unsigned long number = 0;
unsigned long debounceDelay = 50;

int startPauseButtonState = HIGH;
int startPauseLastButtonState = HIGH;
unsigned long startPauseLastDebounceTime = 0;
bool isPaused = true;

int resetButtonState = HIGH;
int resetLastButtonState = HIGH;
unsigned long resetLastDebounceTime = 0;
bool hasJustReset = true;

int flagButtonState = HIGH;
int flagLastButtonState = HIGH;
unsigned long flagLastDebounceTime = 0;
const int maximumFlags = 4;
int flags[maximumFlags] = {0, 0, 0, 0};
int flagIndex = 0;
int flagShowIndex = 0;

void setup() {
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, OUTPUT);
  pinMode(startPauseButtonPin, INPUT_PULLUP);
  pinMode(resetButtonPin, INPUT_PULLUP);
  pinMode(flagButtonPin, INPUT_PULLUP);
  for (int i = 0; i < displayCount; i++) {
    pinMode(displayDigits[i], OUTPUT);
    digitalWrite(displayDigits[i], LOW);
  }
  Serial.begin(9600);
}

void loop() {
  startPauseButtonDebounce();
  resetButtonDebounce();
  flagButtonDebounce();
 
  if (!isPaused) {
    if (millis() - lastIncrement > delayCount) {
      number++;
      number %= 10000;
      lastIncrement = millis();
    }
  }
 
  writeNumber(number);
}
void writeReg(int digit) {
  digitalWrite(latchPin, LOW);
  shiftOut(dataPin, clockPin, MSBFIRST, digit);
  digitalWrite(latchPin, HIGH);
}

void activateDisplay(int displayNumber) {
  for (int i = 0; i < displayCount; i++) {
    digitalWrite(displayDigits[i], HIGH);
  }
  digitalWrite(displayDigits[displayNumber], LOW);
}

void writeNumber(int number) {
  int currentNumber = OVERFLOW_NUMBER_TRICK + number;
  Serial.println(currentNumber);
  
  int displayDigit = 3;
  int lastDigit = 0;

  while (currentNumber != 0) {
    lastDigit = currentNumber % 10;
    activateDisplay(displayDigit);

    whatToWrite = byteEncodings[lastDigit];
    if (displayDigit == UNIT_DISPLAY_DIGIT) {
      whatToWrite ^= decimalPointMask;
    }
    writeReg(whatToWrite);
    
    displayDigit--;
    currentNumber /= 10;
    writeReg(B00000000);
  }
}

void startPauseButtonDebounce() {
  int reading = digitalRead(startPauseButtonPin);

  if (reading != startPauseLastButtonState) {
    startPauseLastDebounceTime = millis();
  }

  if ((millis() - startPauseLastDebounceTime) > debounceDelay) {
    if (reading != startPauseButtonState) {
      startPauseButtonState = reading;

      if (startPauseButtonState == LOW) {
        isPaused = !isPaused;
        hasJustReset = false;
      }
    }
  }

  startPauseLastButtonState = reading;
}

void resetButtonDebounce() {
  int reading = digitalRead(resetButtonPin);

  if (reading != resetLastButtonState) {
    resetLastDebounceTime = millis();
  }

  if ((millis() - resetLastDebounceTime) > debounceDelay) {
    if (reading != resetButtonState) {
      resetButtonState = reading;

      if (resetButtonState == LOW && isPaused && !hasJustReset) {
        number = 0;
        hasJustReset = true;
      } else if (resetButtonState == LOW && hasJustReset) {
        number = 0;
        for (int i = 0; i < maximumFlags; i++) {
          flags[i] = 0;
        }
        flagIndex = 0;
        flagShowIndex = 0;
      }
    }
  }

  resetLastButtonState = reading;
}

void flagButtonDebounce() {
  int reading = digitalRead(flagButtonPin);

  if (reading != flagLastButtonState) {
    flagLastDebounceTime = millis();
  }

  if ((millis() - flagLastDebounceTime) > debounceDelay) {
    if (reading != flagButtonState) {
      flagButtonState = reading;

      if (flagButtonState == LOW && !isPaused) {
        flags[flagIndex++] = number;
        flagIndex %= maximumFlags;
      } else if (flagButtonState == LOW && hasJustReset) {
        flagShowIndex++;
        flagShowIndex %= maximumFlags;
        number = flags[flagShowIndex];
      }
    }
  }

  flagLastButtonState = reading;
}
