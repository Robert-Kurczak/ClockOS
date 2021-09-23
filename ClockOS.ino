#include <Wire.h>
#include "display.h"
#include "RTClib.h"
#include "alarm.h"

#define timePrintTime 600000000	//How long time will be visible, before showing date	[us]
#define yearPrintDelay 5000000	//Time after displayed dd.mm will be changed to yyyy	[us]
#define yearPrintTime 5000000 	//How long yyyy will be displayed                    	[us]

#define modeSelectPin A0
#define button1Pin A3
#define button2Pin A2
#define button3Pin A1

#define buzzerPin 13

display g_mainDisplay;
RTC_DS3231 g_RTC;
alarm g_alarm(g_RTC, 0, 1, 2, buzzerPin);

uint8_t lastMode;
bool alarmInterruptFired = false;
//Function for detecting user interaction, like pressing key or changing mode
bool interrupt(bool checkForButtons = false){
	//---Mode change interrupt---
	uint8_t currentMode = checkMode();

	if(currentMode != lastMode){
		lastMode = currentMode;
		return true;
	}
	//---

	if(g_alarm.fired() && !alarmInterruptFired){
		alarmInterruptFired = true;
		return true;
	}

	if(checkForButtons){
		return pressedButton() != -1;
	}

	return false;
}

//Function for checking what mode is currently set
uint8_t checkMode(){
	//Analog ADC values that indicates current mode, from 0 to 3
	int modeValues[4] = {485, 324, 244, 195};

	for(int i = 0; i < 4; i++){
		if(abs(analogRead(modeSelectPin) - modeValues[i]) < 30){
			return i;
		}
	}
}

//Function returns number of pressed key
//If no key was pressed, returns -1
int8_t pressedButton(){
	if(!digitalRead(button1Pin))		return 1;
	else if(!digitalRead(button2Pin))	return 2;
	else if(!digitalRead(button3Pin))	return 3;
	else								return -1;
}

//Returns string type of given int. If int is a single digit, returns it with leading 0
String formatNumber(uint8_t number){
	String result = number < 10 ? "0" + String(number) : String(number);

	return result;
}

//Function for printing date in normal mode
void printDateNormal(){
	float startTime = micros();

	while(micros() - startTime < yearPrintDelay){
		if(interrupt())	return;

		DateTime now = g_RTC.now();

		g_mainDisplay.print(formatNumber(now.day()) + "." + formatNumber(now.month()));
	}

	startTime = micros();

	while(micros() - startTime < yearPrintTime){
		if(interrupt())	return;

		DateTime now = g_RTC.now();
		g_mainDisplay.print(String(now.year()));
	}
}

//Function for printing date in set mode
void printDateSet(DateTime date, float timeOnDisplay, bool printYear, uint8_t activeHalf){
	float startTime = micros();

	if(!printYear){
		while(micros() - startTime < timeOnDisplay){
			g_mainDisplay.print(formatNumber(date.day()) + "." + formatNumber(date.month()), activeHalf);
		}
	}
	else{
		while(micros() - startTime < timeOnDisplay){
			g_mainDisplay.print(String(date.year()));
		}
	}
}

//Function for printing time in normal mode
void printTimeNormal(){
	g_mainDisplay.colonOn();

	float startTime = micros();

	while(micros() - startTime < timePrintTime){
		if(interrupt()){
			g_mainDisplay.colonOff();
			return;
		}

		DateTime now = g_RTC.now();

		g_mainDisplay.print(formatNumber(now.hour()) + formatNumber(now.minute()));
	}

	g_mainDisplay.colonOff();

	printDateNormal();
}

//Function for printing time in set mode
void printTimeSet(DateTime time, float timeOnDisplay, uint8_t activeHalf){
	float startTime = micros();

	while(micros() - startTime < timeOnDisplay){
		g_mainDisplay.print(formatNumber(time.hour()) + formatNumber(time.minute()), activeHalf);
	}
}

void printMessage(String msg, float timeOnDisplay){
	// g_mainDisplay.colonOff();

	float startTime = micros();

	while(micros() - startTime < timeOnDisplay){
		g_mainDisplay.print(msg);
	}
}

void setup(){
	pinMode(modeSelectPin, INPUT);
	pinMode(button3Pin, INPUT_PULLUP);
	pinMode(button2Pin, INPUT_PULLUP);
	pinMode(button1Pin, INPUT_PULLUP);

	g_RTC.begin();

	lastMode = checkMode();
}

