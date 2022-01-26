#include <EEPROM.h>

class alarm{
    private:
        RTC_DS3231& RTC;
        const uint8_t hourAddress;
        const uint8_t minuteAddress;
        const uint8_t isActiveAddress;
        const uint8_t buzzerPin;

    public:

        alarm(RTC_DS3231& RTC, uint8_t hourAddress, uint8_t minuteAddress, uint8_t isActiveAddress, uint8_t buzzerPin)
        : RTC(RTC), hourAddress(hourAddress), minuteAddress(minuteAddress), isActiveAddress(isActiveAddress), buzzerPin(buzzerPin){
            //If EEPROM was never written, write 7:00 as default alarm (inactive)
            if(EEPROM.read(0) == 255 || EEPROM.read(1) == 255){
                EEPROM.put(hourAddress, 7);
                EEPROM.put(minuteAddress, 0);
                EEPROM.put(isActiveAddress, false);
            }
        }

		bool isActive(){
            return EEPROM.read(isActiveAddress);
        }

        void beep(){
            tone(buzzerPin, 2500);
            delay(50);
            noTone(buzzerPin);
        }

        void set(DateTime alarmDate){
            EEPROM.put(hourAddress, alarmDate.hour());
            EEPROM.put(minuteAddress, alarmDate.minute());
        }

        DateTime read(){
            return DateTime(2000, 1, 1, EEPROM.read(hourAddress), EEPROM.read(minuteAddress), 0);
        }

        void activate(){
            RTC.setAlarm1(this->read(), DS3231_A1_Hour);

            EEPROM.put(isActiveAddress, true);
        }

        void activate(DateTime alarmDate){
            RTC.setAlarm1(alarmDate, DS3231_A1_Hour);

            EEPROM.put(isActiveAddress, true);
        }

        void deactivate(){
            RTC.disableAlarm(1);

            EEPROM.put(isActiveAddress, false);
        }

        bool fired(){
            return RTC.alarmFired(1);
        }

        void clear(){
            RTC.clearAlarm(1);
        }
};
