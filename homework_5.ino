// Arduino UNO program to control and monitor ultrasonic sensor (for distance),
// and a light sensor. The program also uses RGB LED Control to alarm the user a threshold has been exceeded
// and a menu for user interaction through the serial monitor

#include <EEPROM.h>

// RGB pins
const int redPin = 6;
const int greenPin = 3;
const int bluePin = 5;
// ultrasonic sensor pins
const int trigPin = 9;
const int echoPin = 10;
// LDR sensor pin
const int photocellPin = A0;

const int maxLogEntries = 10; // only last 10 readings in memory
// the user cannot exceed these thresholds for the sensor
const int maxLdrThreshold = 1000; 
const int maxUltraSonicThreshold = 200;
// the LED can store this max value
const int maxLedValue = 255;
const int minThreshold = 1;
const int convertUserRate = 1000; // the user inserts seconds but we use miliseconds
const float soundSpeed = 0.034;  // cm per us to calculate the distance
const int noOfBounces = 2; // see the distance calculation function
const int upperLimit[] = { 4, 4, 3, 4, 3 }; // the limit of each menu

// addresses to write in the EEPROM
const int eepromUltrasonicThresholdAddress = 0;
const int eepromLdrThresholdAddress = 1;
const int eepromAutoModeAddress = 2;

// the current place in the menu
int currentMenu = 0;
int currentSubMenu = 0;
bool inSubMenu = false;

// rgb values 
int redValue = -1;
int greenValue = -1;
int blueValue = -1;

bool asked = false; // to display a prompt only once

int samplingRate = 2; // the period in which we save values from sensors

// alerts the rgb when exceeds those values
int defaultUltrasonicThreshold = 10; 
int defaultLdrThreshold = 200;

// sample rate from the user should be in this range
const int minSampRate = 1;
const int maxSampRate = 10;

// to save the logs
int superSonicOutputs[maxLogEntries] = { 0 };
int ldrOutputs[maxLogEntries] = { 0 };
int outputIndex = 0;

// to calculate when to sample again
unsigned long lastAutomaticTime = 0;
unsigned long lastSampleTime = 0;
unsigned long lastLogTime = 0;


void setup() {
  Serial.begin(9600);
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);

  pinMode(photocellPin, INPUT);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  // initialize the eeprom
  EEPROM.put(eepromUltrasonicThresholdAddress, defaultUltrasonicThreshold);
  EEPROM.put(eepromLdrThresholdAddress, defaultLdrThreshold);
  EEPROM.put(eepromAutoModeAddress, false);
  printMenu();
}

void loop() {
  getOption(); // read user menu input
  if(inSubMenu)
    doAction(); // act accordingly to the user input
  if (EEPROM.read(eepromAutoModeAddress)) {
    automaticModeAction(); // if automatic mode is ON we listen for alerts
  }
  logData(); // save the data in logs
}

void printMenu() {
  Serial.println();
  Serial.println("--------MENU--------");
  Serial.println("1. Sensor settings");
  Serial.println("2. Reset logger data");
  Serial.println("3. System status");
  Serial.println("4. RGB LED Control");
}

void getOption() {
  if (Serial.available()) {
    int choice = Serial.parseInt();
    if (choice >= 1 && choice <= upperLimit[currentMenu]) { // choice is in good range
      if (!inSubMenu) {
        currentMenu = choice; // if we are in the main menu, we go to a submenu
        inSubMenu = true;
        printCurrentOption();
      } else {
        if (choice == upperLimit[currentMenu]) {
          currentSubMenu = choice;
          goToMainMenu(); // these are the "Back" statements (they are always last)
        }
          else {
           currentSubMenu = choice; // log the submenu user choice
           inSubMenu = true;
         }
      } 
    } else if(currentSubMenu == 0) { // the menu value is not in the good range
      Serial.println("Choose a valid value."); 
      goToMainMenu(); // print the main menu and refresh its' variables
    }
  }
}


