#include "LedControl.h" // Include the LedControl library for controlling LED matrices
// Pin definitions for the LED matrix
const int dinPin = 12;
const int clockPin = 11;
const int loadPin = 10;
// Pin definitions for the joystick
const int xPin = A0;
const int yPin = A1;
const int swPin = 4;
// Create an LedControl object to interface with the LED matrix
LedControl lc = LedControl(dinPin, clockPin, loadPin, 1); // DIN, CLK, LOAD, number of devices
// Variables to control LED matrix brightness and position
byte matrixBrightness = 2;
byte xPos = 4;
byte yPos = 4;
byte xLastPos = 0;
byte yLastPos = 0;
// Thresholds for joystick movement detection
const int minThreshold = 200;
const int maxThreshold = 600;
// Timing variables for movement updates
const byte moveInterval = 100;
unsigned long lastMoved = 0;
// Size of the LED matrix (8x8)
const byte matrixSize = 8;
bool matrixChanged = true;
// 2D array representing the state (on/off) of each LED in the matrix
byte matrix[matrixSize][matrixSize] = { /* Initial state with all LEDs off */ };
byte walls[matrixSize][matrixSize] = { /* Initial state with all walls off */ };
byte bombs[matrixSize][matrixSize] = { /* Initial state with all bombs off */ };
// Player blink differently, interval of 500ms
byte playerBlinkStatus = 1;
unsigned long lastPlayerBlinkingTime = 0;
int playerBlinkingInterval = 750;
// Bomb blink differently, interval of 100ms
byte bombBlinkStatus = 1;
unsigned long lastBombBlinkingTime = 0;
int bombBlinkingInterval = 50;
// Bomb LED pin and button
const int bombStatusLedPin = 8;
const int fireButtonPin = 3;
byte lastSwButtonState = HIGH, swButtonState = HIGH;
unsigned long lastSwDebounceTime = 0;
byte lastButtonState = HIGH, buttonState = HIGH;
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50;
byte hasBombOnPlayer = 0;
void setup() {
  Serial.begin(9600); // Start serial communication for debugging
  // Initialize the LED matrix
  lc.shutdown(0, false); // Disable power saving, turn on the display
  lc.setIntensity(0, matrixBrightness); // Set the brightness level
  lc.clearDisplay(0); // Clear the display initially
  matrix[xPos][yPos] = 1; // Turn on the initial LED position

  // Set pin mode to bomb status and others
  pinMode(bombStatusLedPin, OUTPUT);
  pinMode(swPin, INPUT_PULLUP);
  pinMode(fireButtonPin, INPUT_PULLUP);

  // Draw bounding walls on the map
  for (int i = 0; i < matrixSize; i++) {
    matrix[0][i] = matrix[i][0] = matrix[matrixSize - 1][i] = matrix[i][matrixSize - 1] = 1;
    walls[0][i] = walls[i][0] = walls[matrixSize - 1][i] = walls[i][matrixSize - 1] = 1;
  }

  // Generate random walls on the map
  for (int i = 1; i < matrixSize - 1; i++) {
    for (int j = 1; j < matrixSize - 1; j++) {
      if (i == xPos && j == yPos) {
        continue;
      }

      walls[i][j] = random(3) == 0;
      matrix[i][j] = walls[i][j];
    }
  }

  // Generate bombs on the map
  for (int i = 0; i < matrixSize; i++) {
    for (int j = 0; j < matrixSize; j++) {
      bombs[i][j] = walls[i][j] || (i == xPos && j == yPos) ? 0 : (random(5) == 0);
    }
  }
}
// Main loop, runs continuously
void loop() {
  // Debounce joystick switch button
  debounceSwitchButton();

  // Debounce fire button
  debounceButton();

  // Blinking player mechanism
  if (millis() - lastPlayerBlinkingTime > playerBlinkingInterval) {
    playerBlinkStatus = !playerBlinkStatus;
    lastPlayerBlinkingTime = millis();
  }
  matrix[xPos][yPos] = playerBlinkStatus;
  lc.setLed(0, xPos, yPos, matrix[xPos][yPos]);

  // Blinking bomb mechanism
  if (millis() - lastBombBlinkingTime > bombBlinkingInterval) {
    bombBlinkStatus = !bombBlinkStatus;
    lastBombBlinkingTime = millis();
  }
  for (int i = 0; i < matrixSize; i++) {
    for (int j = 0; j < matrixSize; j++) {
      if (bombs[i][j] && !(xPos == i && yPos == j)) {
        matrix[i][j] = bombBlinkStatus;
        lc.setLed(0, i, j, matrix[i][j]);
      }
    }
  }

  // Check if it's time to move the LED
  if (millis() - lastMoved > moveInterval) {
    updatePositions(); // Update the LED position based on joystick input
    lastMoved = millis(); // Reset the movement timer
  }
  // Update the LED matrix display if there's been a change
  if (matrixChanged) {
    updateMatrix();
    matrixChanged = false;
  }
}
// Function to update the LED matrix display
void updateMatrix() {
  for (int row = 0; row < matrixSize; row++) {
    for (int col = 0; col < matrixSize; col++) {
      lc.setLed(0, row, col, matrix[row][col]); // Update each LED state
    }
  }
}
// Function to read joystick input and update LED positions accordingly
void updatePositions() {
  int xValue = analogRead(xPin); // Read the X-axis value
  int yValue = analogRead(yPin); // Read the Y-axis value
  // Store the last positions
  xLastPos = xPos;
  yLastPos = yPos;
  // Update xPos based on joystick movement
  if (xValue < minThreshold) {
    xPos = (xPos + 1) % matrixSize;

    if (walls[xPos][yPos]) {
      xPos = xLastPos;
    }
  } else if (xValue > maxThreshold) {
    xPos = (xPos > 0) ? xPos - 1 : matrixSize - 1;

    if (walls[xPos][yPos]) {
      xPos = xLastPos;
    }
  }
  // Update yPos based on joystick movement
  if (yValue < minThreshold) {
    yPos = (yPos > 0) ? yPos - 1 : matrixSize - 1;

    if (walls[xPos][yPos]) {
      yPos = yLastPos;
    }
  } else if (yValue > maxThreshold) {
    yPos = (yPos + 1) % matrixSize;

    if (walls[xPos][yPos]) {
      yPos = yLastPos;
    }
  }
  // Check if the position has changed and update the matrix accordingly
  if (xPos != xLastPos || yPos != yLastPos) {
    matrixChanged = true;
    matrix[xLastPos][yLastPos] = 0; // Turn off the LED at the last position
    matrix[xPos][yPos] = playerBlinkStatus; // Turn on the LED at the new position
  }
}

