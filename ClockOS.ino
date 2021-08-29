#include <Wire.h>
#include "display.h"
#include "Arduino-DS3231/DS3231.h"

#define timePrintTime 600000000     //How long time will be visible, before showing date [us]
#define yearPrintDelay 5000000      //Time after displayed dd.mm will be changed to yyyy [us]
#define yearPrintTime 5000000       //How long yyyy will be displayed                    [us]

display mainDisplay;
DS3231 RTC;
RTCDateTime dateHolder;

bool interrupt(){
    //TODO
}

uint8_t checkMode(){
    //TODO
}

void printDate(display& display, DS3231& RTC){
    float startTime = micros();

    while(micros() - startTime < yearPrintDelay){
        display.print("2708");
    }

    startTime = micros();

    while(micros() - startTime < yearPrintTime){
        display.print("2021");
    }
}

void printTime(display& display, DS3231& RTC){
    display.colonOn();

    float startTime = micros();

    while(micros() - startTime < timePrintTime){
        display.print("1257");
    }
    
    display.colonOff();

    printDate(display, RTC);
}

void setup(){
    Wire.begin();   //Initialize I2C comunication
}

void loop(){
    //Default clock mode
    if(checkMode() == 0){
        printTime(mainDisplay, RTC);
    }
}