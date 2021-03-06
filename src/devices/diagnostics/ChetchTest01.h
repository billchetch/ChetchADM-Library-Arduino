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
            unsigned long ms = 0;

        public: 

            volatile unsigned long count = 0;

            Test01(byte id, byte cat, char *dn);

            bool configure(ADMMessage* message, ADMMessage* response) override;
            void populateMessageToSend(byte messageID, ADMMessage* message) override;
            void loop() override;
            

    }; //end class
} //end namespae
#endif