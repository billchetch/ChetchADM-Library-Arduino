#ifndef CHETCH_ADM_ZMPT101B_H
#define CHETCH_ADM_ZMPT101B_H

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
    class ZMPT101B : public ArduinoDevice {
        public:
            enum Target{
                NONE = 0,
                VOLTAGE = 1,
                HZ = 2,
            };

            enum MessageField{
                PIN = 2,
                SAMPLE_SIZE = 3,
                SAMPLE_INTERVAL = 4,
                TARGET = 5,
                TARGET_VALUE = 6,
                TARGET_TOLERANCE = 7,
                TARGET_LOWER_BOUND = 8,
                TARGET_UPPER_BOUND = 9,
                SCALE_WAVEFORM = 10,
                FINAL_OFFSET = 11,
            };

            static const byte MESSAGE_ID_ADJUSTMENT = 200;

        public:
            byte voltagePin = 14;
            double summedVoltages = 0;
            int hzCount = 0;
            unsigned int sampleCount = 0;
            int sampleSize = 1000;
            unsigned long lastSampled = 0;
            double lastVoltage = 0;
            unsigned long sampleInterval = 500; //in micros ... Use a value of 500,000 to get a good shaped output in the Serial Plotter
            int midPoint = 510;
            double scaleWaveform = 1.5;
            double finalOffset = 2.5;
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

            ZMPT101B(byte id, byte cat, char *dn);

            int getArgumentIndex(ADMMessage *message, MessageField field);
            void configure(ADMMessage* message, ADMMessage* response) override;
            void createMessageToSend(byte messageID, ADMMessage* message) override;

            void setTargetParameters(Target t, double tv, double tt = 0.0, double tlb = 0.0, double tub = -1.0);
            void loop() override;
            double getVoltage();
            double getHz();
            double getTargetedValue();
            bool isTargetedValueInRange();
            double adjustBy(); 
    }; //end class
} //end namespae
#endif