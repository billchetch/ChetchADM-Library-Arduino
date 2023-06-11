#include "ChetchUtils.h"
#include "ChetchTestBandwidth.h"

namespace Chetch{
    
    TestBandwidth::TestBandwidth(byte id, byte cat, char *dn) : ArduinoDevice(id, cat, dn){
        //empty
    }

    int TestBandwidth::getArgumentIndex(ADMMessage *message, MessageField field){
        switch(field){
            default:
                return (int)field;
        }
    }

    bool TestBandwidth::configure(ADMMessage* message, ADMMessage* response){
        if(!ArduinoDevice::configure(message, response))return false;
        
        int argIdx = getArgumentIndex(message, MessageField::ACTIVITY_PIN);
        activityPin = message->argumentAsByte(argIdx);
        if(activityPin >= 0){
            pinMode(activityPin, OUTPUT);
            pinState = LOW;
            digitalWrite(activityPin, pinState);
        }
        response->addInt(activityPin);

        return true;
    }

    void TestBandwidth::status(ADMMessage *message, ADMMessage *response){
        ArduinoDevice::status(message, response);
        if(ADM != NULL){
            response->addULong(ADM->getBytesReceived());
            response->addULong(ADM->getBytesSent());
            response->addULong(ADM->getMessagesReceived());
            response->addULong(ADM->getMessagesSent());
            response->addULong(messagesReceived);
            response->addULong(messagesSent);
        }
    }

    void TestBandwidth::populateMessageToSend(byte messageID, ADMMessage* message){
        ArduinoDevice::populateMessageToSend(messageID, message);

        if(activityPin >= 0)activatePin = true;
        messagesSent++;

        if(messageID == ArduinoDevice::MESSAGE_ID_REPORT){
            message->addULong(ADM->getBytesReceived());
            message->addULong(ADM->getBytesSent());
            message->addULong(ADM->getMessagesReceived());
            message->addULong(ADM->getMessagesSent());
            message->addULong(messagesReceived);
            message->addULong(messagesSent);
        }
    }

    void TestBandwidth::receiveMessage(ADMMessage *message, ADMMessage *response){
        ArduinoDevice::receiveMessage(message, response);

        if(activityPin >= 0)activatePin = true;
        messagesReceived++;
    }

    ArduinoDevice::DeviceCommand TestBandwidth::executeCommand(ADMMessage *message, ADMMessage *response){
        DeviceCommand deviceCommand = ArduinoDevice::executeCommand(message, response);

        //prepare for normal response
        switch(deviceCommand){
            case ANALYSE:
                //analyse the message that was sent
                response->addByte(message->getByteCount());
                break;
        }
                
        return deviceCommand;
    }

    void TestBandwidth::loop(){
        ArduinoDevice::loop();

        if(activatePin){
            if(pinState == LOW){
                pinState = HIGH;
                digitalWrite(activityPin, pinState);
                pinActivatedOn = millis();
            } else if(millis() - pinActivatedOn > 100){
                pinState = LOW;
                digitalWrite(activityPin, pinState);
                activatePin = false;
            }
        }
    }

} //end namespace
