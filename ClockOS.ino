#include <Wire.h>
#include "display.h"
#include "RTClib.h"
#include "alarm.h"

#define timePrintTime 600000			//How long time will be visible, before showing date	[ms]
#define yearPrintDelay 5000				//Time after displayed dd.mm will be changed to yyyy	[ms]
#define yearPrintTime 5000 				//How long yyyy will be displayed                    	[ms]
#define temperaturePrintTime 5000		//How long temperature will be displayed               	[ms]
#define checkRTCdelays 400				//Time between checks of actually time stored in RTC	[ms]

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
	unsigned long startTime = millis();

	DateTime now = g_RTC.now();

	while(millis() - startTime < yearPrintDelay){
		if(interrupt())	return;

		g_mainDisplay.print(formatNumber(now.day()) + "." + formatNumber(now.month()));
	}

	startTime = millis();

	while(millis() - startTime < yearPrintTime){
		if(interrupt())	return;

		g_mainDisplay.print(String(now.year()));
	}
}

//Function for printing date in set mode
void printDateSet(DateTime date, float timeOnDisplay, bool printYear, uint8_t activeHalf){
	unsigned long startTime = millis();

	//Print ddmm
	if(!printYear){
		//Instead of printing for some period of time, print just once
		if(timeOnDisplay == -1){
			g_mainDisplay.print(formatNumber(date.day()) + "." + formatNumber(date.month()), activeHalf);
		}
		else{
			while(millis() - startTime < timeOnDisplay){
				g_mainDisplay.print(formatNumber(date.day()) + "." + formatNumber(date.month()), activeHalf);

				//This function have lower complexity than printTimeNormal
				//therefore printed time by this function is brighter than printTimeNormal one.
				//To compensate it I use delay so the time print can be consistant.
				delayMicroseconds(750);
			}
		}
	}
	//Print yyyy
	else{
		//Instead of printing for some period of time, print just once
		if(timeOnDisplay == -1){
			g_mainDisplay.print(String(date.year()));
		}
		else{
			while(millis() - startTime < timeOnDisplay){
				g_mainDisplay.print(String(date.year()));
			}
		}
	}
}

//Function for printing time in normal mode
//Problem with display freezing probably comes from here
void printTimeNormal(){
	g_mainDisplay.colonOn();

	unsigned long startTime = millis();

	DateTime now = g_RTC.now();
	unsigned long checkRTCtimer = millis();

	while(millis() - startTime < timePrintTime){
		if(interrupt(true)){
			g_mainDisplay.colonOff();
			return;
		}

		//Reading from RTC is time costly and it results in lower display brightness,
		//therefore I check this in predefined time spans
		if(millis() - checkRTCtimer >= checkRTCdelays){
			now = g_RTC.now();

			checkRTCtimer = millis();
		}

		g_mainDisplay.print(formatNumber(now.hour()) + formatNumber(now.minute()));
	}

	g_mainDisplay.colonOff();

	printDateNormal();
}

//Function for printing time in set mode
void printTimeSet(DateTime time, float timeOnDisplay, uint8_t activeHalf){
	unsigned long startTime = millis();

	//Instead of printing for some period of time, print just once
	if(timeOnDisplay == -1){
		g_mainDisplay.print(formatNumber(time.hour()) + formatNumber(time.minute()), activeHalf);
	}
	else{
		while(millis() - startTime < timeOnDisplay){
			g_mainDisplay.print(formatNumber(time.hour()) + formatNumber(time.minute()), activeHalf);

			//This function have lower complexity than printTimeNormal
			//therefore printed time by this function is brighter than printTimeNormal one.
			//To compensate it I use delay so the time print can be consistant.
			delayMicroseconds(750);
		}
	}
}

void printTemperature(){
	bool wasColonOn = g_mainDisplay.activeColon;

	if(wasColonOn){
		g_mainDisplay.colonOff();
	}

	uint8_t temperature = g_RTC.getTemperature();
	String result;
	
	if(temperature < 0 || temperature > 99){
		result = "----";
	}
	else{
		result = String(temperature) + "*C";
	}

	unsigned long startTime = millis();

	while(millis() - startTime < temperaturePrintTime){
		if(interrupt()) return;
		g_mainDisplay.print(result);
	}

	if(wasColonOn){
		g_mainDisplay.colonOn();
	}
}

void printMessage(String msg, float timeOnDisplay){
	bool wasColonOn = g_mainDisplay.activeColon;

	if(wasColonOn){
		g_mainDisplay.colonOff();
	}

	unsigned long startTime = millis();

	while(millis() - startTime < timeOnDisplay){
		g_mainDisplay.print(msg);
	}

	if(wasColonOn){
		g_mainDisplay.colonOn();
	}
}

