#include <Wire.h>
#include "display.h"
#include "RTClib.h"

#define timePrintTime 60000000      //How long time will be visible, before showing date [us]
#define yearPrintDelay 5000000      //Time after displayed dd.mm will be changed to yyyy [us]
#define yearPrintTime 5000000       //How long yyyy will be displayed                    [us]

#define modeSelectPin A0
#define button1Pin A3
#define button2Pin A2
#define button3Pin A1

display mainDisplay;
RTC_DS3231 RTC;

uint8_t lastMode;
bool interrupt(){
    uint8_t currentMode = checkMode();

    if(currentMode != lastMode){
        lastMode = currentMode;
        return true;
    }

    return false;
}

uint8_t checkMode(){
    //Analog values that indicate current mode, from 0 to 3
    int modeValues[4] = {485, 324, 244, 195};

    for(int i = 0; i < 4; i++){
        if(abs(analogRead(modeSelectPin) - modeValues[i]) < 30){
            return i;
        }
    }
}

uint8_t pressedButton(){
    if(!digitalRead(button1Pin)){
        return 1;
    }
    else if(!digitalRead(button2Pin)){
        return 2;
    }
    else if(!digitalRead(button3Pin)){
        return 3;
    }
    else{
        return -1;
    }
}

void printDate(display& display, RTC_DS3231& RTC){
    float startTime = micros();

    while(micros() - startTime < yearPrintDelay){
        if(interrupt()) return;
        
        DateTime now = RTC.now();
        String month = now.month() < 10 ? "0" + String(now.month()) : String(now.month());
        display.print(String(now.day()) + "." + month);
    }

    startTime = micros();

    while(micros() - startTime < yearPrintTime){
        if(interrupt()) return;

        DateTime now = RTC.now();
        display.print(String(now.year()));
    }
}

void printTime(display& display, RTC_DS3231& RTC, long int timeOnDisplay = timePrintTime, bool showDate = true){
    display.colonOn();

    float startTime = micros();

    while(micros() - startTime < timeOnDisplay){
        if(interrupt()){
          display.colonOff();
          return;
        }

        DateTime now = RTC.now();
        
        String hour = now.hour() < 10 ? "0" + String(now.hour()) : String(now.hour());
        String minute = now.minute() < 10 ? "0" + String(now.minute()) : String(now.minute());
        display.print(hour + minute);
    }
    
    display.colonOff();

    if(showDate){
        printDate(display, RTC);
    }
}

void setup(){
    pinMode(A0, INPUT);
    pinMode(A1, INPUT_PULLUP);
    pinMode(A2, INPUT_PULLUP);
    pinMode(A3, INPUT_PULLUP);

    RTC.begin();
    // RTC.adjust(DateTime(F(__DATE__), F(__TIME__)));

    lastMode = checkMode();
}

void loop(){
    uint8_t currentMode = checkMode();

    //Default clock mode
    if(currentMode == 0){
        printTime(mainDisplay, RTC);
    }
    //Set time mode
    else if(currentMode == 1){
        //Changing hours by default
        int offset = 3600;
        mainDisplay.colonOn();

        while(checkMode() == 1){
            printTime(mainDisplay, RTC, 50000, false);

            DateTime now = RTC.now();

            switch(pressedButton()){
                case 1:
                    RTC.adjust(now + TimeSpan(offset));
                    printTime(mainDisplay, RTC, 250000, false);

                    break;

                case 2:
                    RTC.adjust(now - TimeSpan(offset));
                    printTime(mainDisplay, RTC, 250000, false);

                    break;

                case 3:
                    if(offset == 1){
                        offset = 3600;
                    }
                    else{
                        offset /= 60;
                    }

                    printTime(mainDisplay, RTC, 250000, false);

                    break;
            }
        }

        mainDisplay.colonOff();
    }
}
