#ifndef CHETCH_ADM_COUNTER_H
#define CHETCH_ADM_COUNTER_H

#include <Arduino.h>
#include <ChetchArduinoDevice.h>
#include <ChetchADMMessage.h>

namespace Chetch{
    /*
    * Important:  The ZMPT101B can flatten the top and bottom of the wave form if the adjustment screw gives too high a reading.
    * It is necessary to inspect the waveform (half-second intervals are good) and reduce the screw setting so as\ to get a smooth 
    * 'wave' and then scale back up to get the desired values using the 'scaleWaveform' value.
    * 
    */
    class Counter : public ArduinoDevice {
        public:
            enum InterruptMode{
                IM_NONE = 0,
                IM_RISING = 3,
                IM_FALLING = 2,
                IM_CHANGE = 1,
            };

            enum MessageField{
                PIN = 2,
                INTERRUPT_MODE,
            };

            byte pin = 0;

        private:
            static const byte MAX_INSTANCES = 4;
            static byte instanceCount;
            static Counter* instances[MAX_INSTANCES];
            
            InterruptMode interruptMode = InterruptMode::IM_NONE;
            unsigned long count = 0;
            
     
        public: 
            static void handleInterrupt(uint8_t pin, uint8_t tag);
            static Counter* create(byte id, byte cat, char *dn);
            

            Counter(byte id, byte cat, char *dn);
            ~Counter() override;

            int getArgumentIndex(ADMMessage *message, MessageField field);
            bool configure(ADMMessage* message, ADMMessage* response) override;
            void status(ADMMessage* message, ADMMessage* response) override;
            void createMessageToSend(byte messageID, ADMMessage* message) override;
            void loop() override;
            void onInterrupt();
    }; //end class
} //end namespae
#endif