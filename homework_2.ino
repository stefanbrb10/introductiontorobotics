const int numFloors = 3; // the number of floors in the elevator
const int buzzerPin = 9; 
const int blinkingLedPin = 3; 
const int blinkingLedPeriod = 500; // the period at which the LED blinks

const int floor0ButtonPin = 2;
const int floor0LEDPin = 13;
const int floor1ButtonPin = 4;
const int floor1LEDPin = 12;
const int floor2ButtonPin = 7;
const int floor2LEDPin = 8;

int activeFloor = 0; // the current floor
int targetFloor = 0; // the floor we want to get to
int direction; // the elevator moves up or down

// the elevator's 3 states' starting time and delay time
// (because we don't want to use delay() function)
unsigned long openingDoorTime = 0; 
const int openDoorDelay = 2000;
unsigned long closingDoorTime = 0;
const int closeDoorDelay = 2000;
unsigned long movingElevatorTime = 0;
const int movingElevatorDelay = 3000;

// the debounce delay value for the buttons
unsigned int debounceDelay = 50;

// the elevator's 3 states' values
bool isClosing = false;
bool isMoving = false;
bool isOpening = false;

// utility class to include floor specific elements and actions
class Floor{
  private:
    int floorNumber; // floor index
    const int buttonPin; // the pin for the  elevator's calling buttons
    const int ledPin; // it lights up when we are at this specific floor
    byte buttonState; 
    // variables for the debouncing 
    byte reading;
    byte lastReading;
    byte lastDebounceTime;
  
  public:
    Floor(int floorNumber, const int buttonPin, const int ledPin) : floorNumber(floorNumber),
                                       buttonPin(buttonPin), ledPin(ledPin),
                                       buttonState(LOW),
                                       reading(LOW),
                                       lastReading(LOW),
                                       lastDebounceTime(0) {}
    
    //function to call in the setup()
    Initialize(){
      pinMode(buttonPin, INPUT_PULLUP); // the buttons are PULLUP type
      pinMode(ledPin, OUTPUT);
    }

    // check the state of the button
    ReadButton() {
      // don't check the the button if it is the same as the activeFloor
      if (floorNumber != activeFloor) {
        reading = digitalRead(buttonPin);
        if (reading != lastReading) {
          // record the current time for debouncing
          lastDebounceTime = millis();
        }
        // check if the button is stable for the debounce delay
        if ((millis() - lastDebounceTime) > debounceDelay) {
          if (reading != buttonState) {
            buttonState = reading;
            // check if the button is pressed and the elevator is not in motion
            if (buttonState == LOW && !isClosing && !isMoving && !isOpening) {
              // now we can set the targetFloor safely
              targetFloor = floorNumber;
              isOpening = true;
              openingDoorTime = millis(); // now the doors will open
            }
          }
        }
        lastReading = reading;
      }
    }

    int getLedPin() {
        return ledPin;
    }

};

// initialize the array of floors
Floor floors[numFloors] = {
  Floor(0, floor0ButtonPin, floor0LEDPin),
  Floor(1, floor1ButtonPin, floor1LEDPin),
  Floor(2, floor2ButtonPin, floor2LEDPin)
};

void setup() {
Serial.begin(9600); // the Serial Monitor
 for(int i = 0; i < numFloors; i++){
   floors[i].Initialize(); // call the function to determine the INPUT and OUTPUTS (floor buttons and LEDs)
 }
pinMode(buzzerPin, OUTPUT);
pinMode(blinkingLedPin, OUTPUT);
digitalWrite(blinkingLedPin, HIGH); // at the start this LED is on
}

void loop() {
  // if the elevator is not in motion, we read the buttons
  if (!isMoving && !isClosing && !isOpening) {
    floors[0].ReadButton();
    floors[1].ReadButton();
    floors[2].ReadButton();
  }
  digitalWrite(floors[activeFloor].getLedPin(), HIGH); // turn on the current floor LED
  Serial.print(activeFloor);
  Serial.print(" , "); // print the current and target floor
  Serial.println(targetFloor);
  // check if the elevator didn't reach the target
  if (targetFloor != activeFloor) {
    digitalWrite(blinkingLedPin, HIGH); // this LED is off only when the elevator moves (it will blink)
    if (isOpening) {
      unsigned long currentTime = millis();
      // check if the elevator is in the state of opening
      if (currentTime - openingDoorTime <= openDoorDelay) {
        tone(buzzerPin, 330);
      } else {
        // if it is not in the state of opening the doors
        // we want to close the doors
        isOpening = false;
        isClosing = true;
        closingDoorTime = millis();
      }
    }
    if (isClosing) {
      digitalWrite(blinkingLedPin, HIGH);
      unsigned long currentTime = millis();
      if (currentTime - closingDoorTime <= closeDoorDelay) {
        tone(buzzerPin, 400); // different tone than the opening door tone
      } else {
        isClosing = false;
        isOpening = false;
        isMoving = true; // if the doors are closed, the elevator starts to move
        movingElevatorTime = millis();
      }
    }
    if (isMoving) {
      // we see if the elevator should go up or down
      direction = (activeFloor < targetFloor) ? (1) : (-1);
      unsigned long currentTime = millis();
      if (currentTime - movingElevatorTime <= movingElevatorDelay) {
        tone(buzzerPin, 1000);
        // the blinkingLED should blink when the elevator moves
        if((currentTime - movingElevatorTime) % blinkingLedPeriod < blinkingLedPeriod / 2)
          digitalWrite(blinkingLedPin, HIGH);
        else
          digitalWrite(blinkingLedPin, LOW);
      }
      else{ // we reached a floor (at this moment we don't know if it is the destination)
        digitalWrite(floors[activeFloor].getLedPin(), LOW); // turn off the LED for the previous floor
        activeFloor += direction;
        movingElevatorTime = millis();
        noTone(buzzerPin);
      }
      if(activeFloor == targetFloor){ // we reached the destination floor
        isMoving = false;
        isOpening = false;
        isClosing = false; // the elevator motion ended
        tone(buzzerPin, 100, 1000); // it signals that someone can call the elevator again
        digitalWrite(blinkingLedPin, HIGH); // the led stops blinking
      }
    }
  }
}

