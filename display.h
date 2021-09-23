class display{
    private:
        //---Config---
        int printDelays = 500;  //How long each digit will be visible   [us]
        //------

        //---Signs map---
        char signNames[13] = {
            '0',
            '1',
            '2',
            '3',
            '4',
            '5',
            '6',
            '7',
            '8',
            '9',
            'f',
            'n',
            ' '
        };

        byte signCodes[13] = {
            0b00111111,     //0
            0b00000110,     //1
            0b01011011,     //2
            0b01001111,     //3
            0b01100110,     //4
            0b01101101,     //5
            0b01111101,     //6
            0b00000111,     //7
            0b01111111,     //8
            0b01101111,     //9
            0b01110001,     //f
            0b01010100,     //n
            0b00000000      //empty
        };
        //------

        uint8_t signsAmount = sizeof(signNames) / sizeof(signNames[0]);     //Map size

        //Turns one of 4 display cathodes (from 0 to 3) low
        void switchPanel(uint8_t digit){
            DDRB &= 0b11110000;
            DDRB |= (1 << digit);
        }

        //Sets digit segments configuration.
        //Before setting segments configuration,
        //proper cathode should be set (switchPanel)
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

    public:
        //Colon leds are not multiplexed and can be switched independently
        void colonOn(){
            DDRB |= 0b00010000;
        }

        void colonOff(){
            DDRB &= 0b11101111;
        }

        void print(String string, uint8_t activeHalf = 2){
            uint8_t stringSize = string.length();
            uint8_t activePanel = 0;
            uint8_t index = 0;

            while(index < stringSize && activePanel < 4){
                if(string[index] != '.'){
                    bool showDot = false;

                    if(index + 1 < stringSize && string[index + 1] == '.'){
                        showDot = true;
                    }

                    switchPanel(activePanel);
                    setSegments(string[index], showDot);

                    if((activeHalf == 1 && index < 2) || (activeHalf == 0 && index >= 2)){
                        delayMicroseconds(100);
                    }
                    else{
                      delayMicroseconds(printDelays);
                    }
                    
                    clearSegments();

                    activePanel++;
                }

                index++;
            }
        }
};
