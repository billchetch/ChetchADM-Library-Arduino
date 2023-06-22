#include "ChetchUtils.h"
#include "ChetchTicker.h"

namespace Chetch{
    
    Ticker::Ticker(byte id, byte cat, char *dn) : ArduinoDevice(id, cat, dn){
        //empty
    }


    int Ticker::getArgumentIndex(ADMMessage *message, MessageField field){
        switch(field){
            default:
                return (int)field;
        }
    }

   bool Ticker::configure(ADMMessage* message, ADMMessage* response){
        if(!ArduinoDevice::configure(message, response))return false;
        
        int argIdx = getArgumentIndex(message, MessageField::PIN);
        pin = message->argumentAsByte(argIdx);
        argIdx = getArgumentIndex(message, MessageField::PIN_HIGH_DURATION);
        pinHighDuration = message->argumentAsUInt(argIdx);
        argIdx = getArgumentIndex(message, MessageField::PIN_LOW_DURATION);
        pinLowDuration = message->argumentAsUInt(argIdx);

        response->addByte(pin);
        
        pinMode(pin, OUTPUT); 
        pinState = LOW;
        digitalWrite(pin, pinState);
        
        return true;
    }

    void Ticker::populateMessageToSend(byte messageID, ADMMessage* message){
        ArduinoDevice::populateMessageToSend(messageID, message);

        if(messageID == ArduinoDevice::MESSAGE_ID_REPORT){
            message->addULong(tickCount);
        }
    }

	void Ticker::loop(){
        ArduinoDevice::loop();
        

        if(pinState == LOW && millis() - pinLowStartedOn > pinLowDuration){
            pinState = HIGH;
            pinHighStartedOn = millis();
            tickCount++;
            digitalWrite(pin, pinState);
        } else if(pinState == HIGH && millis() - pinHighStartedOn > pinHighDuration){
            pinState = LOW;
            pinLowStartedOn = millis();
            digitalWrite(pin, pinState);
        }
    }

    ArduinoDevice::DeviceCommand Ticker::executeCommand(ADMMessage *message, ADMMessage *response){
        DeviceCommand deviceCommand = ArduinoDevice::executeCommand(message, response);

        switch(deviceCommand){
            default:
                break;
        }
                
        return deviceCommand;
    } 
} //end namespace
