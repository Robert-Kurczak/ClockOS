# ClockOS
Firmware that operates on my hardware quartz clock I built.

## Table of Contents
* [General info](#general-info)
* [Technologies](#technologies)
* [Setup](#setup)
* [Hardware](#hardware)

## General info
The heart of my clock is atmega328p microcontroller that communicates with precise DS3231 RTC Module to meassure the passage of time even when the clock is off.
Clock can work while being plugged to 5V power supply via USB micro cable, or on builtin battery. It will switch automatically between them.

Clock supports:
* Time setting
* Date setting
* Alarm setting
* Brightness control

Settings will be keep even when the clock will be off.

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

Next using my arduino MEGA as ISP programmer, I uploaded my sketch to the IC.

## Hardware
