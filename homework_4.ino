 // Shift register pins
const int latchPin = 11; // connects to STCP
const int clockPin = 10; // connects to SHCP
const int dataPin = 12; // connecst to DS

// Button pins
const int stopPin = 2;
const int resetPin = 8;
const int lapPin = 3;

// 7 segment display pins
const int segD1Pin = 4;
const int segD2Pin = 5;
const int segD3Pin = 6;
const int segD4Pin = 7;
int displayDigits[] = { segD1Pin, segD2Pin, segD3Pin, segD4Pin };

const int displayCount = 4; // we have 4 numbers on our device
const int lapCount = 4; // we can save up to 4 laps, if we exceed, we override the 1st and so on
const int maxValueToDisplay = 10000; // maximum the screen can support (4 digits)
const int last0MinuteValue = 1000; // after this value, we activate the 2nd decimal point (for minutes)
const int secondsInMinute = 60; 

const int encodedNumbersCount = 10;

// Encodings for the display (0 is off 1 is on)
const byte encodedNumbers[encodedNumbersCount] = {
  B11111100,  // 0
  B01100000,  // 1
  B11011010,  // 2
  B11110010,  // 3
  B01100110,  // 4
  B10110110,  // 5
  B10111110,  // 6
  B11100000,  // 7
  B11111110,  // 8
  B11110110,  // 9
};

const byte decimalPointEncoding = B00000001;
const byte emptyScreenEncoding = B00000000; // we will use it to prevent ghosting

unsigned long lastIncrement = 0;
unsigned long countingDelay = 100;

unsigned long displayValue = 0; // the raw value to display
unsigned long lap[lapCount] = { 0 };
int currentLap = 0; // the lap which we will store when we press a button
int currentLapToDisplay = 0;

bool cyclingLaps = false; // indicates if we are in cycling mode
bool isPaused = false;
bool started = false;


const int debounceDelay = 50;
byte pauseButtonState = LOW;
unsigned long lastDebounceTimePause = 0;
volatile bool hasPressedPause = false;
volatile bool startCounterForPause = false;
volatile unsigned long lastLapInterruptTime = 0;


byte buttonStateReset = LOW;
byte lastResetButtonState = LOW;
unsigned long lastDebounceTimeReset = 0;


void writeReg(int digit) {
  digitalWrite(latchPin, LOW); // prepare to shift data
  shiftOut(dataPin, clockPin, MSBFIRST, digit); // shift out the current digit's byte
  digitalWrite(latchPin, HIGH); // latch the data onto the output pins
}

void handlePauseInterrupt() {
  hasPressedPause = true; // to indicate the button was pressed
  startCounterForPause = true; // to know we can start debouncing
}

void readPauseButton() {
  if (startCounterForPause) {
    lastDebounceTimePause = millis();
    startCounterForPause = false;
  }
  if (millis() - lastDebounceTimePause > debounceDelay) {
    pauseButtonState = digitalRead(stopPin);
    if (pauseButtonState == LOW) {
      if (!started)
        started = true; // start the screen
      else {
        isPaused = !isPaused; // toggle the pausing
        cyclingLaps = false; // we are sure we are not in cycling mode
      }
    }
    hasPressedPause = false;
  }
}

void checkReset() {
  int resetButtonState = digitalRead(resetPin);

  if (resetButtonState != lastResetButtonState) {
    lastDebounceTimeReset = millis();
  }

  if ((millis() - lastDebounceTimeReset) > debounceDelay) {
    if (resetButtonState == LOW) {
      if (isPaused)
        displayValue = 0; // reset the number on the screen
      if (cyclingLaps) {
        // reset the laps
        lap[0] = lap[1] = lap[2] = lap[3] = 0;
      }
    }
  }
  lastResetButtonState = resetButtonState;
}

void handleLapInterrupt() {
  static unsigned long interruptLapTime = 0;  
  interruptLapTime = millis();               
  if (interruptLapTime - lastLapInterruptTime > debounceDelay) {  
    if (!isPaused) {
      currentLap = currentLap % lapCount; // not exceeding the 4 lap limit
      lap[currentLap] = displayValue; // store the lap in the array
      currentLap++;
    }
    // cycling mode means we are in pause mode and the display either doesn't show anything
    // or it shows a value from the lap array
    if (isPaused && (displayValue == 0 || displayValue == lap[currentLapToDisplay-1])) {
      cyclingLaps = true;
      currentLapToDisplay %= lapCount;
      displayValue = lap[currentLapToDisplay];
      currentLapToDisplay++;
    }
  }
  lastLapInterruptTime = interruptLapTime;
}


void activateDisplay(int displayNumber) {
  // turn off all pins to avoid ghosting
  for (int i = 0; i < displayCount; i++) {
    digitalWrite(displayDigits[i], HIGH);
  }
  //turn on the current digit
  digitalWrite(displayDigits[displayNumber], LOW);
}

void writeNumber(int number) {
  int currentNumber = number;
  int lastDigit = 0;

  // start with least significant digit 
  for (int displayDigit = displayCount - 1; displayDigit >= 0; displayDigit--) {
    lastDigit = currentNumber % 10;
    activateDisplay(displayDigit); // activate the display for the current digit
    writeReg(encodedNumbers[lastDigit]); // output the digit
    // always activate the decimal point for deciseconds
    // and activate it for minutes when we have minutes
    if (displayDigit == 2 || 
    secondsToMinutes(displayValue) >= last0MinuteValue && displayDigit == 0)
      writeReg(decimalPointEncoding);
    currentNumber /= 10;
    writeReg(emptyScreenEncoding);
  }
}

// convert the raw (seconds) values to an actual time value (with minutes)
int secondsToMinutes(int value){
  int deciseconds = value % 10;
  int seconds = (value/10) % secondsInMinute;
  int minutes = (value/10) / seconds;
  return (minutes * 1000 + seconds * 10 + deciseconds);
}

void setup() {

  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, OUTPUT);

  pinMode(stopPin, INPUT_PULLUP);
  pinMode(resetPin, INPUT_PULLUP);
  pinMode(lapPin, INPUT_PULLUP);

  // initialize digit control pins turn them off
  for (int i = 0; i < displayCount; i++) {
    pinMode(displayDigits[i], OUTPUT);
    digitalWrite(displayDigits[i], LOW);
  }

  Serial.begin(9600);
  attachInterrupt(digitalPinToInterrupt(stopPin), handlePauseInterrupt, FALLING);
  attachInterrupt(digitalPinToInterrupt(lapPin), handleLapInterrupt, FALLING);
}

void loop() {
  // if the interrupt indicates PAUSE was pressed, we interpret this action accordingly
  if (hasPressedPause)
    readPauseButton();

  // for debugging 
  for (int i = 0; i < lapCount; i++) {
    Serial.print(lap[i]);
    Serial.print(" ");
  }
  Serial.println();

  checkReset();

  if (started) {
    // increment each decisecond 
    if (millis() - lastIncrement > countingDelay) {
      if (!isPaused)
        displayValue++;
      displayValue %= maxValueToDisplay; // make sure we don't exceed 4 digits
      lastIncrement = millis();
    }
    writeNumber(secondsToMinutes(displayValue)); // write the value with minutes 
  }
}