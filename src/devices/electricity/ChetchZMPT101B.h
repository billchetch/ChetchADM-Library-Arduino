#ifndef CHETCH_ADM_ZMPT101B_H
#define CHETCH_ADM_ZMPT101B_H

#include <Arduino.h>
#include <ChetchArduinoDevice.h>
#include <ChetchADMMessage.h>

namespace Chetch{
    class ZMPT101B : public ArduinoDevice {
        public:
            enum MessageField{
                PIN = 2,
                SAMPLE_SIZE = 3,
                SAMPLE_INTERVAL = 4,
                TARGET_VOLTAGE = 5,
                TARGET_TOLERANCE = 6,
                VOLTAGE_LOWER_BOUND = 7,
                VOLTAGE_UPPER_BOUND = 8,
                SCALE_WAVEFORM = 9,
                FINAL_OFFSET = 10,
            };

        private:
            byte voltagePin = A0;
            double summedVoltages = 0;
            int hzCount = 0;
            unsigned int sampleCount = 0;
            int sampleSize = 500;
            unsigned long lastSampled = 0;
            double lastVoltage = 0;
            unsigned long sampleInterval = 1000; //in micros
            int midPoint = 510;
            double scaleWaveform = 1.61;
            double finalOffset = 2.5;
            double voltage = 0;
            double minVoltage = 10; //below which we reduce to 0
            double maxVoltage = 250; //above which we reduce to maxVoltage
            double hz = 0; 
            double targetVoltage = -1; //if < 0 then no stabalising is required
            double targetTolerance = 0;
            double voltageLowerBound = 0;
            double voltageUpperBound = -1;
     
        public: 

            ZMPT101B(byte id, byte cat, char *dn);

            int getArgumentIndex(ADMMessage *message, MessageField field);
            void configure(ADMMessage* message, ADMMessage* response) override;
            void createMessage(ADMMessage::MessageType messageType, ADMMessage* message) override;

            void setTargetVoltage(double v, double t = 0.0, double vlb = 0.0, double vub = -1.0);
            void loop() override;
            double getVoltage();
            double getHZ();
            bool isVoltageInRange();
            double adjustVoltageBy();
    }; //end class
} //end namespae
#endif