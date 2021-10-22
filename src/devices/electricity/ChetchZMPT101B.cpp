#include "ChetchUtils.h"
#include "ChetchZMPT101B.h"

namespace Chetch{
    
    ZMPT101B::ZMPT101B(byte id, byte cat, char *dn) : ArduinoDevice(id, cat, dn){
       setTimerTicks(1);
    }


    void ZMPT101B::configure(ADMMessage* message, ADMMessage* response){
        ArduinoDevice::configure(message, response);

        int argIdx = getArgumentIndex(message, MessageField::PIN);
        voltagePin = message->argumentAsByte(argIdx);

        argIdx = getArgumentIndex(message, MessageField::SAMPLE_SIZE);
        sampleSize = message->argumentAsInt(argIdx);

        
        setTargetParameters(
            (Target)message->argumentAsByte(getArgumentIndex(message, MessageField::TARGET)),
            message->argumentAsInt(getArgumentIndex(message, MessageField::TARGET_VALUE)),
            message->argumentAsInt(getArgumentIndex(message, MessageField::TARGET_TOLERANCE)),
            message->argumentAsInt(getArgumentIndex(message, MessageField::TARGET_LOWER_BOUND)),
            message->argumentAsInt(getArgumentIndex(message, MessageField::TARGET_UPPER_BOUND))
            );
        
        pinMode(voltagePin, INPUT);

        response->addByte(target);
        response->addDouble(targetValue);
    }

    int ZMPT101B::getArgumentIndex(ADMMessage *message, ZMPT101B::MessageField field){
        switch(field){
            default:
                return (int)field;
        }
    }

    void ZMPT101B::createMessageToSend(byte messageID, ADMMessage* message){
        ArduinoDevice::createMessageToSend(messageID, message);

        if(messageID == ArduinoDevice::MESSAGE_ID_REPORT){
            message->addDouble(getVoltage());
            message->addDouble(getHz());
        }

        if(messageID == MESSAGE_ID_ADJUSTMENT){
            createMessage(ADMMessage::MessageType::TYPE_WARNING, message);
            message->addDouble(adjustBy());
        }
    }

	void ZMPT101B::setTargetParameters(Target t, double tv, double tt, double tlb, double tub){
        target = t;
        targetValue = tv;
        targetTolerance = tt;
        targetLowerBound = tlb;
        targetUpperBound = tub;
    }                   
    
    void ZMPT101B::loop(){
        //this will flag message to send as DATA for reporting which means if we flag message to create after this it will take precedence
        //so we need to ensure that we restore the exising messageTypeToCreate value so it's picked up on the next loop'
        ArduinoDevice::loop(); 
    
    }

    void ZMPT101B::onTimer(){
        long v = (analogRead(voltagePin) - midPoint);
        summedVoltages += (v * v);
        sampleCount++;

        if((readVoltage < 0 && v >= 0) || (readVoltage > 0 && v <= 0)){
            hzCount++;
        }
        readVoltage = v;
    }



    /*void ZMPT101B::readVoltage(bool verifyInterval){
        //take samples
        unsigned long m = micros();
        if(!verifyInterval || m - lastSampled >= sampleInterval){
            if(sampleCount == 0)startedSampling = micros();

            int readValue = analogRead(voltagePin);
            double v = (double)(readValue - midPoint);
            summedVoltages += sq(v);
            sampleCount++;
            
            lastSampled = m;
    
            //Hz cross over
            int hzCrossOverTolerance = 0;
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
            
            hz = (double)hzCount *( 500000.0 / (double)(micros() - startedSampling));

            //put in history and calculate averages
            voltageHistory[historyIndex] = voltage;
            hzHistory[historyIndex] = hz;
            historyIndex = (historyIndex + 1) % HISTORY_SIZE;
            double voltageTotal = 0;
            double hzTotal = 0;
            int vCount = 0;
            int hCount = 0;
            
            for(int i = 0; i < HISTORY_SIZE; i++){
                if(voltageHistory[i] > 0){
                    voltageTotal += voltageHistory[i];
                    vCount++;
                }
                if(hzHistory[i] > 0){
                    hzTotal += hzHistory[i];
                    hCount++;
                }
            }
            if(vCount > 0)averageVoltage = voltageTotal / (double)vCount;
            if(hCount > 0)averageHz = hzTotal / (double)hCount;
            
            //reset counters etc.
            sampleCount = 0;
            startedSampling = 0;
            summedVoltages = 0;
            hzCount = 0;

            //sample complete so trigger adjustment event
            if(target != Target::NONE && adjustBy() != 0){ 
               enqueueMessageToSend(MESSAGE_ID_ADJUSTMENT);
            }
        }
    } */

    double ZMPT101B::getVoltage(){
        return voltage;
    }

    double ZMPT101B::getHz(){
        return hz;
    }

    double ZMPT101B::getTargetedValue(){
        switch(target){
            case Target::HZ:
                return getHz();
            
            case Target::VOLTAGE:
                return getVoltage();
            
            default:
                return -1;
        }
    }

    bool ZMPT101B::isTargetedValueInRange(){
        if(targetUpperBound <= targetLowerBound)return true;
      
        double v = getTargetedValue();
        return (v >= targetLowerBound && v <= targetUpperBound);
    }
    
    double ZMPT101B::adjustBy(){
        if(targetValue <= 0 || !isTargetedValueInRange())return 0;

        double v = getTargetedValue();
        double adjustment = targetValue - v;
        return abs(adjustment) > targetTolerance ? adjustment : 0;
    } 
} //end namespace
