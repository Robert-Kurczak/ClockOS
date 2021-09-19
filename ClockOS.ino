#include <Wire.h>
#include "display.h"
#include "RTClib.h"

#define timePrintTime 60000000 //How long time will be visible, before showing date [us]
#define yearPrintDelay 5000000 //Time after displayed dd.mm will be changed to yyyy [us]
#define yearPrintTime 5000000  //How long yyyy will be displayed                    [us]

#define modeSelectPin A0
#define button1Pin A3
#define button2Pin A2
#define button3Pin A1

display g_mainDisplay;
RTC_DS3231 g_RTC;

uint8_t lastMode;
//Function for detecting user interaction, like pressing key or changing mode
bool interrupt(){
	//---Mode change interrupt---
	uint8_t currentMode = checkMode();

	if(currentMode != lastMode){
		lastMode = currentMode;
		return true;
	}
	//---

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

//Function for printing date in normal mode
void printDateNormal(){
	float startTime = micros();

	while(micros() - startTime < yearPrintDelay){
		if(interrupt())	return;

		DateTime now = g_RTC.now();

		String day = now.day() < 10 ? "0" + String(now.day()) : String(now.day());
		String month = now.month() < 10 ? "0" + String(now.month()) : String(now.month());

		g_mainDisplay.print(day + "." + month);
	}

	startTime = micros();

	while(micros() - startTime < yearPrintTime){
		if(interrupt())	return;

		DateTime now = g_RTC.now();
		g_mainDisplay.print(String(now.year()));
	}
}

//Function for printing date in set mode
void printDateSet(DateTime date, float timeOnDisplay, bool printYear){
	float startTime = micros();

	if(!printYear){
		while(micros() - startTime < timeOnDisplay){
			String day = date.day() < 10 ? "0" + String(date.day()) : String(date.day());
			String month = date.month() < 10 ? "0" + String(date.month()) : String(date.month());

			g_mainDisplay.print(day + "." + month);
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

		String hour = now.hour() < 10 ? "0" + String(now.hour()) : String(now.hour());
		String minute = now.minute() < 10 ? "0" + String(now.minute()) : String(now.minute());

		g_mainDisplay.print(hour + minute);
	}

	g_mainDisplay.colonOff();

	printDateNormal();
}

//Function for printing time in set mode
void printTimeSet(uint32_t unixStamp, float timeOnDisplay){
	float startTime = micros();

	while(micros() - startTime < timeOnDisplay){
		DateTime now(unixStamp);

		String hour = now.hour() < 10 ? "0" + String(now.hour()) : String(now.hour());
		String minute = now.minute() < 10 ? "0" + String(now.minute()) : String(now.minute());

		g_mainDisplay.print(hour + minute);
	}
}

void setup(){
	pinMode(modeSelectPin, INPUT);
	pinMode(A1, button3Pin);
	pinMode(A2, button2Pin);
	pinMode(A3, button1Pin);

	g_RTC.begin();
	// RTC.adjust(DateTime(F(__DATE__), F(__TIME__)));

	lastMode = checkMode();
}

void loop(){
	switch(checkMode()){
		//Default clock mode
		case 0:
			printTimeNormal();

			break;

		//Set time mode
		case 2:
			//Freezing time when selecting this mode
			//and reducing seconds to 0
			uint32_t unixTime = g_RTC.now().unixtime();
			unixTime -= unixTime % 60;

			//Changing hours by default
			int offset = 3600;

			g_mainDisplay.colonOn();

			while(checkMode() == 2){
				printTimeSet(unixTime, 10000);

				switch(pressedButton()){
					case 1:
						unixTime += offset;
						g_RTC.adjust(DateTime(unixTime));

						printTimeSet(unixTime, 250000);

						break;

					case 2:
						unixTime -= offset;
						g_RTC.adjust(DateTime(unixTime));

						printTimeSet(unixTime, 250000);

						break;

					case 3:
						offset == 3600 ? offset = 60 : offset = 3600;
						printTimeSet(unixTime, 250000);

						break;
				}
			}

			g_mainDisplay.colonOff();

			break;

		//Set date mode
		case 1:
			uint8_t currentSetting = 0;

			while(checkMode() == 1){
				DateTime date = g_RTC.now();

				printDateSet(date, 10000, currentSetting == 2);

				uint8_t day, month;

				switch(pressedButton()){
					case 1:
						switch(currentSetting){
							case 0:
								day = date.day() + 1;

								if(day > 31)		day = 1;
								else if(day < 1)	day = 31;

								date = DateTime(date.year(), date.month(), day, date.hour(), date.minute(), date.second());

								break;

							case 1:
								month = date.month() + 1;

								if(month > 12)		month = 1;
								else if(month < 1)	month = 12;

								date = DateTime(date.year(), month, date.day(), date.hour(), date.minute(), date.second());

								break;

							case 2:
								date = DateTime(date.year() + 1, date.month(), date.day(), date.hour(), date.minute(), date.second());

								break;
						}

						g_RTC.adjust(date);

						printDateSet(date, 250000, currentSetting == 2);

						break;

					case 2:
						switch(currentSetting){
							case 0:
								day = date.day() - 1;

								if(day > 31)		day = 1;
								else if(day < 1)	day = 31;

								date = DateTime(date.year(), date.month(), day, date.hour(), date.minute(), date.second());

								break;

							case 1:
								month = date.month() - 1;

								if(month > 12)		month = 1;
								else if(month < 1)	month = 12;

								date = DateTime(date.year(), month, date.day(), date.hour(), date.minute(), date.second());

								break;

							case 2:
								date = DateTime(date.year() - 1, date.month(), date.day(), date.hour(), date.minute(), date.second());

								break;
						}

						g_RTC.adjust(date);

						printDateSet(date, 250000, currentSetting == 2);

						break;

					case 3:
						currentSetting++;
						currentSetting %= 3;

						printDateSet(date, 250000, currentSetting == 2);

						break;
				}
			}

			break;
	
	}
}
