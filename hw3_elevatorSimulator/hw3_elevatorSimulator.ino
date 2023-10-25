#include "SegmentDisplay.h"
#define NOTE_G6 1568
#define NOTE_D6 1175
#define DEFAULT_BUZZER_FREQUENCY 1
#define MAX_CLOSING_DOOR_TIME 2000
#define STATUS_LED_HIGH_TIME_INTERVAL 1000
#define STATUS_LED_LOW_TIME_INTERVAL 500
#define TRANSIT_MOVE_LIFT 2000

unsigned int segmentDisplayPins[] = {1, 3, 4, 5, 6, 7, 8, 9},
  groundFloorButtonPin = 14, firstFloorButtonPin = 15, secondFloorButtonPin = 16,
  statusLedPin = 10, groundFloorLedPin = 11, firstFloorLedPin = 12, secondFloorLedPin = 13,
  buzzerPin = 2;
unsigned int *decisionQueue, *oldDecisionQueue, queueIndex = 0, decisionQueueSize = 1, currentFloor = 0;
unsigned int groundFloorButtonStatus, firstFloorButtonStatus, secondFloorButtonStatus;
unsigned int lastDebounceTime = 0, currentDebounceTime, currentMillis, closingDoorTime = 0, transitTime = 0, liftStatus = 0;
const SegmentDisplay segmentDisplay(segmentDisplayPins);

unsigned long debounceDelay = 50; // Adjust this value as needed
unsigned long lastGroundFloorButtonTime = 0, lastFirstFloorButtonTime = 0, lastSecondFloorButtonTime = 0;
bool lastGroundFloorButtonState = HIGH, lastFirstFloorButtonState = HIGH, lastSecondFloorButtonState = HIGH;
bool hasBeenCalledForFloor[3] = {0, 0, 0};

void setup() {
  Serial.begin(9600);
  
  // Button pins initialization
  pinMode(groundFloorButtonPin, INPUT_PULLUP);
  pinMode(firstFloorButtonPin, INPUT_PULLUP);
  pinMode(secondFloorButtonPin, INPUT_PULLUP);

  // LED pins initialization
  pinMode(statusLedPin, OUTPUT);
  pinMode(groundFloorLedPin, OUTPUT);
  pinMode(firstFloorLedPin, OUTPUT);
  pinMode(secondFloorLedPin, OUTPUT);

  // Buzzer pin initialization
  pinMode(buzzerPin, OUTPUT);

  // Initialize the decision queue and floor
  decisionQueue = new unsigned int[decisionQueueSize];
  decisionQueue[decisionQueueSize - 1] = 0;
}