void loop(){
	uint8_t currentMode = checkMode();

	//Default clock mode
	if(currentMode == 0){
		printTimeNormal();
	}
	//Set alarm mode
	else if(currentMode == 1){
		DateTime currentAlarm = g_alarm.read();

		//Changing hours by default
		TimeSpan offset = 3600;
		uint8_t activeHalf = 0;

		g_mainDisplay.colonOn();

		while(checkMode() == 1){
			printTimeSet(currentAlarm, 10000, activeHalf);

			if(!digitalRead(button1Pin) && !digitalRead(button3Pin)){
				if(!g_alarm.isActive()){
					g_alarm.set(currentAlarm);
					g_alarm.activate(currentAlarm);

					printMessage("0n", 3000000);
				}
				else{
					g_alarm.deactivate();

					printMessage("0ff", 3000000);
				}
			}
			else{
				switch(pressedButton()){
					case 3:
						offset.totalseconds() == 3600 ? offset = 60 : offset = 3600;
						
						activeHalf++;
						activeHalf %= 2;
	
						printTimeSet(currentAlarm, 250000, activeHalf);
	
						break;
	
					case 1:
						currentAlarm = currentAlarm + offset;
	
						printTimeSet(currentAlarm, 250000, activeHalf);
	
						break;
	
					case 2:
						currentAlarm = currentAlarm - offset;
	
						printTimeSet(currentAlarm, 250000, activeHalf);
	
						break;
				}
			}
		}

		g_mainDisplay.colonOff();

		g_alarm.set(currentAlarm);
	}
	//Set date mode
	else if(currentMode == 2){
		uint8_t currentSetting = 0;

		DateTime date = g_RTC.now();

		uint8_t day = date.day();
		uint8_t month = date.month();
		uint16_t year = date.year();

		while(checkMode() == 2){
			printDateSet(date, 10000, currentSetting == 2, currentSetting);

			switch(pressedButton()){
				case 1:
					switch(currentSetting){
						case 0:
							day++;

							if(day > 31)		day = 1;
							else if(day < 1)	day = 31;

							break;

						case 1:
							month++;

							if(month > 12)		month = 1;
							else if(month < 1)	month = 12;

							break;

						case 2:
							year++;

							break;
					}

					date = DateTime(year, month, day);
					printDateSet(date, 250000, currentSetting == 2, currentSetting);

					break;

				case 2:
					switch(currentSetting){
						case 0:
							day--;

							if(day > 31)		day = 1;
							else if(day < 1)	day = 31;

							break;

						case 1:
							month--;

							if(month > 12)		month = 1;
							else if(month < 1)	month = 12;

							break;

						case 2:
							year--;

							break;
					}

					date = DateTime(year, month, day);
					printDateSet(date, 250000, currentSetting == 2, currentSetting);

					break;

				case 3:
					currentSetting++;
					currentSetting %= 3;

					printDateSet(date, 250000, currentSetting == 2, currentSetting);

					break;
			}
		}

		DateTime currentTime = g_RTC.now();
		g_RTC.adjust(DateTime(year, month, day, currentTime.hour(), currentTime.minute(), currentTime.second()));
	}
	//Set time mode
	else if(currentMode == 3){
		//Freezing time when selecting this mode
		//and reducing seconds to 0
		DateTime time = g_RTC.now();
		time = DateTime(time.year(), time.month(), time.day(), time.hour(), time.minute(), 0);

		//Changing hours by default
		TimeSpan offset = 3600;
		uint8_t activeHalf = 0;

		g_mainDisplay.colonOn();

		while(checkMode() == 3){
			printTimeSet(time, 10000, activeHalf);

			switch(pressedButton()){
				case 1:
					time = time + offset;

					printTimeSet(time, 250000, activeHalf);

					break;

				case 2:
					time = time - offset;

					printTimeSet(time, 250000, activeHalf);

					break;

				case 3:
					offset.totalseconds() == 3600 ? offset = 60 : offset = 3600;

					activeHalf++;
					activeHalf %= 2;

					printTimeSet(time, 250000, activeHalf);

					break;
			}
		}

		g_RTC.adjust(time);
		g_mainDisplay.colonOff();

	}

	//Handle alarm
	if(g_alarm.fired()){
		g_alarm.alarmBuzz();

		alarmInterruptFired = false;
		g_alarm.clear();
	}
}
