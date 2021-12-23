#include "ChetchUtils.h"
#include "ChetchCounter.h"
#include "ChetchInterrupt.h"


namespace Chetch{
    byte Counter::instanceCount = 0;
    
    void Counter::handleInterrupt(uint8_t pin) {
        if (instances[0] != NULL && instances[0]->pin == pin) {
            instances[0]->onInterrupt();
        }
        else if (instances[1] != NULL && instances[1]->pin == pin) {
            instances[1]->onInterrupt();
        }
        else if (instances[2] != NULL && instances[2]->pin == pin) {
            instances[2]->onInterrupt();
        }
        else if (instances[3] != NULL && instances[3]->pin == pin) {
            instances[3]->onInterrupt();
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
            instances[instanceCount] = instance;
            instanceCount++;
            return instance;
        }
    }

    Counter::Counter(byte id, byte cat, char *dn) : ArduinoDevice(id, cat, dn){
        //empty
    }

    Counter::~Counter() {
        if (interruptMode != InterruptMode::IM_NONE) {
            CInterrupt::removeInterrupt(pin);
        }
    }

    bool Counter::configure(ADMMessage* message, ADMMessage* response){
        if(!ArduinoDevice::configure(message, response))return false;

        int argIdx = getArgumentIndex(message, MessageField::PIN);
        pin = message->argumentAsByte(argIdx);

        argIdx = getArgumentIndex(message, MessageField::INTERRUPT_MODE);
        interruptMode = (InterruptMode)message->argumentAsByte(argIdx);

        if (interruptMode != InterruptMode::IM_NONE) {
            CInterrupt::addInterrupt(pin, handleInterrupt, (uint8_t)interruptMode);
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
            
        }
    }

    void Counter::loop(){
        ArduinoDevice::loop(); 
        
    }

    void Counter::onInterrupt(){
        
    }

    
} //end namespace
