#ifndef CHETCH_ADM_SWITCH_DEVICE_H
#define CHETCH_ADM_SWITCH_DEVICE_H

#include <Arduino.h>
#include <ChetchArduinoDevice.h>
#include <ChetchADMMessage.h>

namespace Chetch{
    class SwitchDevice : public ArduinoDevice {
        public:
            enum SwitchMode {
                ACTIVE = 1,
                PASSIVE = 2
            };

            enum MessageField{
                MODE = 2,
                PIN = 3,
                PIN_STATE = 4,
                TOLERANCE = 5,
            };


            static const byte MESSAGE_ID_TRIGGERED = 200;

        private:
            SwitchMode mode;
            byte pin = 0;
            bool pinState = false;
            int tolerance = 0;
            unsigned long recording = 0;
            
        public: 
            
            SwitchDevice(byte id, byte cat, char *dn);

            int getArgumentIndex(ADMMessage *message, MessageField field);

            bool configure(ADMMessage* message, ADMMessage* response) override;
            void populateMessageToSend(byte messageID, ADMMessage* message) override;
            void loop() override;
            DeviceCommand executeCommand(ADMMessage *message, ADMMessage *response) override;
            virtual void trigger();

    }; //end class
} //end namespae
#endif