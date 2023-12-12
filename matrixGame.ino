#include "LedControl.h"  
const int dinPin = 12;
const int clockPin = 11;
const int loadPin = 10;

const int pinSW = 2;
const int pinX = A0;
const int pinY = A1;

const int rowCount = 8;
const int colCount = 8;
const int displayBrightness = 4;
const int playerBlinkRate = 600;
const int bombBlinkRate = 200;
const int debounceDelay = 50;
const int explodingTime = 2500;
unsigned long lastDebounceTime = 0;
int lastButtonState = LOW;
int buttonState = LOW;

bool planted = false;

unsigned long plantBombTime = 0;

unsigned long lastPlayerBlink = 0;
bool blinkStatePlayer = true;

unsigned long lastBombBlink = 0;
bool blinkStateBomb = true;
int wallCount = 0;
int wallsBlown = 0;
unsigned long startGameTime;

bool matrix[colCount][rowCount] = { false };
LedControl lc = LedControl(dinPin, clockPin, loadPin, 1);  //DIN, CLK, LOAD, No. DRIVER

byte wPattern[rowCount][colCount] = {
  { 1, 0, 0, 0, 0, 0, 0, 1 },
  { 1, 0, 0, 0, 0, 0, 0, 1 },
  { 1, 0, 0, 0, 0, 0, 0, 1 },
  { 1, 0, 0, 0, 0, 0, 0, 1 },
  { 1, 0, 0, 1, 0, 0, 0, 1 },
  { 1, 0, 1, 0, 1, 0, 0, 1 },
  { 1, 1, 0, 0, 0, 1, 1, 1 },
  { 1, 0, 0, 0, 0, 0, 0, 1 }
};

struct position {
  int x, y;
};
position currentPositon;
position joystickPosition;
position bombPosition;

bool joyMoved = false;
const int minJoyThreshold = 300;
const int maxJoyThreshold = 700;

bool lost = false;
bool won = false;


void setup() {
  Serial.begin(9600);
  currentPositon.x = 0;
  currentPositon.y = 0;

  pinMode(pinSW, INPUT_PULLUP);
  pinMode(pinX, INPUT);
  pinMode(pinY, INPUT);

  randomSeed(analogRead(0));
  generateWalls();

  lc.shutdown(0, false);
  lc.setIntensity(0, displayBrightness);
  lc.clearDisplay(0);

  startGameTime = millis();
}

void loop() {
  Serial.print("You have blown ");
  Serial.print(wallsBlown);
  Serial.println(" walls");
  
  for (int i = 0; i < rowCount; i++)
    for (int j = 0; j < colCount; j++)
      if (i != currentPositon.x || j != currentPositon.y || lost)
        lc.setLed(0, i, j, matrix[i][j]);

  if (!lost && !won) {
    lc.setLed(0, currentPositon.x, currentPositon.y, getPlayerState());
    readButton();
    if (planted) {
      if (millis() - plantBombTime < explodingTime) {
        lc.setLed(0, bombPosition.x, bombPosition.y, getBombState());
      } else {
        explode();
      }
    }
    movePlayer();
  }
}

void movePlayer() {
  joystickPosition.x = analogRead(pinX);
  joystickPosition.y = analogRead(pinY);

  if (!joyMoved) {
    if (joystickPosition.y < minJoyThreshold && canMove("right"))
      currentPositon.y--;
    else if (joystickPosition.y > maxJoyThreshold && canMove("left"))
      currentPositon.y++;
    else if (joystickPosition.x < minJoyThreshold && canMove("up"))
      currentPositon.x--;
    else if (joystickPosition.x > maxJoyThreshold && canMove("down"))
      currentPositon.x++;

    joyMoved = true;
  }

  if (joystickPosition.y >= minJoyThreshold && joystickPosition.y <= maxJoyThreshold && joystickPosition.x >= minJoyThreshold && joystickPosition.x <= maxJoyThreshold)
    joyMoved = false;
}

bool canMove(const char* direction) {
  Serial.println(direction);
  if (direction == "right"){
    return !matrix[currentPositon.x][currentPositon.y - 1] && currentPositon.y > 0;
  }
  if (direction == "left")
    return !matrix[currentPositon.x][currentPositon.y + 1] && currentPositon.y < rowCount - 1;
  if (direction == "down"){
    if(currentPositon.x < rowCount - 1)
      return !matrix[currentPositon.x + 1][currentPositon.y];
  }
  if (direction == "up")
    return !matrix[currentPositon.x - 1][currentPositon.y] && currentPositon.x > 0 ;
  return false;
}

int getPlayerState() {
  if (millis() - lastPlayerBlink > playerBlinkRate) {
    blinkStatePlayer = !blinkStatePlayer;
    lastPlayerBlink = millis();
  }
  return blinkStatePlayer;
}

int getBombState() {
  if (millis() - lastBombBlink > bombBlinkRate) {
    blinkStateBomb = !blinkStateBomb;
    lastBombBlink = millis();
  }
  return blinkStateBomb;
}

void generateWalls() {
  for (int i = 0; i < colCount; i++) 
    for (int j = 0; j < rowCount; j++) {
      if(i - currentPositon.x < 2 && j - currentPositon.y < 2)
        matrix[i][j] = 0;
      else
        matrix[i][j] = (random(rowCount * colCount) < rowCount * colCount / 2);
      if(matrix[i][j])
        wallCount++;
    }
}

void readButton() {
  int reading = digitalRead(pinSW);
  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != buttonState) {
      buttonState = reading;
      if (buttonState == HIGH && !planted && startGameTime - millis() > 2000) {
        plantBomb();
      }
    }
  }
  lastButtonState = reading;
}

void plantBomb() {
  plantBombTime = millis();
  planted = true;
  bombPosition.x = currentPositon.x;
  bombPosition.y = currentPositon.y;
}

void explode() {
  if (matrix[bombPosition.x + 1][bombPosition.y]) {
    matrix[bombPosition.x + 1][bombPosition.y] = 0;
    wallsBlown++;
  }
  if (matrix[bombPosition.x - 1][bombPosition.y]) {
    matrix[bombPosition.x - 1][bombPosition.y] = 0;
    wallsBlown++;
  }
  if (matrix[bombPosition.x][bombPosition.y + 1]) {
    matrix[bombPosition.x][bombPosition.y + 1] = 0;
    wallsBlown++;
  }
  if (matrix[bombPosition.x][bombPosition.y - 1]) {
    matrix[bombPosition.x][bombPosition.y - 1] = 0;
    wallsBlown++;
  }
  if (abs(currentPositon.x - bombPosition.x) < 2 && abs(currentPositon.y - bombPosition.y) < 2) {
    displayLoseAnimation();
    lost = true;
  }
  if (wallsBlown == wallCount) {
    won = true;
    displayWinAnimation();
  }
  planted = false;
}

void displayLoseAnimation() {
  for (int i = 0; i < colCount; i++) {
    for (int j = 0; j < rowCount; j++) {
      if (j == rowCount - 1 || i == rowCount - 2)
        matrix[i][j] = 1;
      else matrix[i][j] = 0;
    }
  }
}

void displayWinAnimation() {
  for (int i = 0; i < colCount; i++) {
    for (int j = 0; j < rowCount; j++) {
      matrix[i][j] = wPattern[i][j];
    }
  }
}
