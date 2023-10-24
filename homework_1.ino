// Program to output an RGB LED using the values from three potentiometeres

// The pin indexes for each color of the RGB LED
const int redPin = 11;
const int greenPin = 10;
const int bluePin = 9;

// The pin indexes for each potentiometer (we can also use other input tools)
const int redValuePin = A0;
const int greenValuePin = A1;
const int blueValuePin = A2;

// The values that the poteniometers
// return which will be used to represent each color's intensity
int redValue = 0;
int greenValue = 0;
int blueValue = 0;

// The potentiometer color's value has to be converted 
// from the raw interval to the LED interval
const int lowerLimitRaw = 0;
const int upperLimitRaw = 1023;
const int lowerLimitLED = 0;
const int upperLimitLED = 255;
const int timeReset = 1000;

void setup() {
Serial.begin(9600); // the Serial Monitor

// Make the color pins an output
pinMode(redPin, OUTPUT);
pinMode(greenPin, OUTPUT);
pinMode(bluePin, OUTPUT);
}


void loop() {
// First, we read the raw values from the potentiometer
// then, we map it to a value compatible with the LED
redValue = analogRead(redValuePin);
redValue = map(redValue, lowerLimitRaw, upperLimitRaw, lowerLimitLED, upperLimitLED);
greenValue = analogRead(greenValuePin); 
greenValue = map(greenValue, lowerLimitRaw, upperLimitRaw, lowerLimitLED, upperLimitLED);
blueValue = analogRead(blueValuePin);
blueValue = map(blueValue, lowerLimitRaw, upperLimitRaw, lowerLimitLED, upperLimitLED);

printValues();
setColor(redValue, greenValue, blueValue);
delay(timeReset); // repeat the process for a period of time(ms)
}

// function to output the color on the RGB LED
void setColor(int red, int green, int blue) {
analogWrite(redPin, red);
analogWrite(greenPin, green);
analogWrite(bluePin, blue);
}

// utility function to display the color values on the Serial Monitor
void printValues(){
Serial.print(redValue);
Serial.print(", ");
Serial.print(greenValue);
Serial.print(", ");
Serial.println(blueValue);
}
