#include "ChetchUtils.h"
#include "ChetchSwitchDevice.h"

namespace Chetch{
    
    SwitchDevice::SwitchDevice(byte id, byte cat, char *dn) : ArduinoDevice(id, cat, dn){
        //empty
    }


    int SwitchDevice::getArgumentIndex(ADMMessage *message, MessageField field){
        switch(field){
            default:
                return (int)field;
        }
    }

   void SwitchDevice::configure(ADMMessage* message, ADMMessage* response){
        ArduinoDevice::configure(message, response);
        
        int argIdx = getArgumentIndex(message, MessageField::MODE);
        mode = (SwitchMode)message->argumentAsByte(argIdx);
        argIdx = getArgumentIndex(message, MessageField::PIN);
        pin = message->argumentAsByte(argIdx);
        argIdx = getArgumentIndex(message, MessageField::PIN_STATE);
        pinState = message->argumentAsBool(argIdx);
        argIdx = getArgumentIndex(message, MessageField::TOLERANCE);
        tolerance = message->argumentAsInt(argIdx);

        response->addByte((byte)mode);
        response->addByte(pin);
        response->addBool(pinState);
        response->addInt(tolerance);

        switch(mode){
            case SwitchMode::PASSIVE:
                pinMode(pin, INPUT); break;

            case SwitchMode::ACTIVE:
                pinMode(pin, OUTPUT); 
                digitalWrite(pin, pinState);
                break;
                
        }
    }

    void SwitchDevice::createMessage(ADMMessage::MessageType messageTypeToCreate, ADMMessage* message){
        ArduinoDevice::createMessage(messageTypeToCreate, message);

        if(mode == SwitchMode::PASSIVE && messageTypeToCreate == ADMMessage::MessageType::TYPE_DATA){
            message->addBool(pinState);
        }
        if(mode == SwitchMode::ACTIVE && messageTypeToCreate == ADMMessage::MessageType::TYPE_COMMAND_RESPONSE){
            message->addByte(pinState ? DeviceCommand::ON : DeviceCommand::OFF);
            message->addBool(pinState);
        }
    }

	void SwitchDevice::loop(){
        ArduinoDevice::loop();
        
        if(mode == SwitchMode::PASSIVE){
            bool currentPinState = digitalRead(pin);
            if(currentPinState != pinState){
                //if there is a change of pin state then if we were already recording we simply reset
                //if we weren't recording then we start'
                recording = recording == 0 ? millis() : 0;
                pinState = currentPinState;
            } else if(recording > 0 && millis() - recording >= tolerance){ 
                //so here we are reording and have gone over tolerance so we trigger and reset
                trigger();
                recording = 0;
            }
        } else {
            if(recording > 0 && millis() - recording >= tolerance){
                trigger();
                recording = 0;
            }
        }

    }

    ArduinoDevice::DeviceCommand SwitchDevice::executeCommand(ADMMessage *message, ADMMessage *response){
        DeviceCommand deviceCommand = ArduinoDevice::executeCommand(message, response);

        switch(deviceCommand){
            case ON:
            case OFF:
                if(mode == SwitchMode::ACTIVE){
                    pinState = deviceCommand == DeviceCommand::ON;
                    bool currentPinState = digitalRead(pin);
                    if(currentPinState != pinState){
                        //if there is a request to change of pin state then if we haven't started recording then we start
                        if(recording == 0)recording = millis();
                    } else {
                        //otherwise we've either requested something already the case OR we've undone our previous request
                        recording = 0;
                    }
                    response->clear();
                } else {
                    addErrorInfo(response, ErrorCode::INVALID_COMMAND, message);
                }
                break;
        }
                
        return deviceCommand;
    } 

    void SwitchDevice::trigger(){
        if(mode == SwitchMode::PASSIVE){
            messageTypeToCreate = ADMMessage::MessageType::TYPE_DATA;
        } else {
            digitalWrite(pin, pinState);
            messageTypeToCreate = ADMMessage::MessageType::TYPE_COMMAND_RESPONSE;
        }
    }


} //end namespace
