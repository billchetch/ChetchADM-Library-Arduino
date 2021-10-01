#include "ChetchUtils.h"
#include "ChetchZMPT101B.h"

namespace Chetch{
    
    ZMPT101B::ZMPT101B(byte id, byte cat, char *dn) : ArduinoDevice(id, cat, dn){
        //empty
    }


    void ZMPT101B::configure(ADMMessage* message, ADMMessage* response){
        ArduinoDevice::configure(message, response);

        int argIdx = getArgumentIndex(message, MessageField::PIN);
        voltagePin = message->argumentAsByte(argIdx);

        argIdx = getArgumentIndex(message, MessageField::SAMPLE_SIZE);
        sampleSize = message->argumentAsInt(argIdx);

        argIdx = getArgumentIndex(message, MessageField::SAMPLE_INTERVAL);
        sampleInterval = message->argumentAsULong(argIdx);

        setTargetVoltage(
            message->argumentAsDouble(getArgumentIndex(message, MessageField::TARGET_VOLTAGE)),
            message->argumentAsDouble(getArgumentIndex(message, MessageField::TARGET_TOLERANCE)),
            message->argumentAsDouble(getArgumentIndex(message, MessageField::VOLTAGE_LOWER_BOUND)),
            message->argumentAsDouble(getArgumentIndex(message, MessageField::VOLTAGE_UPPER_BOUND))
            );

        //argIdx = getArgumentIndex(message, MessageField::SAMPLE_INTERVAL);
        //sampleInterval = message->argumentAsULong(argIdx);
    }

    int ZMPT101B::getArgumentIndex(ADMMessage *message, ZMPT101B::MessageField field){
        switch(field){
            default:
                return (int)field;
        }
    }

    void ZMPT101B::createMessage(ADMMessage::MessageType messageTypeToCreate, ADMMessage* message){
        ArduinoDevice::createMessage(messageTypeToCreate, message);
        message->addInt(220);
    }

	void ZMPT101B::setTargetVoltage(double v, double t, double vlb, double vub){
        targetVoltage = v;
        targetTolerance = t;
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
        if(targetVoltage <= 0 || !isVoltageInRange())return 0;

        double v = getVoltage();
        double adjustment = targetVoltage - v;
        return abs(adjustment) > targetTolerance ? adjustment : 0;
    } 
} //end namespace
