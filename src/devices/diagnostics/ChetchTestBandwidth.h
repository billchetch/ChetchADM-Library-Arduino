#ifndef CHETCH_ADM_TESTBW_H
#define CHETCH_ADM_TESTBW_H

#include <Arduino.h>
#include <ChetchArduinoDevice.h>
#include <ChetchADMMessage.h>
#include <ChetchADM.h>

namespace Chetch{
    class TestBandwidth : public ArduinoDevice {
        public:
            enum MessageField{
                ACTIVITY_PIN = 2,
            };

        private:
            int activityPin = -1;
            bool activatePin;
            bool pinState;
            unsigned long pinActivatedOn = 0;
            unsigned long messagesReceived = 0;
            unsigned long messagesSent = 0;
            
        public: 
            TestBandwidth(byte id, byte cat, char *dn);

            int getArgumentIndex(ADMMessage *message, MessageField field);

            bool configure(ADMMessage* message, ADMMessage* response) override;
            void loop() override;
            void status(ADMMessage *message, ADMMessage *response) override;
            DeviceCommand executeCommand(ADMMessage *message, ADMMessage *response) override;

            void populateMessageToSend(byte messageID, ADMMessage* message) override;
            void receiveMessage(ADMMessage *message, ADMMessage *response) override;

    }; //end class
} //end namespae
#endif