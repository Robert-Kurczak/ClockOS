#include "display.h"

#include <Wire.h>
#include "Arduino-DS3231/DS3231.h"

#define timePrintTime 60    //How long time will be visible, before showing date [s]
#define yearPrintDelay 5    //Time after displayed dd.mm will be changed to yyyy [s]
#define yearPrintTime 5     //How long yyyy will be displayed                    [s]

display mainDisplay;

DS3231 RTC;
RTCDateTime dateHolder;

uint8_t mode = 0;

uint8_t pressedKey(){
    //Returning pressed key
}

void setup(){
    Wire.begin();   //Initialize I2C comunication
}

void loop() {
// mainDisplay.printDate("24082021");

    //Default clock mode
    if(mode == 0){
        double startTime = micros();

        while(micros() - startTime < timePrintTime){

            if(pressedKey() == -1){
                dateHolder = RTC.getDateTime();

                mainDisplay.printTime(String(dateHolder.hour) + String(dateHolder.minute));
            }
            else{
            }
        }


    }
}