void debounceSwitchButton() {
  int reading = digitalRead(swPin);

  if (reading != lastSwButtonState) {
    lastSwDebounceTime = millis();
  }

  if ((millis() - lastSwDebounceTime) > debounceDelay) {
    if (reading != swButtonState) {
      swButtonState = reading;
      if (swButtonState == LOW && !hasBombOnPlayer) {
        hasBombOnPlayer = bombs[xPos][yPos];
        bombs[xPos][yPos] = 0;
      } else if (swButtonState == LOW && hasBombOnPlayer && !bombs[xPos][yPos]) {
        hasBombOnPlayer = 0;
        bombs[xPos][yPos] = 1;
      }

      digitalWrite(bombStatusLedPin, hasBombOnPlayer);
    }
  }
  lastSwButtonState = reading;
}

void debounceButton() {
  int reading = digitalRead(fireButtonPin);

  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != buttonState) {
      swButtonState = reading;
      if (swButtonState == LOW && hasBombOnPlayer) {
        hasBombOnPlayer = 0;

        for (int i = 0; i < matrixSize; i++) {
          walls[xPos][i] = walls[i][yPos] = 0;
          matrix[xPos][i] = matrix[i][yPos] = 0;
        }
        matrixChanged = true;
      }

      digitalWrite(bombStatusLedPin, hasBombOnPlayer);
    }
  }
  lastButtonState = reading;
}
