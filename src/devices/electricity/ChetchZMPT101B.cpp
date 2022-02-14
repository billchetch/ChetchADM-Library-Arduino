#include "ChetchUtils.h"
#include "ChetchADC.h"
#include "ChetchZMPT101B.h"

#if defined(ARDUINO_AVR_MEGA2560)
    #define TIMER_NUMBER 4
    #define TIMER_PRESCALER 8
    #define TIMER_COMPARE_A 499 //calculate micros from this and8 prescaler
#else
    #define TIMER_NUMBER 0
    #define TIMER_PRESCALER 0
    #define TIMER_COMPARE_A 0 //calculate micros from this and8 prescaler
#endif


namespace Chetch{

#if TIMER_NUMBER == 4
    ISR(TIMER4_COMPA_vect){
        ZMPT101B::handleTimerInterrupt();
    }
#endif

    ISRTimer* ZMPT101B::timer = NULL;
    byte ZMPT101B::instanceCount = 0;
    byte ZMPT101B::currentInstance = 0; //current instance for reading ISR
    ZMPT101B* ZMPT101B::instances[ZMPT101B::MAX_INSTANCES];
    
    ZMPT101B* ZMPT101B::create(byte id, byte cat, char *dn){
        if(instanceCount >= MAX_INSTANCES || TIMER_NUMBER <= 0){
            return NULL;
        } else {
            if(instanceCount == 0){
                cli();
                timer = ISRTimer::create(TIMER_NUMBER, TIMER_PRESCALER, ISRTimer::TimerMode::COMPARE);
                timer->setCompareA(0, TIMER_COMPARE_A);
                sei();

                CADC::init(false); //turn off trigger interrupt

                for(byte i = 0; i < MAX_INSTANCES; i++){
                    instances[i] = NULL;
                }
            }

            byte idx = 0;
            for(byte i = 0; i < MAX_INSTANCES; i++){
                if(instances[i] == NULL){
                    idx = i;
                    break;
                }
            }

            ZMPT101B* instance = new ZMPT101B(id, cat, dn);
            instance->setInstanceIndex(idx);
            instances[idx] = instance;
            instanceCount++;
            return instance;
        }
    }

    void ZMPT101B::handleTimerInterrupt(){
        static ZMPT101B* zmpt = instances[currentInstance];
        if(zmpt->sampling && !CADC::isReading()){
            zmpt->onAnalogRead(CADC::readResult());
            currentInstance = (currentInstance + 1) % instanceCount; 
        } else {
            zmpt->sampling = true;
        }
        CADC::startRead(zmpt->voltagePin);
    }


    ZMPT101B::ZMPT101B(byte id, byte cat, char *dn) : ArduinoDevice(id, cat, dn){
        for(byte i = 0; i < BUFFER_SIZE; i++){
            buffer[i] = 0;
        }
    }

    ZMPT101B::~ZMPT101B(){
        instances[instanceIndex] = NULL;
        instanceCount--;
    }

    void ZMPT101B::setInstanceIndex(byte idx){
        instanceIndex = idx;
    }

    void ZMPT101B::setVoltagePin(byte pin){
        voltagePin = pin;
        pinMode(voltagePin, INPUT);
    }

    bool ZMPT101B::configure(ADMMessage* message, ADMMessage* response){
        if(!ArduinoDevice::configure(message, response))return false;

        int argIdx = getArgumentIndex(message, MessageField::PIN);
        setVoltagePin( message->argumentAsByte(argIdx));

        argIdx = getArgumentIndex(message, MessageField::SAMPLE_SIZE);
        sampleSize = message->argumentAsInt(argIdx);

        target = (Target)message->argumentAsByte(getArgumentIndex(message, MessageField::TARGET));
        if(target != Target::NONE){
            setTargetParameters(
                target,
                message->argumentAsInt(getArgumentIndex(message, MessageField::TARGET_VALUE)),
                message->argumentAsInt(getArgumentIndex(message, MessageField::TARGET_TOLERANCE)),
                message->argumentAsInt(getArgumentIndex(message, MessageField::TARGET_LOWER_BOUND)),
                message->argumentAsInt(getArgumentIndex(message, MessageField::TARGET_UPPER_BOUND))
                );
        }

        
        response->addByte(target);
        response->addDouble(targetValue);

        return true;
    }

