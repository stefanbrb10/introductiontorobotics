# Introduction to Robotics (2023 - 2024)

Here is the space for all my work in the Introduction to Robotics class, taken by me in the 3rd year of my Computer Engineering degree. Each homework includes requirements, implementation details, code, and image files.

My goal for the upcoming work is to be able to tell people I know how to build something that you can see without a screen.

<details>
<summary><strong>Homework 1 - Light an RGB LED using separate potentiometers</strong></summary>

This homework focuses on using three potentiometers for each color channel (red, green, and blue). The values from the potentiometers are adapted to output each color's intensity on the LED. After finishing this task, I feel like I understood the basics of the field: the breadboard, Analog vs Digital, the Arduino IDE, etc.

**Components:**
- 1x RGB LED
- 3x Potentiometers
- Arduino UNO
- Breadboard
- 3x 330 Ohm resistors
- Wires

The final hardware work is captured on the picture below and on [this video link](https://youtu.be/odYh3JC_jyA?si=RXHRvevwee9fD3rG), and the code can be found on the repository.

![image](https://github.com/stefanbrb10/introductiontorobotics/assets/35970743/0af54fdd-1940-4970-a4af-ea1c8e126ec9)
</details>

<details>
<summary><strong>Homework 2 - Elevator Control System using Arduino components</strong></summary>

This assignment is an elevator simulator that uses LEDs as signals for the current floor. Each floor has a button to call the elevator. My code has the system's logic implemented, so it functions like a real-life elevator. After completing this task, I feel more confident in my ability to organize my code and translate my thoughts into real-world functionality through programming.

**Components:**
- 4x LEDs (different colors)
- 3x Buttons
- 1x Buzzer
- 4x 330 Ohm resistors & 1x 1kOhm resistor
- Wires

The final hardware work is captured on the picture below and on [this video link](https://youtube.com/shorts/i1zYdx4s9Cg?feature=share), and the code can be found on the repository.

![image](https://github.com/stefanbrb10/introductiontorobotics/assets/35970743/888d47f4-f7b1-465b-a961-61a462445321)
</details>

<details>
<summary><strong>Homework 3 - Draw a character on a 7-segment display using a joystick</strong></summary>

Using a joystick's position, we can draw on a 7-segment display. A long press will reset the display, and a short press will toggle the current segment's position (indicated by blinking). After completing this task, I understood the common anode vs cathode logic, how to use ISR (Interrupt Service Routine), and by each week, my code gets cleaner.

**Components:**
- 1x 7-segment display
- 1x Joystick
- 8x 330 Ohm resistors
- Wires, Arduino UNO

The final hardware work is captured on the picture below and on [this video link](https://www.youtube.com/watch?v=Km1vf0I4Pb0), and the code can be found on the repository as [homework_3.ino](https://github.com/stefanbrb10/introductiontorobotics/blob/main/homework_3.ino).

![image](https://github.com/stefanbrb10/introductiontorobotics/assets/35970743/13bf82e2-d57e-4298-a8d6-e794f54ce552)
</details>

<details>
<summary><strong>Homework 4 - Stopwatch timer with Shift Register for 4x7-segment display</strong></summary>

This task uses a 4x7-segment digit display for a stopwatch timer, similar to the default iPhone app, but limited to the capabilities of the 4x7-segment display. There are 3 buttons (start/pause, reset, save/cycle lap). I feel the purpose of this task has been accomplished. I learned about the precision of interrupts, how a shift register works (clock, latch, data), and my code commenting gets more complete.

**Components:**
- 1x 4-7segment display
- 1x Shift register 74HC595
- 330 Ohm resistors
- Wires, Arduino Uno

The final hardware work is captured on the picture below and on [this video link](https://youtube.com/shorts/aBiMd_EpWYw?feature=share), and on the code in the repository as [homework_4.ino](https://github.com/stefanbrb10/introductiontorobotics/blob/main/homework_4.ino).

![image](https://github.com/stefanbrb10/introductiontorobotics/assets/35970743/9fd29df9-7123-4fe7-be7e-319e23dd095a)
</details>

<details>
<summary><strong>Homework 5 - Pseudo-smart environment monitor and logger </strong></summary>

This Arduino UNO project combines an ultrasonic sensor and a light sensor, with user interaction through the serial monitor. A menu-driven interface allows users to configure sensor settings, reset data logs, check system status, and control an RGB LED for alerts. The use of EEPROM enables data persistence, and the system logs sensor data at specified intervals. The project offers practical experience in sensor integration, menu systems, EEPROM usage, user input handling, RGB LED control, and timing in embedded systems.

**Components:**
- 1x LDR sensor
- 1x Ultrasonic sensor
- 1k and 330 Ohm resistors
- 1x RGB LED
- Wires, Arduino UNO

The final hardware work is captured on the picture below and on [this video link](https://youtu.be/zV3ibLjZzx8), and the code can be found on the repository as [homework_5.ino](https://github.com/stefanbrb10/introductiontorobotics/blob/main/homework_5.ino).

![image](https://github.com/stefanbrb10/introductiontorobotics/assets/35970743/7ece5583-0ac0-4c2b-8f7d-92d04be0f223)

</details>


<details>
<summary><strong>Homework 6 - Bomberman Game on matrix </strong></summary>

This project has a bomberman style logic. The player moves on a map with randomly generated walls. Winning consists of demolishing all the walls using a bomb. Losing consists of being in the dangerous area while the bomb is exploding. 

**Components:**
- 1x 8x8 matrix
- 1x MAX7219
- resistors and capacitors
- 1x joystick
- Wires, Arduino UNO

The final hardware work is captured on the picture below and on [this video link](https://youtube.com/shorts/MaLJcmhFMBo?feature=share), and the code can be found on the repository as [homework_5.ino](https://github.com/stefanbrb10/introductiontorobotics/blob/main/homework_6.ino).

![image](https://github.com/stefanbrb10/introductiontorobotics/assets/35970743/cbf32460-1a92-47ba-abe6-525248509c52)


</details>


<details>
<summary><strong>Homework 7 - Matrix project - checkpoint #1 </strong></summary>

After homework 6, I added an LCD screen for menu options and dynamic details (level, time and will implement more), stored values in EEPROM. Next, i will implement lifes, enemies, leaderboard and more.

**Components:**
- 1x 8x8 matrix
- 1x MAX7219
- 1x LCD screen
- 1x potentiometer
- resistors and capacitors
- 1x joystick
- Wires, Arduino UNO

The final hardware work is captured on the picture below and on [this video link](https://youtube.com/shorts/MaLJcmhFMBo?feature=share), and the code can be found on the repository as [homework_5.ino](https://github.com/stefanbrb10/introductiontorobotics/blob/main/homework_6.ino).

![image](https://github.com/stefanbrb10/introductiontorobotics/assets/35970743/cbf32460-1a92-47ba-abe6-525248509c52)


</details>
