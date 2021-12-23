#include "ChetchUtils.h"
#include "ChetchCounter.h"


namespace Chetch{

    byte Counter::instanceIndex = 0;
    Counter* Counter::instances[Counter::MAX_INSTANCES];
    
    Counter* Counter::create(byte id, byte cat, char *dn){
        if(instanceIndex >= MAX_INSTANCES){
            return NULL;
        } else {
            Counter* instance = new Counter(id, cat, dn);
            instances[instanceIndex++] = instance;
            return instance;
        }
    }


    Counter::Counter(byte id, byte cat, char *dn) : ArduinoDevice(id, cat, dn){
        //empty
    }

    bool Counter::configure(ADMMessage* message, ADMMessage* response){
        if(!ArduinoDevice::configure(message, response))return false;

        int argIdx = getArgumentIndex(message, MessageField::PIN);
        pin = message->argumentAsByte(argIdx);

        argIdx = getArgumentIndex(message, MessageField::INTERRUPT_MODE);
        interruptMode = (InterruptMode)message->argumentAsByte(argIdx);

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
