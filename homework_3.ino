// the pins for the input 
const int pinSW = 2;
const int pinX = A0;
const int pinY = A1;

// the pins for the output
const int pinA = 12;
const int pinB = 10;
const int pinC = 9;
const int pinD = 8;
const int pinE = 7;
const int pinF = 6;
const int pinG = 5;
const int pinDP = 4;

const int segSize = 8; // our 7 segment display has 7 pins for number +dp

// the values from the joystick
int xValue = 0; 
int yValue = 0;

// volatile variables from the interrupt function
volatile bool hasPressed = false; // to acknowledge that the button got pressed
volatile bool startCounterPress = false; // to start the count for the debounce
volatile bool canReset = false; // to know we are able to reset at the current moment
volatile bool startReset = false; // to know when to count the time until releasing the button

unsigned long lastDebounceTime = 0; 
const unsigned long debounceDelay = 50;
const int longPressDuration = 2000; 

byte swState = LOW;
byte lastSwState = HIGH;
const int blinkingPeriod = 500;
unsigned long prevBlink = 0;
byte blinkState = LOW;
byte ledState[segSize] = { LOW }; // LOW if the segment is OFF
unsigned long lastResetTime = 0;
unsigned long resetTime = 0;


bool joyMoved = false; 
// the threshold on which the joystick changes values on the display
const int minThreshold = 300;
const int maxThreshold = 700;


int segments[segSize] = {
  pinA, pinB, pinC, pinD, pinE, pinF, pinG, pinDP
};

const int directionCount = 4;
//each number represents the direction column in the movementMatrix
const int up = 0;
const int down = 1;
const int left = 2;
const int right = 3;

const int canNotMove = -1;
const int segmentA = 0;
const int segmentB = 1;
const int segmentC = 2;
const int segmentD = 3;
const int segmentE = 4;
const int segmentF = 5;
const int segmentG = 6;
const int segmentDP = 7;

// For example, we can move from segment G to segments A (up) and D (down)
const int movementMatrix[segSize][directionCount] = {
     // up         down    left       right
  { canNotMove, segmentG, segmentF, segmentB },     // a
  { segmentA, segmentG, segmentF, canNotMove },     // b
  { segmentG, segmentD, segmentE, segmentDP },      // c
  { segmentG, canNotMove, segmentE, segmentC },     // d
  { segmentG, segmentD, canNotMove, segmentC },     // e
  { segmentA, segmentG, canNotMove, segmentB },     // f
  { segmentA, segmentD, canNotMove, canNotMove },   // g
  { canNotMove, canNotMove, segmentC, canNotMove }  // dp
};

// we always start from DP
int currentPosition = segmentDP;

void turnOffLeds() {
  for (int i = 0; i < segSize - 1; i++)
    ledState[i] = LOW;
  currentPosition = segmentDP; // even after reset we start over from DP
}

void blinkLed(int pin) {
  unsigned long now = millis();
  if (now - prevBlink >= blinkingPeriod) {
    blinkState = !blinkState;
    prevBlink = now;
  }
}

void displayLeds() {
  for (int i = 0; i < segSize; i++) {
    if (currentPosition == i) {
      blinkLed(currentPosition);
      digitalWrite(segments[currentPosition], blinkState); // blink the current LED
    } else {
      digitalWrite(segments[i], ledState[i]); // the other leds don't blink
    }
  }
}

// when pressing the SW, we know we might reset the display or turn on a segment
void handleInterrupt() {
  hasPressed = true;
  startCounterPress = true;
  canReset = true;
  startReset = true;
}

void readButton() {
  if (startCounterPress) {
    // the exact moment we need to count 
    lastDebounceTime = millis();
    startCounterPress = false;
  }
    // check if the button is stable for the debounce delay
  if (millis() - lastDebounceTime > debounceDelay) {
    swState = digitalRead(pinSW);
    if (swState != lastSwState && swState == LOW) {
      // if the button was pressed and NOT RESET, we toggle the segment
      ledState[currentPosition] = !ledState[currentPosition];
    }
    hasPressed = false;
  }
}

void checkForReset() {
  if (startReset) {
    lastResetTime = millis();
    startReset = false;
  }
  swState = digitalRead(pinSW);

  if (swState != lastSwState && swState == LOW) {
    if (millis() - lastResetTime > longPressDuration) {
      turnOffLeds();
      canReset = false;
    }
  } else {
    canReset = false;
  }
}

void moveLeds() {
  int nextPosition = -1;
  xValue = analogRead(pinX);
  yValue = analogRead(pinY);

  if (!joyMoved) {
    if (yValue < minThreshold)
      nextPosition = movementMatrix[currentPosition][left];
    if (yValue > maxThreshold)
      nextPosition = movementMatrix[currentPosition][right];
    if (xValue < minThreshold)
      nextPosition = movementMatrix[currentPosition][down];
    if (xValue > maxThreshold)
      nextPosition = movementMatrix[currentPosition][up];

    if (nextPosition != -1) // if the joystick indicates a possible direction
      currentPosition = nextPosition;
    joyMoved = true;
  }

  if (yValue >= minThreshold && yValue <= maxThreshold && xValue >= minThreshold && xValue <= maxThreshold)
    joyMoved = false;
}


void setup() {
  Serial.begin(9600);
  pinMode(pinSW, INPUT_PULLUP);
  pinMode(pinX, INPUT);
  pinMode(pinY, INPUT);

  for (int i = 0; i < segSize; i++) {
    pinMode(segments[i], OUTPUT);
  }

  attachInterrupt(digitalPinToInterrupt(pinSW), handleInterrupt, FALLING);
}

void loop() {
  displayLeds(); // show each segment's state
  moveLeds(); // read the joystick input

  // if the IRS indicates the button was pressed, we check if we should toggle the segment or reset the display
  if (hasPressed) 
    readButton();

  if (canReset) 
    checkForReset();
}
