#include "ChetchUtils.h"
#include "ChetchCounter.h"
#include "ChetchInterrupt.h"


namespace Chetch{
    byte Counter::instanceCount = 0;
    Counter* Counter::instances[];
    volatile unsigned long Counter::handleInterruptCount = 0;

    void Counter::handleInterrupt(uint8_t pin, uint8_t tag) {
        //handleInterruptCount++;
        if(instanceCount > tag){
            instances[tag]->onInterrupt();
        }
    }

    Counter* Counter::create(byte id, byte cat, char* dn) {
        if (instanceCount >= MAX_INSTANCES) {
            return NULL;
        } else {
            if (instanceCount == 0) {
                for (byte i = 0; i < MAX_INSTANCES; i++) {
                    instances[i] = NULL;
                }
            }
            Counter* instance = new Counter(id, cat, dn);
            byte idx = 0;
            for (byte i = 0; i < MAX_INSTANCES; i++) {
                if(instances[i] == NULL){
                    idx = i;
                    break;
                }
            }
            instance->setInstanceIndex(idx);
            instances[idx] = instance;
            instanceCount++;
            return instance;
        }
    }

    Counter::Counter(byte id, byte cat, char *dn) : ArduinoDevice(id, cat, dn){
        //empty
    }

    Counter::~Counter() {
        if (interruptMode != 0) {
            CInterrupt::removeInterrupt(pin);
        }
        instances[instanceIndex] = NULL;
        instanceCount--;
    }

    void Counter::setInstanceIndex(byte idx){
        instanceIndex = idx;
    }

    void Counter::setPin(byte pin){
        this->pin = pin;
    }

    bool Counter::setInterruptMode(byte mode){
        if (mode != 0 && interruptMode == 0) { //one time set
            interruptMode = mode;
            return CInterrupt::addInterrupt(pin, instanceIndex, handleInterrupt, interruptMode);
        }
        return true;
    }

    bool Counter::configure(ADMMessage* message, ADMMessage* response){
        if(!ArduinoDevice::configure(message, response))return false;

        int argIdx = getArgumentIndex(message, MessageField::PIN);
        setPin(message->argumentAsByte(argIdx));

        argIdx = getArgumentIndex(message, MessageField::INTERRUPT_MODE);
        setInterruptMode(message->argumentAsByte(argIdx));

        argIdx = getArgumentIndex(message, MessageField::TOLERANCE);
        tolerance = message->argumentAsULong(argIdx);

        response->addByte(pin);
        response->addByte(interruptMode);

        return true;
    }

    void Counter::status(ADMMessage *message, ADMMessage *response){
        ArduinoDevice::status(message, response);

        response->addULong(count);
        response->addByte((byte)interruptMode);
    }

    int Counter::getArgumentIndex(ADMMessage *message, Counter::MessageField field){
        switch(field){
            default:
                return (int)field;
        }
    }

    void Counter::populateMessageToSend(byte messageID, ADMMessage* message){
        ArduinoDevice::populateMessageToSend(messageID, message);

        if(messageID == ArduinoDevice::MESSAGE_ID_REPORT){
            message->addULong(count);
            unsigned long duration = micros() - countStartedOn;
            message->addULong(duration);

            count = 0;
            countStartedOn = micros();
        }
    }

    void Counter::enable(bool enable){
        ArduinoDevice::enable(enable);
        if(!enable){
            count = 0;
            countStarted = false;
        }
    }


    void Counter::loop(){
        ArduinoDevice::loop(); 
        
        if(!countStarted){
            countStarted = true;
            countStartedOn = micros();
            count = 0;
        }
    }

    void Counter::onInterrupt(){
        if(countStarted && (tolerance == 0 || count == 0 || (micros() - countedOn >= tolerance))){
            count++;
            countedOn = micros();
        }
    }

    unsigned long Counter::getCount(){
        return count;
    }

    
} //end namespace
