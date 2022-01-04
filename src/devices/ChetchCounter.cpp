#include "ChetchUtils.h"
#include "ChetchCounter.h"
#include "ChetchInterrupt.h"


namespace Chetch{
    byte Counter::instanceCount = 0;
    
    void Counter::handleInterrupt(uint8_t pin, uint8_t tag) {
        if(instanceCount > tag){
            instances[tag]->onInterrupt();
        }
    }

    Counter* Counter::create(byte id, byte cat, char* dn) {
        if (instanceCount >= MAX_INSTANCES) {
            return NULL;
        }
        else {
            if (instanceCount == 0) {
                for (byte i = 0; i < MAX_INSTANCES; i++) {
                    instances[i] = NULL;
                }
            }
            Counter* instance = new Counter(id, cat, dn);
            instance->setInstanceIndex(instanceCount);
            instances[instanceCount] = instance;
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
    }

    void Counter::setInstanceIndex(byte idx){
        instanceIndex = idx;
    }

    bool Counter::configure(ADMMessage* message, ADMMessage* response){
        if(!ArduinoDevice::configure(message, response))return false;

        int argIdx = getArgumentIndex(message, MessageField::PIN);
        pin = message->argumentAsByte(argIdx);

        argIdx = getArgumentIndex(message, MessageField::INTERRUPT_MODE);
        interruptMode = message->argumentAsByte(argIdx);

        if (interruptMode != 0) {
            CInterrupt::addInterrupt(pin, instanceIndex, handleInterrupt, interruptMode);
        }

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

    void Counter::createMessageToSend(byte messageID, ADMMessage* message){
        ArduinoDevice::createMessageToSend(messageID, message);

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
        if(countStarted)count++;
    }

    
} //end namespace
