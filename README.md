# ClockOS
Firmware that operates on my hardware quartz clock I built.

## Table of Contents
* [General info](#general-info)
* [Technologies](#technologies)
* [Setup](#setup)
* [Hardware](#hardware)

## General info
The heart of my clock is atmega328p microcontroller that communicates with precise DS3231 RTC Module to meassure the passage of time even when the clock is off.
Clock can work while being plugged to 5V power supply via USB micro cable, or on built-in battery. It will switch automatically between them.

Clock supports:
* Time setting
* Date setting
* Alarm setting
* Brightness control

Settings will be keep even when the clock will be off. In addition to the date and time, it is able to display the ambient temperature.

## Technologies
 1. C++
 2. [Arduino Wire Library](https://www.arduino.cc/en/reference/wire)
 3. [Adafruit RTClib](https://github.com/adafruit/RTClib) 2.0.2

## Setup
I used arduinoIDE with MiniCore library to burn proper bootloader, with the following settings.
* Clock: "Internal 8Mhz"
* BOD: "BOD disabled"
* EEPROM: "EEPROM retained"
* Compiler LTO: "LTO enabled"

Next using my arduino MEGA as ISP programmer, I uploaded my .ino sketch to the IC.

## Hardware
### Design
<img src="/Images/Clock_Schematic.png" alt="Clock schematic" width="1000">
<img src="/Images/Clock_PCB.png" alt="Clock pcb" width="1000">

### Few pictures
<p>
 <img src="/Images/IMG_1.jpg" alt="Clock picture 1" height="250">
 <img src="/Images/IMG_2.jpg" alt="Clock picture 2" height="250">
 <img src="/Images/IMG_3.jpg" alt="Clock picture 3" height="250">
 <img src="/Images/IMG_4.jpg" alt="Clock picture 4" height="250">
</p>

#### You can see more picture from the whole process of building it [here]()
