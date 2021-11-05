#ifndef CHETCH_ADM_LCD_H
#define CHETCH_ADM_LCD_H

#include <Arduino.h>
#include <ChetchArduinoDevice.h>
#include <ChetchADMMessage.h>
#include <LiquidCrystal.h>

namespace Chetch{
    class LCD : public ArduinoDevice {
        public:
            enum DisplayDimensions{
                D16x2 = 1,
            };

            enum DataPinSequence{
                Pins_5_2 = 1,
                Pins_2_5 = 2,
            };

            enum MessageField{
                DATA_PINS = 2,
                ENABLE_PIN,
                REGISTER_SELECT_PIN,
                DISPLAY_DIMENSIONS,
                TEXT_TO_PRINT,
                CURSOR_POS_X,
                CURSOR_POS_Y,
            };


        private:
            LiquidCrystal* lcd = NULL;

            DataPinSequence dataPins;
            byte enablePin = 0;
            byte registerSelectPin = 0;
            DisplayDimensions dimensions;
            
        public: 
            
            LCD(byte id, byte cat, char *dn);
            ~LCD() override;

            int getArgumentIndex(ADMMessage *message, MessageField field);

            void configure(ADMMessage* message, ADMMessage* response) override;
            void createMessageToSend(byte messageID, ADMMessage* message) override;
            void loop() override;
            DeviceCommand executeCommand(ADMMessage *message, ADMMessage *response) override;

    }; //end class
} //end namespae
#endif