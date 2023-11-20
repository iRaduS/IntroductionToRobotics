#include <EEPROM.h>

int menuItemSelect = 0;
int menuSubItemSelect = 0;
const int trigPin = 9;
const int echoPin = 10;
const int lightSensorPin = A0;
const int redPin = 5;
const int greenPin = 6;
const int bluePin = 11;
long duration = 0;
int distance = 0;
int lightSensor = 0;
bool automaticLedToggle = false;
bool isWaitingInput = false;
String inputString = "";
int redValue = 0;
int greenValue = 0;
int blueValue = 0;
byte stateUltrasonic = HIGH;
unsigned long timeUltrasonic = 0;
const unsigned long limitUltrasonic = 100;
long samplingInterval = 1;
long ultrasonicMinValue = 9999;
long lightMinValue = 9999;
bool selected = false;
const int szLastValue = 10;
int lastValuesSensor[szLastValue];
int valueSensors[szLastValue] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
unsigned long lastTimeSampling = 0;
unsigned long valueSensorIndex = 0;

void setup() {
  Serial.begin(9600);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(lightSensorPin, INPUT);
  pinMode(redPin, OUTPUT);
  pinMode(bluePin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  digitalWrite(trigPin, LOW);
  showMainMenu();
}

void loop() {
  lightSensor = analogRead(lightSensorPin);
  if ((millis() - timeUltrasonic) >= limitUltrasonic) {
    if (stateUltrasonic == HIGH) {
      digitalWrite(trigPin, LOW);
    }
    stateUltrasonic = !stateUltrasonic;
    timeUltrasonic = millis();
  }
  digitalWrite(trigPin, stateUltrasonic);
  if (stateUltrasonic == LOW) {
    duration = pulseIn(echoPin, HIGH);
    if (duration) {
      distance = duration * 0.034 / 2;
    }
  }

  if (selected && Serial.available() > 0) {
    Serial.read();
    selected = false;
  }

  if (!menuItemSelect && Serial.available() > 0) {
    menuItemSelect = ((char) Serial.read()) - '0';
    selected = true;
    showSubMenu(menuItemSelect);
  } else if (!menuSubItemSelect && menuItemSelect && Serial.available() > 0 && !inputString.length()) {
    menuSubItemSelect = ((char) Serial.read()) - '0';
    selected = true;
  }

  if (!selected) {
    actionMenu(menuItemSelect, menuSubItemSelect);
  }

  if (automaticLedToggle && (distance < ultrasonicMinValue || lightSensor < lightMinValue)) {
    analogWrite(redPin, 0);
    analogWrite(bluePin, 0);
    analogWrite(greenPin, 0);
    digitalWrite(bluePin, LOW);
    digitalWrite(redPin, HIGH);
    digitalWrite(greenPin, LOW);
  } else if (automaticLedToggle && !(distance < ultrasonicMinValue || lightSensor < lightMinValue)) {
    analogWrite(redPin, 0);
    analogWrite(bluePin, 0);
    analogWrite(greenPin, 0);
    digitalWrite(bluePin, LOW);
    digitalWrite(redPin, LOW);
    digitalWrite(greenPin, HIGH);
  }
}

void actionMenu(int &itemSelect, int &subItemSelect) {
  if (!itemSelect || !subItemSelect) {
    return;
  }

  switch (itemSelect) {
    case 1: {
        switch (subItemSelect) {
          case 1: {
              if (Serial.available() <= 0  && !inputString.length()) {
                Serial.println("Please enter a number between 1 to 10");
              }
              while (Serial.available() <= 0 && !inputString.length());
              if (Serial.available() > 0) {
                inputString += ((char) Serial.read());
              } else if (Serial.available() <= 0 && inputString.length()) {
                long selection = inputString.toInt();
                if (!selection || selection > 10 || selection < 0) {
                  Serial.println("Invalid value (1-10)");
                } else {
                  samplingInterval = selection;
                }
                Serial.print("Set sampling interval to ");
                Serial.println(samplingInterval);

                inputString = "";
                menuSubItemSelect = 0;
                showSubMenu(menuItemSelect);
              }

              break;
            }
          case 2: {
              if (Serial.available() <= 0  && !inputString.length()) {
                Serial.println("Please enter a number between 1 to 300");
              }
              while (Serial.available() <= 0 && !inputString.length());
              if (Serial.available() > 0) {
                inputString += ((char) Serial.read());
              } else if (Serial.available() <= 0 && inputString.length()) {
                long selection = inputString.toInt();
                if (!selection || selection < 0 || selection > 300) {
                  Serial.println("Invalid value (1-300)");
                } else {
                  ultrasonicMinValue = selection;
                }
                Serial.print("Set ultrasonic threshold value to ");
                Serial.println(ultrasonicMinValue);

                inputString = "";
                menuSubItemSelect = 0;
                showSubMenu(menuItemSelect);
              }

              break;
            }
          case 3: {
              if (Serial.available() <= 0 && !inputString.length()) {
                Serial.println("Please enter a number between 1 to 1023");
              }
              while (Serial.available() <= 0 && !inputString.length());
              if (Serial.available() > 0) {
                inputString += ((char) Serial.read());
              } else if (Serial.available() <= 0 && inputString.length()) {
                long selection = inputString.toInt();
                if (!selection || selection < 0 || selection > 1023) {
                  Serial.println("Invalid value (1-1023)");
                } else {
                  lightMinValue = selection;
                }
                Serial.print("Set light sensor threshold value to ");
                Serial.println(lightMinValue);

                inputString = "";
                menuSubItemSelect = 0;
                showSubMenu(menuItemSelect);
              }
              break;
            }
          case 4: {
              menuItemSelect = menuSubItemSelect = 0;
              showMainMenu();

              break;
            }
          default: {
              Serial.println("Invalid subitem selection.");
              menuSubItemSelect = 0;
              showSubMenu(menuItemSelect);
              break;
            }
        }
        break;
      }
    case 2: {
        switch (subItemSelect) {
          case 1: {
              for (int i = 0; i < (szLastValue) * 16; i++) {
                EEPROM.put(i, 0);
              }
              menuItemSelect = menuSubItemSelect = 0;
              showMainMenu();

              break;
            }
          case 2: {
              menuItemSelect = menuSubItemSelect = 0;
              showMainMenu();

              break;
            }
          default: {
              Serial.println("Invalid subitem selection.");
              menuSubItemSelect = 0;
              showSubMenu(menuItemSelect);
              break;
            }
        }
        break;
      }
    case 3: {
        switch (subItemSelect) {
          case 1: {
              if (Serial.available() > 0) {
                inputString += ((char) Serial.read());
              } else if (Serial.available() <= 0 && inputString.length() && inputString.charAt(0) == 'q') {
                inputString = "";
                menuSubItemSelect = 0;
                showSubMenu(menuItemSelect);
              }
              
              if ((millis() - lastTimeSampling) >= samplingInterval * 1000) {
                Serial.print("Ultrasonic distance reading: ");
                Serial.println(distance);

                Serial.print("Light reading: ");
                Serial.println(lightSensor);

                valueSensors[valueSensorIndex++] = distance;
                valueSensors[valueSensorIndex++] = lightSensor;
                valueSensorIndex %= 10;

                lastTimeSampling = millis();
              }
              break;
            }
          case 2: {
              Serial.print("Sampling interval to ");
              Serial.println(samplingInterval);
                
              Serial.print("Light sensor threshold value to ");
              Serial.println(lightMinValue);

              Serial.print("Ultrasonic threshold value to ");
              Serial.println(ultrasonicMinValue);
            
              menuSubItemSelect = 0;
              showSubMenu(menuItemSelect);
              break;
            }
          case 3: {
              Serial.println("Here are the last 10 sensor readings");
              for (int i = 0; i < szLastValue; i++) {
                EEPROM.put(i * 16, valueSensors[i]);
              }
              for (int i = 0; i < szLastValue; i++) {
                EEPROM.get(i * 16, lastValuesSensor[i]);
                Serial.println(lastValuesSensor[i]);
              }
              Serial.println();
              menuSubItemSelect = 0;
              showSubMenu(menuItemSelect);
              break;
            }
          case 4: {
              menuItemSelect = menuSubItemSelect = 0;
              showMainMenu();

              break;
            }
          default: {
              Serial.println("Invalid subitem selection");
              menuSubItemSelect = 0;
              showSubMenu(menuItemSelect);
              break;
            }
        }
        break;
      }
    case 4: {
        switch (subItemSelect) {
          case 1: {
              if (Serial.available() <= 0  && !inputString.length()) {
                Serial.println("Please enter the format (0-255) (0-255) (0-255) (for R G B) i.e. 245 123 0: ");
              }
              while (Serial.available() <= 0 && !inputString.length());

              if (Serial.available() > 0) {
                inputString += ((char) Serial.read());
              } else if (Serial.available() <= 0 && inputString.length()) {
                if (isValidFormat(inputString)) {
                  analogWrite(redPin, redValue);
                  analogWrite(bluePin, blueValue);
                  analogWrite(greenPin, greenValue);
                } else {
                  Serial.println("Invalid format of the values!");
                }

                inputString = "";
                menuSubItemSelect = 0;
                showSubMenu(menuItemSelect);
              }

              break;
            }
          case 2: {
              automaticLedToggle = !automaticLedToggle;
              menuSubItemSelect = 0;

              if (automaticLedToggle) {
                analogWrite(redPin, 0);
                analogWrite(bluePin, 0);
                analogWrite(greenPin, 0);
                digitalWrite(bluePin, LOW);
                digitalWrite(redPin, LOW);
                digitalWrite(greenPin, HIGH);
              } else {
                digitalWrite(bluePin, LOW);
                digitalWrite(redPin, LOW);
                digitalWrite(greenPin, LOW);
                analogWrite(redPin, redValue);
                analogWrite(bluePin, blueValue);
                analogWrite(greenPin, greenValue);
              }

              Serial.print("Toogle automatic mode ");
              Serial.println(automaticLedToggle ? "ON" : "OFF");
              Serial.println();
              showSubMenu(menuItemSelect);

              break;
            }
          case 3: {
              menuItemSelect = menuSubItemSelect = 0;
              showMainMenu();

              break;
            }
          default: {
              Serial.println("Invalid subitem selection.");
              menuSubItemSelect = 0;
              showSubMenu(menuItemSelect);
              break;
            }
        }
        break;
      }
    default: {
        break;
      }
  }
}

bool isValidFormat(String input) {
  return sscanf(input.c_str(), "%d %d %d", &redValue, &greenValue, &blueValue) == 3 &&
         redValue >= 0 && redValue <= 255 &&
         greenValue >= 0 && greenValue <= 255 &&
         blueValue >= 0 && blueValue <= 255;
}

void showMainMenu() {
  Serial.println("Main menu");
  Serial.println("1. SENSOR SETINGS");
  Serial.println("2. RESET LOGGER DATA");
  Serial.println("3. SYSTEM STATUS");
  Serial.println("4. RGB LED CONTROL");
}

void showSubMenu(int currentItemSelect) {
  switch (currentItemSelect) {
    case 1: {
        Serial.println("\t1. Sensors sampling interval");
        Serial.println("\t2. Ultrasonic alert threshold");
        Serial.println("\t3. LDR alert threshold");
        Serial.println("\t4. Back");
        break;
      }
    case 2: {
        Serial.println("Are you sure?");
        Serial.println("\t1. Yes");
        Serial.println("\t2. No");
        break;
      }
    case 3: {
        Serial.println("\t1. Current sensor readings");
        Serial.println("\t\t(press q to exit this mode if selected)");
        Serial.println("\t2. Current sensor settings");
        Serial.println("\t3. Display logged data");
        Serial.println("\t4. Back");
        break;
      }
    case 4: {
        Serial.println("\t1. Manual color control");
        Serial.println("\t2. LED: toggle automatic ON/OFF");
        Serial.println("\t3. Back");
        break;
      }
    default: {
        Serial.println("\tInvalid selection!");
        break;
      }
  }
}