    void ZMPT101B::status(ADMMessage *message, ADMMessage *response){
        ArduinoDevice::status(message, response);

        response->addByte(voltagePin);
    }

    int ZMPT101B::getArgumentIndex(ADMMessage *message, ZMPT101B::MessageField field){
        switch(field){
            default:
                return (int)field;
        }
    }

    void ZMPT101B::populateMessageToSend(byte messageID, ADMMessage* message){
        ArduinoDevice::populateMessageToSend(messageID, message);

        if(messageID == ArduinoDevice::MESSAGE_ID_REPORT){
            message->addDouble(getVoltage());
            message->addDouble(getHz());
        }

        if(messageID == MESSAGE_ID_ADJUSTMENT){
            populateMessage(ADMMessage::MessageType::TYPE_WARNING, message);
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
        ArduinoDevice::loop(); 
        if(timer == NULL)return;
        if(!timer->isEnabled() && isReady()){
            timer->enable();
            return;
        }

        static unsigned long hzStarted = 0;
        static unsigned long hzFinished = 0;
        static long readVoltage = 0;
        
        //read data from buffer
        for(byte i = 0; i < BUFFER_SIZE; i++){
            if(buffer[i] == 0)break;
            long v = buffer[i] - midPoint;
            summedVoltages += (v * v);
            if((readVoltage < 0 && v >= 0) || (readVoltage > 0 && v <= 0)){
                if(hzCount == 0)hzStarted = micros();
                hzFinished = micros();
                hzCount++;
            }

            sampleCount++;
            readVoltage = v;
            buffer[i] = 0;
        }
        
        bufferIdx = 0;

        //now if we have enough samples generate a RMS voltage
        if(sampleCount >= sampleSize){
            double sv = (double)summedVoltages;
            double sc = (double)sampleCount;
            double vt = (sqrt(sv/sc) * scaleWaveform) + finalOffset;
            voltage = 0.5*voltage + 0.5*vt;
            if(voltage < minVoltage)voltage = 0;
            if(voltage > maxVoltage)voltage = maxVoltage;
            
            if(voltage > 0){
                double sampleDuration = ((double)(hzFinished - hzStarted)); 
                hz = sampleDuration > 0 ? ((double)(hzCount - 1.0) * 500000.0 / sampleDuration) : 0;
            } else {
                hz = 0;
            }
            summedVoltages = 0;
            sampleCount = 0; 
            hzCount = 0;
            hzStarted = 0;
            readVoltage = 0;

            //adjusment
            /*if(target != Target::NONE && adjustBy() != 0){ 
               enqueueMessageToSend(MESSAGE_ID_ADJUSTMENT);
            }*/
        }  
    }

    void ZMPT101B::onAnalogRead(uint16_t v){
        buffer[bufferIdx] = v;
        bufferIdx = (bufferIdx + 1) % BUFFER_SIZE;
        //temp for debugging ... remove!
        if(bufferIdx > maxBufferIdx)maxBufferIdx = bufferIdx;
    }

    double ZMPT101B::getVoltage(){
        return voltage;
    }

    double ZMPT101B::getHz(){
        return hz;
    }

    char *ZMPT101B::getSummary(){
        static char strv[6];
        static char strh[6];
        static char summary[16];
        dtostrf(getVoltage(), 3, 1, strv);
        dtostrf(getHz(), 3, 1, strh);
        sprintf(summary, "%sV %sHz", strv, strh);
        return summary;
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
