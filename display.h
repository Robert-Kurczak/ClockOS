class display{
    private:
        //---Config---
        int printDelays = 200;          //How long each digit will be visible [us]
        //------

        //---Signs map---
        char signNames[10] = {
            '0',
            '1',
            '2',
            '3',
            '4',
            '5',
            '6',
            '7',
            '8',
            '9'
        };

        byte signCodes[10] = {
            0b00111111,   //0
            0b00000110,   //1
            0b01011011,   //2
            0b01001111,   //3
            0b01100110,   //4
            0b01101101,   //5
            0b01111101,   //6
            0b00000111,   //7
            0b01111111,   //8
            0b01101111    //9
        };
        //------

        uint8_t signsAmount = sizeof(signNames) / sizeof(signNames[0]);     //Map size

        //Turns one of 4 display cathodes (from 0 to 3) low
        void switchDigit(uint8_t digit){
            DDRB &= 0b11110000;
            DDRB |= (1 << digit);
        }

        //Sets digit segments configuration.
        //Before setting segments configuration,
        //proper cathode should be set (switchDigit)
        void setSegments(char sign, bool setDot){
            byte segmentsConfig;

            for(uint8_t i = 0; i < signsAmount; i++){
                if(signNames[i] == sign){
                    segmentsConfig = signCodes[i];
                    break;
                }
            }

            if(setDot){
                segmentsConfig |= 0b10000000;
            }

            DDRD = segmentsConfig;
        }

        //Clears currently segments configuration.
        //Before switching to other digit (changing cathode)
        //this function should be called, so previously displayed config
        //won't apear for a short period of time, before proper one.
        //This would result in halfly dimmed segments of previous configuration.
        void clearSegments(){
            DDRD = 0b00000000;
        }

        void colonOn(){
            DDRB &= 0b11101111;
        }

        void colonOff(){
            DDRB |= 0b00010000;
        }

    public:
        void printTime(String hhmm){
            colonOn();

            for(uint8_t i = 0; i < 4; i++){
                switchDigit(i);
                setSegments(hhmm[i], false);

                delayMicroseconds(printDelays);

                clearSegments();
            }

            colonOff();
        }

        // void printDate(String ddmmyyyy){
        //     unsigned long startTime = micros();

        //     while(micros() - startTime < yearPrintDelay * 1000000){
        //         for(uint8_t i = 0; i < 4; i++){
        //             switchDigit(i);
        //             setSegments(ddmmyyyy[i], i == 1);
                    
        //             delayMicroseconds(printDelays);

        //             clearSegments();
        //         }
        //     }

        //     startTime = micros();

        //     while(micros() - startTime < yearPrintTime * 1000000){
        //         for(uint8_t i = 0; i < 4; i++){
        //             switchDigit(i);
        //             setSegments(ddmmyyyy[i + 4], false);

        //             delayMicroseconds(printDelays);

        //             clearSegments();
        //         }
        //     }
        // }
};