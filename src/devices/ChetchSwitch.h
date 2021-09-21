#ifndef CHETCH_ADM_SWITCH_H
#define CHETCH_ADM_SWITCH_H

#include <Arduino.h>
#include <ChetchArduinoDevice.h>
#include <ChetchADMMessage.h>

namespace Chetch{
    class Switch : public ArduinoDevice {
        public:
            enum Mode {
                ACTIVE = 1,
                PASSIVE = 2
            };

            enum MessageField{
                MODE = 2,
                PIN = 3,
                INITIAL_STATE = 4,
            };

        private:
            Mode mode;
            byte pin = 0;
            bool state = true;
            unsigned long recording = 0;
            int tolerance = 0;

        public: 
            
            Switch(byte id, byte cat, char *dn);

            int getArgumentIndex(ADMMessage *message, MessageField field);

            void configure(ADMMessage* message, ADMMessage* response) override;
            void createMessage(ADMMessage::MessageType messageType, ADMMessage* message) override;
            void loop() override;
            DeviceCommand executeCommand(ADMMessage *message, ADMMessage *response) override;
            virtual void trigger();

    }; //end class
} //end namespae
#endif