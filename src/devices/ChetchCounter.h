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
            enum MessageField{
                PIN = 2,
                INTERRUPT_MODE,
                TOLERANCE,
                PIN_STATE_TO_COUNT,
            };

            enum class ErrorSubCode{
                FAILED_INTTERUPT_MODE = 101, //check pin is interrupt possible pin (see CINterrupt)
            };

            static volatile unsigned long handleInterruptCount;

        private:
            static const byte MAX_INSTANCES = 4;
            static byte instanceCount;
            static Counter* instances[MAX_INSTANCES];
            
            byte instanceIndex = 0; //passed to interrupt
            byte pin = 0;
            byte interruptMode = 0; //can be RISING, FALLING, CHANGE (0 for no interrupt)
            volatile uint8_t * inreg;
            uint8_t bitMask;

            bool pinStateToCount = LOW;
            unsigned long tolerance = 0; //in millis
            bool countStarted = false;
            unsigned long countStartedOn = 0; //in micros as when count started
            volatile unsigned long firstCountOn = 0; //in micros when the first interrupt fired
            volatile unsigned long lastCountOn = 0; //in micros when the last interrupt fired
            volatile unsigned long count = 0;
            volatile unsigned long countedOn = 0;
            volatile bool pinState = LOW;
            
     
        public: 
            static void handleInterrupt(uint8_t pin, uint8_t tag);
            static Counter* create(byte id, byte cat, char *dn);
            

            Counter(byte id, byte cat, char *dn);
            ~Counter() override;

            void setInstanceIndex(byte idx);
            void setPin(byte pin);
            bool setInterruptMode(byte mode);
            int getArgumentIndex(ADMMessage *message, MessageField field);
            bool configure(ADMMessage* message, ADMMessage* response) override;
            void status(ADMMessage* message, ADMMessage* response) override;
            void enable(bool enable = true) override;
            void populateMessageToSend(byte messageID, ADMMessage* message) override;
            void loop() override;
            
            void onInterrupt();
            unsigned long getCount();
    }; //end class
} //end namespae
#endif