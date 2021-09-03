#ifndef CHETCH_ADM_ZMPT101B_H
#define CHETCH_ADM_ZMPT101B_H

#include <Arduino.h>
#include <ChetchArduinoDevice.h>
#include <ChetchADMMessage.h>

namespace Chetch{
    class Test01 : public ArduinoDevice {
        private:
            
     
        public: 

            Test01(byte id, byte cat, char *dn);

            int configure(ADMMessage* message, ADMMessage* response) override;
            void createMessage(ADMMessage::MessageType messageType, ADMMessage* message) override;

    }; //end class
} //end namespae
#endif