void alarmPrint(uint64_t ms){
	g_mainDisplay.colonOn();

	unsigned long startTime = millis();

	DateTime now = g_RTC.now();

	while(millis() - startTime < ms){
		if(interrupt(true)){
			return;
		}
		
		g_mainDisplay.print(formatNumber(now.hour()) + formatNumber(now.minute()));
	}

	g_mainDisplay.colonOff();
}

void alarmBuzz(){
	while(!interrupt(true)){
		g_alarm.beep();

		alarmPrint(40);

		g_alarm.beep();

		alarmPrint(160);
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
		switch(pressedButton()){
			case -1:
				printTimeNormal();
				break;

			case 1:
				printDateNormal();
				break;
			
			case 2:
				printTemperature();
				break;

			case 3:
				g_mainDisplay.colonOn();

				g_mainDisplay.currentDelayMode++;
				g_mainDisplay.currentDelayMode %= 4;

				//Delay so the button wouldn't change brightness every processor tick
				printTimeSet(g_RTC.now(), 250, 2);

				g_mainDisplay.colonOff();
				break;
		}
	}
	//Set alarm mode
	else if(currentMode == 1){
		DateTime currentAlarm = g_alarm.read();

		//Changing hours by default
		TimeSpan offset = 3600;
		uint8_t activeHalf = 0;

		bool changes = false;

		g_mainDisplay.colonOn();

		while(checkMode() == 1){
			printTimeSet(currentAlarm, -1, activeHalf);

			if(!digitalRead(button1Pin) && !digitalRead(button3Pin)){
				if(!g_alarm.isActive()){
					g_alarm.set(currentAlarm);
					g_alarm.activate(currentAlarm);

					printMessage("0n", 3000);
				}
				else{
					g_alarm.deactivate();

					printMessage("0ff", 3000);
				}
			}
			else{
				switch(pressedButton()){
					case 1:
						currentAlarm = currentAlarm + offset;
	
						printTimeSet(currentAlarm, 250, activeHalf);
						
						if(!changes) changes = true;
	
						break;
	
					case 2:
						currentAlarm = currentAlarm - offset;
	
						printTimeSet(currentAlarm, 250, activeHalf);

						if(!changes) changes = true;
	
						break;

					case 3:
						offset.totalseconds() == 3600 ? offset = 60 : offset = 3600;
						
						activeHalf++;
						activeHalf %= 2;
	
						printTimeSet(currentAlarm, 250, activeHalf);
	
						break;
				}
			}
		}

		g_mainDisplay.colonOff();

		if(changes){
			g_alarm.set(currentAlarm);

			//If alarm is in active mode and changes were made,
			//RTC alarm have to be overwritten with new time.
			if(g_alarm.isActive()){
				g_alarm.activate();
			}
		}
	}
	//Set date mode
	else if(currentMode == 2){
		uint8_t currentSetting = 0;

		DateTime date = g_RTC.now();

		uint8_t day = date.day();
		uint8_t month = date.month();
		uint16_t year = date.year();

		bool changes = false;

		while(checkMode() == 2){
			printDateSet(date, -1, currentSetting == 2, currentSetting);

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
					printDateSet(date, 250, currentSetting == 2, currentSetting);

					if(!changes) changes = true;

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
					printDateSet(date, 250, currentSetting == 2, currentSetting);

					if(!changes) changes = true;

					break;

				case 3:
					currentSetting++;
					currentSetting %= 3;

					printDateSet(date, 250, currentSetting == 2, currentSetting);

					break;
			}
		}

		DateTime currentTime = g_RTC.now();

		if(changes){
			g_RTC.adjust(DateTime(year, month, day, currentTime.hour(), currentTime.minute(), currentTime.second()));
		}
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
			printTimeSet(time, -1, activeHalf);

			switch(pressedButton()){
				case 1:
					time = time + offset;

					printTimeSet(time, 250, activeHalf);

					break;

				case 2:
					time = time - offset;

					printTimeSet(time, 250, activeHalf);

					break;

				case 3:
					offset.totalseconds() == 3600 ? offset = 60 : offset = 3600;

					activeHalf++;
					activeHalf %= 2;

					printTimeSet(time, 250, activeHalf);

					break;
			}
		}

		g_RTC.adjust(time);
		g_mainDisplay.colonOff();

	}

	//Handle alarm
	if(g_alarm.fired()){
		alarmBuzz();

		alarmInterruptFired = false;
		g_alarm.clear();
	}
}
