// Bomberman game on 8x8 matrix arduino
// The player moves around the matrix and has to destroy walls using bombs. The purpose is to clean the map ASAP

#include "LedControl.h"
#include <LiquidCrystal.h>
#include <EEPROM.h>

// LCD pins
const byte rs = 9;
const byte en = 8;
const byte d4 = 7;
const byte d5 = 6;
const byte d6 = 5;
const byte d7 = 4;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

// MAX7219 pins
const int dinPin = 12;
const int clockPin = 11;
const int loadPin = 10;
LedControl lc = LedControl(dinPin, clockPin, loadPin, 1);

// SW pins
const int pinSW = 2;
const int pinX = A0;
const int pinY = A1;
const int brightnessPin = A2; // LCD brightness pin

const int greetingTime = 1000; // the greeting message is displayed this period
const int scrollingSpeed = 500; // each 0.5s the text that is too large refreshes
unsigned long lastScroll = 0;
int currentChar = 0;

String menuOptions[] = { "Start game", "Settings", "About" }; // move through those
int menuCount = 3;
String aboutInfo = "Creator: Stefan Barbu  Github: stefanbrb10; @2023unibuc  press SW to go back";
String settingsOptions[] = { "Matrix brightness", "LCD brightness", "Back" };
int settingsCount = 3;

int currentMenuOption = 0; // first layer
int currentSubmenuOption = 0; // second layer
// variables to indicate the level we're on
bool inSubSubMenu = false;
bool inMainMenu = true;
bool inSubMenu = false;

bool firstImpulse = true; // to skip the first unnecesary "press" on SW

// values to save to EEPROM and their intervals
int matrixBrightness;
const int matrixBrightnessAddress = 0;
const int lcdBrightnessAddress = 1;
int lcdBrightness;
const int minBrightness = 0;
const int maxBrightness = 16;

// for debouncing
unsigned long lastDebounceTime = 0;
int lastSwState = LOW;
int swState = LOW;
const int debounceDelay = 50;

const int playerBlinkRate = 600;
const int bombBlinkRate = 200;
const int explodingTime = 2500;  // the time the player has to get out of the exploding zone
unsigned long startGameTime;


byte doodleCharacter[8] = {
  0b01110,
  0b01001,
  0b01011,
  0b11010,
  0b10001,
  0b11111,
  0b10101,
  0b10101
};

const int lcdCols = 16; // to divide the LCD 
const int lcdRows = 2;

struct Position {
  int x, y;
};

const Position initialPosition = { 0, 0 }; // initial position of the player
Position currentPosition; 
Position joystickPosition; // raw joystick positiom
Position bombPosition;
bool joyMoved = false;
const int minJoyThreshold = 300;
const int maxJoyThreshold = 700;

const int matrixSize = 8;
byte matrix[matrixSize][matrixSize] = { false };

byte wPattern[matrixSize][matrixSize] = {
  { 1, 0, 0, 0, 0, 0, 0, 1 },
  { 1, 0, 0, 0, 0, 0, 0, 1 },
  { 1, 0, 0, 0, 0, 0, 0, 1 },
  { 1, 0, 0, 0, 0, 0, 0, 1 },
  { 1, 0, 0, 1, 0, 0, 0, 1 },
  { 1, 0, 1, 0, 1, 0, 0, 1 },
  { 1, 1, 0, 0, 0, 1, 1, 1 },
  { 1, 0, 0, 0, 0, 0, 0, 1 }
};

int wallCount = 0;   // how many walls were generated
int wallsBlown = 0;  // how many walls the player has blown

// they change when the game has come to an end
bool lost = false;
bool won = false;

bool toStartGame = false; // to know that the player selected the option "start game"
bool inGame = false;

bool planted = false;             // indicates the bomb has been planted
unsigned long plantBombTime = 0;  // the starting time of planting

unsigned long lastPlayerBlink = 0;  // to know when to blink
bool blinkStatePlayer = true;

unsigned long lastBombBlink = 0;
bool blinkStateBomb = true;   // indicates the state of the bomb LED
const int explodingZone = 2;  // all the blocks in a radius of 2 will explode

unsigned long lastTimeShow = 0;
const int showRate = 1;
bool playing = false;

const int levelProbability[] = {14, 32}; // the probability that a wall is on a cell
const int levelCount = 2;
int currentLevel = 1;


void setup() {
  Serial.begin(9600);
  pinMode(pinSW, INPUT_PULLUP);
  pinMode(pinX, INPUT);
  pinMode(pinY, INPUT);
  pinMode(brightnessPin, OUTPUT);
  lcdBrightness = EEPROM.read(lcdBrightnessAddress); // use the EEPROM vlue
  analogWrite(brightnessPin, lcdBrightness);
  randomSeed(analogRead(0));
  lcd.begin(lcdCols, lcdRows);

  lc.shutdown(0, false);
  matrixBrightness = EEPROM.read(matrixBrightnessAddress);
  lc.setIntensity(0, matrixBrightness); 
  lc.clearDisplay(0);

  displayGreetingMessage();
}

