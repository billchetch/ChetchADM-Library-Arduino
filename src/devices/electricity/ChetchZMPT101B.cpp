#include "ChetchUtils.h"
#include "ChetchZMPT101B.h"

namespace Chetch{
    
    ZMPT101B::ZMPT101B(byte id, byte cat, char *dn) : ArduinoDevice(id, cat, dn){
        //empty
    }


    int ZMPT101B::configure(ADMMessage* message, ADMMessage* response){
        int argIdx = ArduinoDevice::configure(message, response);

        voltagePin = message->argumentAsInt(argIdx++);
        sampleSize = message->argumentAsInt(argIdx++);
        sampleInterval = message->argumentAsULong(argIdx++);

        setStableVoltage(
            message->argumentAsDouble(argIdx++),
            message->argumentAsDouble(argIdx++),
            message->argumentAsDouble(argIdx++),
            message->argumentAsDouble(argIdx++)
            );
    }

    void ZMPT101B::createMessage(ADMMessage::MessageType messageTypeToCreate, ADMMessage* message){
        ArduinoDevice::createMessage(messageTypeToCreate, message);
        message->addInt(120);
    }

	void ZMPT101B::setStableVoltage(double v, double t, double vlb, double vub){
        stableVoltage = v;
        stabiliseThreshold = t;
        voltageLowerBound = vlb;
        voltageUpperBound = vub;
    }                   
    
    void ZMPT101B::loop(){
        ArduinoDevice::loop();
        
        //take samples
        unsigned long m = micros();
        if(m - lastSampled >= sampleInterval){
            double v = (double)(analogRead(voltagePin) - midPoint);
            summedVoltages += sq(v);
            sampleCount++;
            lastSampled = m;
    
            //Hz cross over
            if((lastVoltage < 0 && v >= 0) || (lastVoltage > 0 && v <= 0)){
                hzCount++;
            }
            lastVoltage = v;
        }
    
        //combine samples for final values
        if(sampleCount >= sampleSize){
            voltage = (sqrt(summedVoltages/(double)sampleCount) * scaleWaveform) + finalOffset;
            if(voltage < minVoltage)voltage = 0;
            if(voltage > maxVoltage)voltage = maxVoltage;
        
            //Serial.print("Sum / Count:"); Serial.print(summedVoltages); Serial.print(" / "); Serial.println(sampleCount);
            //Serial.print("V: "); Serial.println(voltage);
        
            hz = (double)hzCount *( 500000.0 / (double)(sampleCount * sampleInterval));
            //Serial.print("Hz count: "); Serial.println(hzCount);
            //Serial.print("Hz: "); Serial.println(hz);
            sampleCount = 0;
            summedVoltages = 0;
            hzCount = 0;
        }
    }

    double ZMPT101B::getVoltage(){
        return voltage;
    }

    double ZMPT101B::getHZ(){
        return hz;
    }

    bool ZMPT101B::isVoltageInRange(){
        if(voltageUpperBound <= voltageLowerBound)return true;
      
        double v = getVoltage();
        return (v >= voltageLowerBound && v <= voltageUpperBound);
    }
    
    double ZMPT101B::adjustVoltageBy(){
        if(stableVoltage <= 0 || !isVoltageInRange())return 0;

        double v = getVoltage();
        double adjustment = stableVoltage - v;
        return abs(adjustment) > stabiliseThreshold ? adjustment : 0;
    } 
} //end namespace
