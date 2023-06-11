#ifndef CHETCH_ADM_TICKER_H
#define CHETCH_ADM_TICKER_H

#include <Arduino.h>
#include <ChetchArduinoDevice.h>
#include <ChetchADMMessage.h>

namespace Chetch{
    class Ticker : public ArduinoDevice {
        public:
            enum MessageField{
                PIN = 2,
                PIN_HIGH_DURATION = 3,
                PIN_LOW_DURATION = 4,
            };

        
        private:
            byte pin = 0;
            unsigned int pinHighDuration = 0;
            unsigned long pinHighStartedOn = 0;
            unsigned int pinLowDuration = 0;
            unsigned long pinLowStartedOn;
            unsigned long tickCount = 0;
            bool pinState = LOW;
            
        public: 
            
            Ticker(byte id, byte cat, char *dn);

            int getArgumentIndex(ADMMessage *message, MessageField field);

            bool configure(ADMMessage* message, ADMMessage* response) override;
            void populateMessageToSend(byte messageID, ADMMessage* message) override;
            void loop() override;
            DeviceCommand executeCommand(ADMMessage *message, ADMMessage *response) override;

    }; //end class
} //end namespae
#endif