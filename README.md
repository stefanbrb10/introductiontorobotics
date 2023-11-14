# Introduction to Robotics (2023 - 2024)
Here is the space for all my work in the Intoduction to Robotics class, taken by me in the 3rd year of my Computer Engineering degree. Each homework includes requirements, implementation details, code and image files.

My goal for the upcoming work is to be able to tell people I know how to build something that you can see without a screen.

## Homework 1 - Light an RGB LED using separate potentiometers
This homework focuses on using three potentiometers for each color channel (red, green and blue). The values from the potentiometers are adapted to ouput each color's intensity on the LED.
After finishing this task i feel like i understood the basics of the field: the breadboard, Analog vs Digital, the Arduino IDE, etc.
<br> <br>
Components:
  - 1x RGB LED
  - 3x Pontentiometers
  - Arduino UNO
  - Breadboard
  - 3x 330 Ohm resistors
  - Wires

The final hardware work is captured on the picture below and on [this video link](https://youtu.be/odYh3JC_jyA?si=RXHRvevwee9fD3rG), and the code can be found on the repository. 
<br>
![image](https://github.com/stefanbrb10/introductiontorobotics/assets/35970743/0af54fdd-1940-4970-a4af-ea1c8e126ec9)

## Homework 2 - Elevator Control System using Arduino components
This assignment is an elevator simulator that uses LEDs as signals for the current floor. Each floor has a button to call the elevator. My code has the system's logic implemented, so it functions like a real-life elevator.<br> <br> After completing this task, I feel more confident in my ability to organize my code and translate my thoughts into real-world functionality through programming.
<br><br>
Components:
- 4x LEDs (different colors)
- 3x buttons
- 1x buzzer
- 4x 330 Ohm resistors & 1x 1kOhm resistor
- Wires

  The final hardware work is captured on the picture below and on [this video link](https://youtube.com/shorts/i1zYdx4s9Cg?feature=share), and the code can be found on the repository.
  <br>
  ![image](https://github.com/stefanbrb10/introductiontorobotics/assets/35970743/888d47f4-f7b1-465b-a961-61a462445321)

## Homework 3 - Draw a character on a 7 segment display using joystick
Using a joystick's position, we can draw on a 7 segment display. A long press will reset the display and a short press will toggle the current segment's position (indicated by blinking). <br><br>
After completing this task, I understood the common anode vs cathode logic, how to use ISR (Interrupt Service Routine) and by each week, my code gets cleaner.
<br> <br>
Components:
- 1x 7 segment display
- 1x joystick
- 8x 330 Ohm resistors
- Wires, Arduino UNO

  The final hardware work is captured on the picture below and on [this video link](https://www.youtube.com/watch?v=Km1vf0I4Pb0), and the code can be found on the repository as [homework_3.ino](https://github.com/stefanbrb10/introductiontorobotics/blob/main/homework_3.ino).
![image](https://github.com/stefanbrb10/introductiontorobotics/assets/35970743/13bf82e2-d57e-4298-a8d6-e794f54ce552)

## Homework 4 - Stopwatch timer with Shift Register for 4x7segment display
This task uses 4x7segment digit display for a stopwatch timer, similar to the default iPhone app, but limited to the capabilites of the 4x7segment display. There are 3 buttons (start/pause, reset, save/cycle lap).
<br> <br>
I feel the purpose of this task has been accomplished. I learned about the precision of interrupts, how a shift register works (clock, latch, data) and my code commenting gets more complete. 
<br> <br>
Components:
- 1x 4-7segment display
- 1x shift register 74HC595
- 330 Ohm resistors
- Wires, Arduino Uno

  The final hardware work is captured on the picture below and on [this video link] (https://youtube.com/shorts/aBiMd_EpWYw?feature=share), and on the code in the repository as [homework_4.ino] (https://github.com/stefanbrb10/introductiontorobotics/blob/main/homework_4.ino).
![image](https://github.com/stefanbrb10/introductiontorobotics/assets/35970743/9fd29df9-7123-4fe7-be7e-319e23dd095a)