void loop() {
  if (inMainMenu)
    displayMenu();
  if (!inMainMenu && inSubMenu && !toStartGame) {
    handleMenuOption(); // act accordingly to what the player has chosen through joystick
  }
  if (inSubSubMenu) {
    handleSubMenuOption();
  }
  readSw();
  if (toStartGame) {
    startGame(); // start the game (init)
  }
  if (inGame) {
    playGame();
  }
}

void displayDoodleCharacters() {
  lcd.createChar(0, doodleCharacter);
  lcd.setCursor(0, 0);
  lcd.write((uint8_t)0);
  lcd.setCursor(lcdCols - 1, 0);
  lcd.write((uint8_t)0);
}

void handleMenuOption() {
  if (currentMenuOption == 0) {
    toStartGame = true;
    inMainMenu = false;
    inSubMenu = false;
  }
  if (currentMenuOption == 1) {
    move(currentSubmenuOption, settingsOptions, settingsCount, "Settings"); // scroll though the options
  }
  if (currentMenuOption == 2)
    displayAbout();
}

void handleSubMenuOption() {
  if (currentMenuOption == 1){
    if(currentSubmenuOption == 0)
     readNumber("Matrix brightness", matrixBrightness); // get the user values
     else if (currentSubmenuOption == 1)
      readNumber("LCD brightness", lcdBrightness);
  }
}

void displayCenteredText(String text, int line) {
  lcd.setCursor(0, line);
  int textLen = text.length();
  int centerIndex = (lcdCols - textLen) / 2; // center of lcd
  for (int i = 0; i < centerIndex; i++)
    lcd.print(' '); // the actual text is surrounded by whitespace
  lcd.print(text);
  for (int i = 0; i < centerIndex; i++)
    lcd.print(' ');
}

void displayGreetingMessage() {
  displayCenteredText("Welcome to", 0);
  displayCenteredText("Bomberman", 1);
  displayDoodleCharacters();
  delay(greetingTime);
  lcd.clear();
}

void displayMenu() {
  move(currentMenuOption, menuOptions, menuCount, "Main menu"); // scroll through menu options
}

void scrollText(String text, int line) {
  // show the scrolling text if it is bigger than the lcd
  if (millis() - lastScroll > scrollingSpeed) {
    lcd.clear();
    lcd.setCursor(0, line);
    lcd.print(text.substring(currentChar, currentChar + lcdCols));
    currentChar++;
    if (currentChar >= text.length()) {
      currentChar = 0;
    }
    lastScroll = millis();
  }
}

void move(int &option, String optionList[], int len, String title) {
  displayCenteredText(title, 0);
  joystickPosition.y = analogRead(pinY);
  displayCenteredText(optionList[option], 1); // option

  if (!joyMoved) {
    if (joystickPosition.y < minJoyThreshold && option < len - 1)
      option++;
    else if (joystickPosition.y > maxJoyThreshold && option > 0)
      option--;
    joyMoved = true;
  }

  if (joystickPosition.y >= minJoyThreshold && joystickPosition.y <= maxJoyThreshold)
    joyMoved = false;
}

void readSw() {
  int reading = digitalRead(pinSW);
  if (reading != lastSwState) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != swState) {
      swState = reading;
      if (swState == HIGH) {
        if (firstImpulse)
          firstImpulse = false; // ignore first impulse
        else if (inMainMenu) {
          lcd.clear();
          inMainMenu = false;
          inSubMenu = true;
        } else if (inSubMenu) {
          handleSubMenuLogic();
        } 
        else if (inSubSubMenu) {
          handleSubSubMenuLogic(); // for now, save values to eeprom
        }
        if (inGame && (lost || won)) {
          exitGame(); // resfresh game
        }
      }
    }
  }
  lastSwState = reading;
}

void handleSubMenuLogic() {
  if ((currentMenuOption == 1 && currentSubmenuOption == settingsCount - 1) || currentMenuOption == 2) {
    inMainMenu = true;
    inSubMenu = false;
    clearMatrix();
  } else if (currentMenuOption == 1) {
    inSubSubMenu = true;
    inSubMenu = false;
  }
}

void handleSubSubMenuLogic() {
  if (currentMenuOption == 1) {
    if (currentSubmenuOption == 0 && matrixBrightness >= minBrightness && matrixBrightness <= maxBrightness){
      EEPROM.put(matrixBrightnessAddress, matrixBrightness);
      for(int i = 0; i < matrixSize; i++)
        lc.setLed(0, 0, i, 1);
    }
    if(currentSubmenuOption == 1 && lcdBrightness >= minBrightness && lcdBrightness <= maxBrightness){
      EEPROM.put(lcdBrightnessAddress, lcdBrightness);
    }
    inSubSubMenu = false;
    inSubMenu = true;
  }
}

void readNumber(String numToDisplay, int &number) {
  displayCenteredText(numToDisplay, 0);
  joystickPosition.x = analogRead(pinX);
  displayCenteredText(String(number), 1);

  if (!joyMoved) {
    if (joystickPosition.x < minJoyThreshold)
      number++;
    else if (joystickPosition.x > maxJoyThreshold)
      number--;
    joyMoved = true;
  }

  if (joystickPosition.x >= minJoyThreshold && joystickPosition.x <= maxJoyThreshold)
    joyMoved = false;
}