// functions to display menu options
void printCurrentOption() {
  switch (currentMenu) {
    case 1:
      printSensorSettings();
      break;
    case 2:
      printLoggerData();
      break;
    case 3:
      printSystemStatus();
      break;
    case 4:
      printRGBLedControl();
      break;
    default:
      Serial.println("The given option is not valid. Try a value between 1 and 4.");
      break;
  }
}

void printSensorSettings() {
  Serial.println("    1.1 Sensors Sampling Interval");
  Serial.println("    1.2 Ultrasonic Alert Threshold");
  Serial.println("    1.3 LDR Alert Threshold");
  Serial.println("    1.4 Back");
}

void printLoggerData() {
  Serial.println("    Are you sure?");
  Serial.println("      2.1 Yes");
  Serial.println("      2.2 No");
  Serial.println("      2.3 Back");
}

void printSystemStatus() {
  Serial.println("    3.1 Current Sensor Readings");
  Serial.println("    3.2 Current Sensor Settings");
  Serial.println("    3.3 Display Logged Data");
  Serial.println("    3.4 Back");
}

void printRGBLedControl() {
  Serial.println("    4.1 Manual Color Control");
  Serial.println("    4.2 LED: Toggle Automatic ON/OFF");
  Serial.println("    4.3 Back");
}

// act accordingly to the user input in the menu 
void doAction() {
  if (currentMenu == 1) {
    doActionSensorSettings();
  } else if (currentMenu == 2) {
    doLogDataAction();
  } else if (currentMenu == 3) {
    doSystemStatusActions();
  } else if (currentMenu == 4) {
    doActionRGB();
  }
}

void doActionRGB() {
  if (currentSubMenu == 1) {
    if (!asked) {
      Serial.println("Give the values of red, green and blue between 0-255 divided by blankspace");
      asked = true;
    }
    // get the user rgb values and write the in the led only when they are valid
    if (Serial.available() && currentSubMenu) {
      redValue = Serial.parseInt();
      greenValue = Serial.parseInt();
      blueValue = Serial.parseInt();
      if (redValue >= 0 && redValue <= maxLedValue && greenValue >= 0 && greenValue <= maxLedValue && blueValue >= 0 && blueValue <= maxLedValue) {
        analogWrite(redPin, redValue);
        analogWrite(greenPin, greenValue);
        analogWrite(bluePin, blueValue);
        EEPROM.update(eepromAutoModeAddress, false); // when we go on manual, the auto is OFF
        goToMainMenu();
      }
    }
  } else if (currentSubMenu == 2) {
    // toggle automatic mode
    if (EEPROM.read(eepromAutoModeAddress)) {
      EEPROM.update(eepromAutoModeAddress, false);
    } else {
      EEPROM.update(eepromAutoModeAddress, true);
    }
    Serial.print("Now the automatic mode for the LED is ");
    if (EEPROM.read(eepromAutoModeAddress) == true) {
      Serial.println("ON");
    } else {
      Serial.println("OFF");
      analogWrite(greenPin, 0); // turn off rgb led when it is OFF
      analogWrite(redPin, 0);
    }
    goToMainMenu();
  }
}


// read the values until they are valid and update the EEPROM
void doActionSensorSettings() {
  if (currentSubMenu == 1) {
    if (!asked) {
      Serial.println("Give a sampling rate for the sensors (1-10)");
      asked = true;
    }
    if (Serial.available()) {
      samplingRate = Serial.parseInt();
      if (samplingRate >= minSampRate && samplingRate <= maxSampRate) {
        goToMainMenu();
      } else {
        Serial.println("The value is not between 1 and 10. Add another one.");
      }
    }
  } else if (currentSubMenu == 2) {
    if (!asked) {
      Serial.println("Give a threshold for the ultrasonic sensor to alert");
      asked = true;
    }
    if (Serial.available()) {
      int reading = Serial.parseInt();
      if (reading >= minThreshold && reading <= maxUltraSonicThreshold) {
        EEPROM.update(eepromUltrasonicThresholdAddress, reading);
        goToMainMenu();
      }
    }
  } else if (currentSubMenu == 3) {
    if (!asked) {
      Serial.println("Give a threshold for the LDR sensor to alert");
      asked = true;
    }
    if (Serial.available()) {
      int value = Serial.parseInt();
      EEPROM.update(eepromLdrThresholdAddress, value);
      if (value >= minThreshold && value <= maxLdrThreshold) {
        goToMainMenu();
      }
    }
  }
}

