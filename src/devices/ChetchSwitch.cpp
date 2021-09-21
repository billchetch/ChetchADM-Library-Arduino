#include "ChetchUtils.h"
#include "ChetchSwitch.h"

namespace Chetch{
    
    Switch::Switch(byte id, byte cat, char *dn) : ArduinoDevice(id, cat, dn){
        //empty
    }


    int Switch::getArgumentIndex(ADMMessage *message, MessageField field){
        switch(field){
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
        argIdx = getArgumentIndex(message, MessageField::INITIAL_STATE);
        state = message->argumentAsBool(argIdx);


        switch(mode){
            case Mode::PASSIVE:
                pinMode(pin, INPUT); break;

            case Mode::ACTIVE:
                pinMode(pin, OUTPUT); break;
                
        }
    }

    void Switch::createMessage(ADMMessage::MessageType messageTypeToCreate, ADMMessage* message){
        ArduinoDevice::createMessage(messageTypeToCreate, message);

        if(mode == Mode::PASSIVE){
            message->addBool(state);
        }
    }

	void Switch::loop(){
        ArduinoDevice::loop();
        
        if(mode == Mode::PASSIVE){
            bool newState = digitalRead(pin);
            if(newState != state){
                recording = millis();
                state = newState;
            } else if(recording > 0 && recording - millis() >= tolerance){
                trigger();
                recording = 0;
            }
        }
    }

    ArduinoDevice::DeviceCommand Switch::executeCommand(ADMMessage *message, ADMMessage *response){
        DeviceCommand deviceCommand = ArduinoDevice::executeCommand(message, response);

        switch(deviceCommand){
            case ON:
                if(mode == Mode::ACTIVE){
                    digitalWrite(pin, HIGH);
                }
                break;

            case OFF:
                if(mode == Mode::ACTIVE){
                    digitalWrite(pin, LOW);
                }
                break;
        }
                
        return deviceCommand;
    }

    void Switch::trigger(){
        messageTypeToCreate = ADMMessage::MessageType::TYPE_DATA;
    }


} //end namespace
