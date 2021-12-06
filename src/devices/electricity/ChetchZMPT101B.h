#ifndef CHETCH_ADM_ZMPT101B_H
#define CHETCH_ADM_ZMPT101B_H

#include <Arduino.h>
#include <ChetchArduinoDevice.h>
#include <ChetchADMMessage.h>
#include <ChetchISRTimer.h>

namespace Chetch{
    /*
    * Important:  The ZMPT101B can flatten the top and bottom of the wave form if the adjustment screw gives too high a reading.
    * It is necessary to inspect the waveform (half-second intervals are good) and reduce the screw setting so as\ to get a smooth 
    * 'wave' and then scale back up to get the desired values using the 'scaleWaveform' value.
    * 
    */
    class ZMPT101B : public ArduinoDevice {
        public:
            enum Target{
                NONE = 0,
                VOLTAGE = 1,
                HZ = 2,
            };

            enum MessageField{
                PIN = 2,
                SAMPLE_SIZE,
                TARGET,
                TARGET_VALUE,
                TARGET_TOLERANCE,
                TARGET_LOWER_BOUND,
                TARGET_UPPER_BOUND,
                SCALE_WAVEFORM,
                FINAL_OFFSET,
            };

            static const byte MESSAGE_ID_ADJUSTMENT = 200;
            static const byte BUFFER_SIZE = 128;
            static const byte MAX_INSTANCES = 2;

        public: //TODO make private
            static ISRTimer* timer;
            static byte instanceIndex;
            static byte currentInstance; //each time an ISR is fired this updates so as to read the next instance voltage
            static ZMPT101B* instances[];
            
            byte voltagePin = A0;
            
            volatile int buffer[BUFFER_SIZE];
            volatile byte bufferIdx = 0;
            volatile bool sampling = false; //set to true in ISR
            volatile byte maxBufferIdx = 0;
            
            unsigned long sampleCount = 0;
            unsigned long summedVoltages = 0;
            unsigned long hzCount = 0;
            
            unsigned long sampleSize = 2000;
           
            int midPoint = 512;
            double scaleWaveform = 1.34;
            double finalOffset = 0; //2.5;
            double voltage = 0;
            double minVoltage = 10; //below which we reduce to 0
            double maxVoltage = 250; //above which we reduce to maxVoltage
            double hz = 0; 
            
            
            Target target = Target::NONE;
            double targetValue = -1; //if < 0 then no stabalising/adjustment is required
            double targetTolerance = 0;
            double targetLowerBound = 0;
            double targetUpperBound = -1;
            
     
        public: 
            static ZMPT101B* create(byte id, byte cat, char *dn);
            static void handleTimerInterrupt();

            ZMPT101B(byte id, byte cat, char *dn);

            int getArgumentIndex(ADMMessage *message, MessageField field);
            bool configure(ADMMessage* message, ADMMessage* response) override;
            void status(ADMMessage* message, ADMMessage* response) override;
            void createMessageToSend(byte messageID, ADMMessage* message) override;
            void setVoltagePin(byte pin);
            void setTargetParameters(Target t, double tv, double tt = 0.0, double tlb = 0.0, double tub = -1.0);
            void loop() override;
            void onAnalogRead(uint16_t v);
            double getVoltage();
            double getHz();
            char *getSummary();
            double getTargetedValue();
            bool isTargetedValueInRange();
            double adjustBy(); 
    }; //end class
} //end namespae
#endif