void doLogDataAction() {
  if (currentSubMenu == 1) { // reset log data 
    for (int i = 0; i < maxLogEntries; i++) {
      ldrOutputs[i] = 0;
      superSonicOutputs[i] = 0;
      outputIndex = 0;
    }
    Serial.println("Logged data has been reset.");
    goToMainMenu();
  } else if (currentSubMenu == 2) {
    goToMainMenu(); // just go back when user says NO
  }
}

void doSystemStatusActions() {
  if (currentSubMenu == 1) {
    unsigned long now = millis();
    // continuosly print the sensor values in the samp rate until the user enters 0
    if (now - lastSampleTime > samplingRate * convertUserRate) {
      printDistance();
      printLightIntensity();
      lastSampleTime = now; 
    }
    if (Serial.available() && inSubMenu) {
      int exit = Serial.parseInt();
      if (exit == 0)
        goToMainMenu();
    }
  } else if (currentSubMenu == 2 && inSubMenu) {
    Serial.print("Sampling rate: ");
    Serial.print(samplingRate);
    Serial.print("   Ultrasonic threshold: ");
    Serial.print(EEPROM.read(eepromUltrasonicThresholdAddress));
    Serial.print("  LDR Alert thershold: ");
    Serial.print(EEPROM.read(eepromLdrThresholdAddress));
    goToMainMenu();
  } else if (currentSubMenu == 3) {
    for (int i = 0; i < maxLogEntries; i++) {
      Serial.print("[");
      Serial.print(i);
      Serial.print(":]  LDR: ");
      Serial.print(ldrOutputs[i]);
      Serial.print("  Supersonic: ");
      Serial.println(superSonicOutputs[i]);
    }
    goToMainMenu();
  }
}

void printDistance() {
  Serial.print("Distance: ");
  Serial.print(getDistance());
  Serial.print(" cm | ");
}

int getDistance() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
// Sound wave reflects from the obstacle, so to calculate the distance we
// consider half of the distance traveled.
  int duration = pulseIn(echoPin, HIGH);

  int distance = duration * soundSpeed / noOfBounces;
  return distance;
}

void printLightIntensity() {
  int photocellValue = analogRead(photocellPin);
  Serial.print(" Light intensity: ");
  Serial.print(photocellValue);
  Serial.println(".  |  Write 0 to exit.");
}

// function to refresh the menu variables and display the main menu again
void goToMainMenu() {
  currentMenu = 0;
  inSubMenu = false;
  currentSubMenu = 0;
  asked = false;
  printMenu();
}

void automaticModeAction() {
  unsigned long now = millis();
  if (now - lastAutomaticTime > samplingRate * convertUserRate) {
    if (getDistance() < EEPROM.read(eepromUltrasonicThresholdAddress) || analogRead(photocellPin) < EEPROM.read(eepromLdrThresholdAddress)) {
      redValue = maxLedValue; 
      greenValue = 0; 
      blueValue = 0;
    } else {
      redValue = 0;
      greenValue = maxLedValue; 
      blueValue = 0;
    }

    digitalWrite(redPin, redValue);
    digitalWrite(greenPin, greenValue);
    digitalWrite(bluePin, blueValue);
  }
}

void logData() {
  unsigned long now = millis();
  if (now - lastLogTime > samplingRate * convertUserRate) {
    int distance = getDistance();
    int lightIntesity = analogRead(photocellPin);
    // if we exceed the limit, we overwrite the log data
    if (outputIndex > maxLogEntries) {
      outputIndex = 0;
    }
    ldrOutputs[outputIndex] = lightIntesity;
    superSonicOutputs[outputIndex] = distance;
    outputIndex++;
    lastLogTime = now;
  }
}
