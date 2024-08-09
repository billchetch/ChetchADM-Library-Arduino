#include "ChetchUtils.h"
#include "ChetchADC.h"
#include "ChetchZMPT101B.h"

#if defined(ARDUINO_AVR_MEGA2560)
    #define TIMER_NUMBER 4
    #define TIMER_PRESCALER 8 //'ticks'every 0.5 microseconds
#else
    #define TIMER_NUMBER 0
    #define TIMER_PRESCALER 0
#endif


namespace Chetch{
    
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
                sei();

                CADC::init(false); //turn off trigger interrupt

                for(byte i = 0; i < MAX_INSTANCES; i++){
                    instances[i] = NULL;
                }
                currentInstance = 0;
            }

            //get first available slot
            byte idx = 0;
            for(byte i = 0; i < MAX_INSTANCES; i++){
                if(instances[i] == NULL){
                    idx = i;
                    break;
                }
            }

            ZMPT101B* instance = new ZMPT101B(id, cat, dn);
            instance->setInstanceIndex(idx);
            instance->useTimer = true;
            instances[idx] = instance;
            instanceCount++;

            unsigned int m = 500;
            unsigned int comp = timer->microsToTicks(m);
            /*Serial.print("Setting ZMPT101B timer comp to ");
            Serial.print(comp);
            Serial.print(" ticks = ");
            Serial.print(m);
            Serial.println(" micros");*/
            timer->registerCallback(&ZMPT101B::handleTimerInterrupt, ISRTimer::LOWEST_PRIORITY, comp);
            return instance;
        }
    }

    void ZMPT101B::handleTimerInterrupt(){
        static ZMPT101B* zmpt = NULL;
        
        //free up other interrupts
        //sei();

        zmpt = instances[currentInstance];
        
        if (!CADC::isReading()) {
            zmpt->onAnalogRead(CADC::readResult());
            CADC::startRead(zmpt->voltagePin);
        }
        
    }


    ZMPT101B::ZMPT101B(byte id, byte cat, char *dn) : ArduinoDevice(id, cat, dn){
        
        //clean to begin
        for(byte i = 0; i < BUFFER_SIZE; i++){
            buffer[i] = 0;
        }
    }

    ZMPT101B::~ZMPT101B(){
        instances[instanceIndex] = NULL;
        instanceCount--;
        if(timer->isEnabled()){
            timer->disable();
        }
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
    
    void ZMPT101B::onAnalogRead(uint16_t value) {
        //This method is called by the timer interrupt
        
        if (bufferIdx < BUFFER_SIZE) {
            buffer[bufferIdx++] = value;
        }
        
    }

    void ZMPT101B::loop(){
        ArduinoDevice::loop(); 
        
        if(useTimer && !timer->isEnabled() && isReady()){
            timer->enable();
            CADC::startRead(voltagePin);
            return;
        }

        if (samplingPaused)return;

        //Per sample set values
        static unsigned long sampleCount = 0;
        static unsigned long summedVoltages = 0;
        static unsigned long hzCount = 0;
        static unsigned long hzCountDuration = 0; //in timer ticks
        //static int tempBuffer[BUFFER_SIZE];
        //static bool temp = false;
        
        if (bufferIdx >= BUFFER_SIZE) {
            //per batch values
            int prevVoltage = 0;
            int hzCountStartedOn = -1;
            int hzCountEndedOn = 0;
            int currentPosition = 0; //1 means above 10v, -1 means below 10v, 0 unknown
            int prevPosition = 0; //1 means above 10v, -1 means below 10v, 0 unknown
            
            //go through the data on teh buffer
            for (int i = 0; i < bufferIdx; i++) {
                int currentVoltage = (int)buffer[i] - midPoint;
                //tempBuffer[i] = currentVoltage; // buffer[i];
                
                if (currentVoltage > hzNoiseThreshold) {
                    currentPosition = 1;
                }
                else if (currentVoltage < -hzNoiseThreshold) {
                    currentPosition = -1;
                }
                else {
                    currentPosition = 0;
                }

                //sum the squares for rms later
                summedVoltages += ((long)currentVoltage * (long)currentVoltage);

                if (currentPosition == 1 && prevPosition == -1 || currentPosition == -1 && prevPosition == 1) {
                    if (hzCountStartedOn == -1) {
                        hzCountStartedOn = i;
                    }
                    else {
                        hzCountEndedOn = i;
                        hzCount++;
                    }
                }
                if (currentPosition != 0)prevPosition = currentPosition;
            }

            hzCountDuration += hzCountEndedOn - hzCountStartedOn;
            sampleCount += BUFFER_SIZE;

            //fill up buffer again for some more samples
            bufferIdx = 0;
        } //finished processing buffer

        //now process all the samples
        if (sampleCount >= sampleSize) {
            //some analysis
            double newVoltage = sqrt((double)summedVoltages / (double)sampleCount); // *scaleWaveform) + finalOffset;
            uint32_t duration = timer->interruptsToMicros(&ZMPT101B::handleTimerInterrupt, hzCountDuration);
            double newHz = (500000.0 * (double)hzCount) / (double)duration;

            assignResults(newVoltage, newHz);

            //reset for next set of samples
            sampleCount = 0;
            summedVoltages = 0;
            hzCount = 0;
            hzCountDuration = 0;
        } //end sample finished conditional
    }

    void ZMPT101B::pauseSampling(bool pause) {
        samplingPaused = pause;
    }

    void ZMPT101B::assignResults(double newVoltage, double newHz) {
        //assign to key properties
        voltage = newVoltage;
        hz = newHz;

        /*Serial.print(" V: ");
        Serial.print(voltage);
        Serial.print(",");
        Serial.print(" Hz: ");
        Serial.print(hz);
        Serial.println();*/

        //raise some events
        raiseEvent(EVENT_NEW_RESULTS);

        static bool adjustmentRequired = false;
        double adj = adjustBy();
        if (adj != 0) {
            adjustmentRequired = true;
            raiseEvent(EVENT_ADJUSTMENT_REQUIRED);
        }
        else if (adjustmentRequired) {
            adjustmentRequired = false;
            raiseEvent(EVENT_TARGET_ATTAINED);
        }
    }

    double ZMPT101B::getVoltage(){
        return voltage;
    }

    double ZMPT101B::getHz(){
        return hz;
    }

    char *ZMPT101B::getSummary(){
        char strv[6];
        char strh[6];
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
        return abs(adjustment) < targetTolerance ? 0 : adjustment;
    } 
} //end namespace
