#ifndef CHETCH_ADM_TEST01_H
#define CHETCH_ADM_TEST01_H

#include <Arduino.h>
#include <ChetchArduinoDevice.h>
#include <ChetchADMMessage.h>

namespace Chetch{
    class Test01 : public ArduinoDevice {
        private:
            int testValue = 0;
            int incrementTestValue = 0;

        public: 

            Test01(byte id, byte cat, char *dn);

            void configure(ADMMessage* message, ADMMessage* response) override;
            void createMessage(ADMMessage::MessageType messageType, ADMMessage* message) override;

    }; //end class
} //end namespae
#endif