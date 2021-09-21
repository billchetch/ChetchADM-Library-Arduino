#include "ChetchUtils.h"
#include "ChetchSwitch.h"

namespace Chetch{
    
    Switch::Switch(byte id, byte cat, char *dn) : ArduinoDevice(id, cat, dn){
        //empty
    }


    int Swich::getArgumentIndex(ADMMessage *message, MessageField field){
        swich(field){
            default:
                return (int)field;
        }
    }

    void Switch::configure(ADMMessage* message, ADMMessage* response){
        ArduinoDevice::configure(message, response);
        
        int argIdx = getArgumentIndex(message, MessageField::MODE);
        mode = (Mode)message->argumentAsByte(argIdx);
        argIdx = getArgumentIndex(message, MessageField::PIN);
        pin = message->argumentAsByte(argIdx);
        argIdx = getArgumentIndex(message, MessageField::TRIGGER_STATE);
        triggerState = message->argumentAsBool(argIdx);


        switch(mode){
            case Mode::PASSIVE:
                pinMode(pin, INPUT); break;

            case Mode::ACTIVE:
                pinMode(pin, OUTPUT); break;
                
        }
    }

    void Switch::createMessage(ADMMessage::MessageType messageTypeToCreate, ADMMessage* message){
        ArduinoDevice::createMessage(messageTypeToCreate, message);

        
    }

	void Switch::loop(){
        ArduinoDevice::loop();
        
        if(mode == Mode::PASSIVE){
            bool state = digitalRead(pin);
            if(state == triggerState){
            } else {
                
            }
        }
    }

    void Switch::trigger(bool ){
        
    }
} //end namespace
