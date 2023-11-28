// Bomberman game on 8x8 matrix arduino
// The player moves around the matrix and has to destroy walls using bombs. The purpose is to clean the map ASAP
#include "LedControl.h"
// MAX7219 pins
const int dinPin = 12;
const int clockPin = 11;
const int loadPin = 10;
LedControl lc = LedControl(dinPin, clockPin, loadPin, 1);  

// joystick pins
const int pinSW = 2;
const int pinX = A0;
const int pinY = A1;

const int rowCount = 8;
const int colCount = 8;
const int displayBrightness = 4; // matrix LED brightness
const int playerBlinkRate = 600; 
const int bombBlinkRate = 200;
const int debounceDelay = 50; 
const int explodingTime = 2500; // the time the player has to get out of the exploding zone

// variables to get the right output of the sw button (debounce)
unsigned long lastDebounceTime = 0;
int lastSwState = LOW;
int swState = LOW;

bool planted = false; // indicates the bomb has been planted
unsigned long plantBombTime = 0; // the starting time of planting

unsigned long lastPlayerBlink = 0; // to know when to blink
bool blinkStatePlayer = true;

unsigned long lastBombBlink = 0;
bool blinkStateBomb = true; // indicates the state of the bomb LED
const int explodingZone = 2; // all the blocks in a radius of 2 will explode 

int wallCount = 0; // how many walls were generated
int wallsBlown = 0; // how many walls the player has blown
unsigned long startGameTime; 

bool joyMoved = false;
// the threshold on which the joystick changes values on the display
const int minJoyThreshold = 300;
const int maxJoyThreshold = 700;

// they change value when the game has come to an end
bool lost = false;
bool won = false;
bool firstBlow = false;

// the map of the game
bool matrix[colCount][rowCount] = { false };

// to display when the game is over and won
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

const position initialPosition = { 0, 0 };
position currentPosition; // position of the player
position joystickPosition; // joystick indexes
position bombPosition; // where the bomb got planted at the moment


void setup() {
  Serial.begin(9600);
  currentPosition.x = initialPosition.x;
  currentPosition.y = initialPosition.y;

  pinMode(pinSW, INPUT_PULLUP);
  pinMode(pinX, INPUT);
  pinMode(pinY, INPUT);

  randomSeed(analogRead(0)); // seed for the random() function 
  generateWalls(); // the walls are generated only at setup

  lc.shutdown(0, false); // turn off power saving and enable display
  lc.setIntensity(0, displayBrightness); // set brightness
  lc.clearDisplay(0);

  startGameTime = millis();
}

void loop() {
  Serial.print(currentPosition.x);
  Serial.print("  ");
  Serial.println(currentPosition.y);

  for (int row = 0; row < rowCount; row++)
    for (int column = 0; column < colCount; column++)
      if (row != currentPosition.x || column != currentPosition.y || lost) // to make the blink look "smooth"
        lc.setLed(0, row, column, matrix[row][column]); // show the current state of the map

  // the game logic is valid only when the game didn't end
  if (!lost && !won) {
    lc.setLed(0, currentPosition.x, currentPosition.y, getPlayerState());
    checkForPlant(); // get the input of the sw button, and plant the bomb when it is pressed
    if (planted) { // if the bomb is planted then blink
      if (millis() - plantBombTime < explodingTime) {
        lc.setLed(0, bombPosition.x, bombPosition.y, getBombState());
      } else {
        explode(); // if the time passed, it explodes
      }
    }
    movePlayer(); // read the joystick position and move around
  }
}

void movePlayer() {
  joystickPosition.x = analogRead(pinX);
  joystickPosition.y = analogRead(pinY);

  if (!joyMoved) {
    // the joystick is moved accordingly and we dont have wall and we are in the map
    if (joystickPosition.y < minJoyThreshold && canMove("right"))
      currentPosition.y--;
    else if (joystickPosition.y > maxJoyThreshold && canMove("left"))
      currentPosition.y++;
    else if (joystickPosition.x < minJoyThreshold && canMove("up"))
      currentPosition.x--;
    else if (joystickPosition.x > maxJoyThreshold && canMove("down"))
      currentPosition.x++;

    joyMoved = true;
  }

  if (joystickPosition.y >= minJoyThreshold && joystickPosition.y <= maxJoyThreshold && joystickPosition.x >= minJoyThreshold && joystickPosition.x <= maxJoyThreshold)
    joyMoved = false; // to get the joystick position only once
}

bool canMove(const char* direction) {
  if (direction == "right") {
    // no wall and in the map
    return !matrix[currentPosition.x][currentPosition.y - 1] && currentPosition.y > 0;
  }
  if (direction == "left")
    return !matrix[currentPosition.x][currentPosition.y + 1] && currentPosition.y < rowCount - 1;
  if (direction == "down") {
    if (currentPosition.x < rowCount - 1)
      return !matrix[currentPosition.x + 1][currentPosition.y];
  }
  if (direction == "up")
    return !matrix[currentPosition.x - 1][currentPosition.y] && currentPosition.x > 0;
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
  for (int row = 0; row < colCount; row++)
    for (int column = 0; column < rowCount; column++) {
      if (row - currentPosition.x < explodingZone && column - currentPosition.y < explodingZone)
        matrix[row][column] = 0;
      else
        matrix[row][column] = (random(rowCount * colCount) < rowCount * colCount / 2); // minimum 50% of map is walls
      if (matrix[row][column])
        wallCount++;
    }
}

void checkForPlant() {
  int reading = digitalRead(pinSW);
  if (reading != lastSwState) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != swState) {
      swState = reading;
      if (swState == HIGH && !planted && startGameTime - millis() > 2000) {
        if(firstBlow) // to skip the inital impulse of the SW 
          plantBomb();
        else firstBlow = true;
      }
    }
  }
  lastSwState = reading;
}

void plantBomb() {
  plantBombTime = millis();
  planted = true;
  bombPosition.x = currentPosition.x;
  bombPosition.y = currentPosition.y;
}

void explode() {
  destroyWall(bombPosition.x + 1, bombPosition.y);
  destroyWall(bombPosition.x - 1, bombPosition.y);
  destroyWall(bombPosition.x, bombPosition.y + 1);
  destroyWall(bombPosition.x, bombPosition.y - 1);
  
  // check if the player is in the dangerous zone
  if (abs(currentPosition.x - bombPosition.x) < explodingZone && abs(currentPosition.y - bombPosition.y) < explodingZone) {
    displayLoseAnimation();
    lost = true;
  } else if (wallsBlown == wallCount) {
    won = true;
    displayWinAnimation();
  }
  planted = false; // after a bomb has exploded, we can plant again
}

void destroyWall(int x, int y) {
  if (matrix[x][y]) {
    matrix[x][y] = 0;
    wallsBlown++; // count to know when the game ends
  }
}


void displayLoseAnimation() {
  for (int row = 0; row < colCount; row++) {
    for (int column = 0; column < rowCount; column++) {
      if (column == rowCount - 1 || row == rowCount - 2)
        matrix[row][column] = 1;
      else matrix[row][column] = 0;
    }
  }
}

void displayWinAnimation() {
  for (int row = 0; row < colCount; row++) {
    for (int column = 0; column < rowCount; column++) {
      matrix[row][column] = wPattern[row][column];
    }
  }
  Serial.print("Your score is: ");
  Serial.println(startGameTime - millis());
}
