#ifndef CHETCH_ADM_ZMPT101B_H
#define CHETCH_ADM_ZMPT101B_H

#include <Arduino.h>
#include <ChetchArduinoDevice.h>
#include <ChetchADMMessage.h>

namespace Chetch{
    class ZMPT101B : public ArduinoDevice {
        private:
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
            double stableVoltage = -1; //if < 0 then no stabalising is required
            double stabiliseThreshold = 0;
            double voltageLowerBound = 0;
            double voltageUpperBound = -1;
     
        public: 

            ZMPT101B(byte id, byte cat, char *dn);

            void setStableVoltage(double v, double t = 0, double vlb = 0, double vub = -1);
            void loop() override;
            double getVoltage();
            double getHZ();
            bool isVoltageInRange();
            double adjustVoltageBy();
    }; //end class
} //end namespae
#endif