void loop() {
  currentMillis = millis();
  
  debounceButton(groundFloorButtonPin, lastGroundFloorButtonState, lastGroundFloorButtonTime, currentMillis, groundFloorButtonStatus, 0);
  debounceButton(firstFloorButtonPin, lastFirstFloorButtonState, lastFirstFloorButtonTime, currentMillis, firstFloorButtonStatus, 1);
  debounceButton(secondFloorButtonPin, lastSecondFloorButtonState, lastSecondFloorButtonTime, currentMillis, secondFloorButtonStatus, 2);

  if (queueIndex != (decisionQueueSize - 1)) {
    if (!closingDoorTime && !liftStatus) {
      closingDoorTime = millis();
    }
    if (abs(millis() - closingDoorTime) < MAX_CLOSING_DOOR_TIME && !liftStatus) {
      Serial.println("Closing door");
      Serial.print("Current millis: ");
      Serial.print(millis());
      Serial.print(" closing door millis: ");
      Serial.println(closingDoorTime);
      tone(buzzerPin, NOTE_G6);
    }
    
    if (abs(millis() - closingDoorTime) >= MAX_CLOSING_DOOR_TIME && !liftStatus) {
      liftStatus = 1; // set to tranzit through floors
      transitTime = millis();
      noTone(buzzerPin);
    }

    if (liftStatus == 1) {
      tone(buzzerPin, NOTE_D6);
      if ((millis() - transitTime) % STATUS_LED_HIGH_TIME_INTERVAL == 0) {
        digitalWrite(statusLedPin, HIGH);
      } else if ((millis() - transitTime) % STATUS_LED_LOW_TIME_INTERVAL == 0) {
        digitalWrite(statusLedPin, LOW);
      }

      if ((millis() - transitTime) % TRANSIT_MOVE_LIFT == 0 && transitTime != millis()) {
        if (decisionQueue[queueIndex + 1] > currentFloor) {
          Serial.println("Going up");
          Serial.print("Current millis: ");
          Serial.print(millis());
          Serial.print(" transit time millis: ");
          Serial.println(transitTime);
          
          currentFloor += 1;
          updateFloorsLedStatus(currentFloor);
        } else if (decisionQueue[queueIndex + 1] < currentFloor) {
          Serial.println("Going down");
          Serial.print("Current millis: ");
          Serial.print(millis());
          Serial.print(" transit time millis: ");
          
          Serial.println(transitTime);
          currentFloor -= 1;
          updateFloorsLedStatus(currentFloor);
        } else {
          liftStatus = 2;
        }
      }
    }

    if (liftStatus == 2) {
      noTone(buzzerPin);
      liftStatus = closingDoorTime = 0;
      queueIndex++;
      hasBeenCalledForFloor[currentFloor] = false;
    }
  } else {
    digitalWrite(statusLedPin, HIGH);
    updateFloorsLedStatus(currentFloor);
  }
}

// Function to debounce a button
void debounceButton(int buttonPin, bool& lastButtonState, unsigned long& lastDebounceTime, unsigned long currentMillis, unsigned int& buttonStatus, unsigned int floorToAdd) {
  int reading = digitalRead(buttonPin);

  if (reading != lastButtonState) {
    lastDebounceTime = currentMillis;
  }

  if ((currentMillis - lastDebounceTime) > debounceDelay) {
    if (reading != buttonStatus) {
      buttonStatus = reading;

      if (buttonStatus == LOW) {
        Serial.print("Detected push for floor ");
        Serial.println(floorToAdd);
        if (currentFloor != floorToAdd && decisionQueue[decisionQueueSize - 1] != floorToAdd && !hasBeenCalledForFloor[floorToAdd]) {
          Serial.println("Adding it to the queue");
          if (oldDecisionQueue != NULL) {
            delete[] oldDecisionQueue;
          }
          oldDecisionQueue = new unsigned int[decisionQueueSize];
          for (unsigned int i = 0; i < decisionQueueSize; i++) {
            oldDecisionQueue[i] = decisionQueue[i];
          }
          if (decisionQueue != NULL) {
            delete[] decisionQueue;
          }
          decisionQueue = new unsigned int[++decisionQueueSize];
          for (unsigned int i = 0; i < decisionQueueSize - 1; i++) {
            decisionQueue[i] = oldDecisionQueue[i];
          }
          decisionQueue[decisionQueueSize - 1] = floorToAdd;
          hasBeenCalledForFloor[floorToAdd] = true;
        }
      }
    }
  }

  lastButtonState = reading;
}

void updateFloorsLedStatus(unsigned int floorNo) {
  switch (floorNo) {
      case 0: {
        digitalWrite(groundFloorLedPin, HIGH);
        digitalWrite(firstFloorLedPin, LOW);
        digitalWrite(secondFloorLedPin, LOW);
        break;
      }
      case 1: {
        digitalWrite(groundFloorLedPin, LOW);
        digitalWrite(firstFloorLedPin, HIGH);
        digitalWrite(secondFloorLedPin, LOW);
        break;
      }
      case 2: {
        digitalWrite(groundFloorLedPin, LOW);
        digitalWrite(firstFloorLedPin, LOW);
        digitalWrite(secondFloorLedPin, HIGH);
        break;
      }
      default: {
        break;
      }
    }

    segmentDisplay.updateDisplayWithFloorNumber(floorNo);
}