void displayAbout() {
  scrollText(aboutInfo, 1);
  displayDoodleCharacters();
}

void startGame() {
  currentPosition.x = initialPosition.x;
  currentPosition.y = initialPosition.y;
  generateWalls(levelProbability[currentLevel - 1]);
  toStartGame = false; // the game starts only once
  inGame = true;
  startGameTime = millis();
  for (int row = 0; row < matrixSize; row++)
    for (int column = 0; column < matrixSize; column++)
      lc.setLed(0, row, column, 1);
}


void generateWalls(const int probability) { // now the probability is accordingly to the level
  for (int row = 0; row < matrixSize; row++)
    for (int column = 0; column < matrixSize; column++) {
      if (row - currentPosition.x < explodingZone && column - currentPosition.y < explodingZone)
        matrix[row][column] = false; // to ensure the player can move at the beginning
      else 
        matrix[row][column] = (random(matrixSize * matrixSize) < probability);
      if(matrix[row][column])
        wallCount++;
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
    joyMoved = false;  // to get the joystick position only once
}

bool canMove(const char *direction) {
  if (direction == "right") {
    // no wall and in the map
    return !matrix[currentPosition.x][currentPosition.y - 1] && currentPosition.y > 0;
  }
  if (direction == "left")
    return !matrix[currentPosition.x][currentPosition.y + 1] && currentPosition.y < matrixSize - 1;
  if (direction == "down") {
    if (currentPosition.x < matrixSize - 1)
      return !matrix[currentPosition.x + 1][currentPosition.y];
  }
  if (direction == "up")
    return !matrix[currentPosition.x - 1][currentPosition.y] && currentPosition.x > 0;
  return false;
}

int getState(unsigned long &lastBlink, int rate, bool &blinkState) {
  if (millis() - lastBlink > rate) {
    blinkState = !blinkState; 
    lastBlink = millis();
  }
  return blinkState;
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
        plantBomb();
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
    if(currentLevel < levelCount){
      wallsBlown = 0; // resfresh this to be able to count at the next playing
      currentLevel++;
      displayWinAnimation();
    }
    else {
      displayCenteredText("You finished the game", 0);
    }
  }
  planted = false;  // after a bomb has exploded, we can plant again
}

void destroyWall(int x, int y) {
  if (matrix[x][y]) {
    matrix[x][y] = 0;
    wallsBlown++;  // count to know when the game ends
  }
}

void displayLoseAnimation() {
  for (int row = 0; row < matrixSize; row++) {
    for (int column = 0; column < matrixSize; column++) {
      if (column == matrixSize - 1 || row == matrixSize - 2)
        matrix[row][column] = 1;
      else matrix[row][column] = 0;
    }
  }
}

void displayWinAnimation() {
  for (int row = 0; row < matrixSize; row++) {
    for (int column = 0; column < matrixSize; column++) {
      matrix[row][column] = wPattern[row][column];
    }
  }
}

void clearMatrix() {
  for (int i = 0; i < matrixSize; i++)
    for (int j = 0; j < matrixSize; j++) {
      matrix[i][j] = false;
      lc.setLed(0, i, j, matrix[i][j]);
    }
}

void plantBombLogic() {
  if (millis() - plantBombTime < explodingTime) {
    lc.setLed(0, bombPosition.x, bombPosition.y, getState(lastBombBlink, bombBlinkRate, blinkStateBomb));
  } else {
    explode();  // if the time passed, it explodes
  }
}

void clock() { // count the time from the beginning of the game
  if (millis() - lastTimeShow > showRate) {
    String textToDisplay = "Time: ";
    textToDisplay += String((millis() - startGameTime) / 1000);
    displayCenteredText(textToDisplay,  0);
    lastTimeShow = millis();
  }
}

void displayLevel(){
  String textToDisplay = "Level: ";
  textToDisplay += String(currentLevel);
  displayCenteredText(textToDisplay, 1);
}

void exitGame() {
  inGame = false;
  inMainMenu = true;
  lost = false;
  won = false;
  clearMatrix();
}

void playGame() {
  Serial.print(wallCount);
  Serial.print("  ");
  Serial.println(wallsBlown);

  for (int row = 0; row < matrixSize; row++)
    for (int column = 0; column < matrixSize; column++)
      if (row != currentPosition.x || column != currentPosition.y || lost)  // to make the blink look "smooth"
        lc.setLed(0, row, column, matrix[row][column]);
  if (!lost && !won) {
    clock();
    displayLevel();
    lc.setLed(0, currentPosition.x, currentPosition.y, getState(lastPlayerBlink, playerBlinkRate, blinkStatePlayer));
    checkForPlant();  // get the input of the sw button, and plant the bomb when it is pressed
    if (planted) {    // if the bomb is planted then blink
      plantBombLogic();
    }
    movePlayer();  // read the joystick position and move around
  }
}
