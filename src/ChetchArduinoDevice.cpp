#include "ChetchUtils.h"
#include "ChetchArduinoDevice.h"

namespace Chetch{
	ArduinoDevice::ArduinoDevice(byte id, byte category, char* dname){
        this->ID = id;
        this->category = category;

        if(dname != NULL){
            for(int i = 0; i < strlen(dname); i++){
                name[i] = dname[i];
            }
            name[strlen(dname)] = 0;
        }
    }

    int ArduinoDevice::initialise(ADMMessage *message, ADMMessage *response){
        initialised = true;
        if(message->hasArgument(DEVICE_ENABLED_INDEX)){
            enable(message->argumentAsBool(DEVICE_ENABLED_INDEX));
        }
        if(message->hasArgument(REPORT_INTERVAL_INDEX)){
            setReportInterval(message->argumentAsInt(REPORT_INTERVAL_INDEX));
        }
        return 0;
    }

    int ArduinoDevice::configure(ADMMessage *message, ADMMessage *response){
        configured = true;
        int argIdx = 0;
        setReportInterval(message->argumentAsInt(argIdx++));
        return argIdx;
    }

    bool ArduinoDevice::isActive(){
        return enabled && configured;
    }

    byte ArduinoDevice::getID(){
        return ID;
    }

    char *ArduinoDevice::getName(){
        return name;
    }
    
    void ArduinoDevice::enable(bool enable){
        enabled = enable;
    }

    void ArduinoDevice::setReportInterval(int interval){
        reportInterval = interval;
    }
    
    bool ArduinoDevice::isMessageReady(){
        return messageTypeToCreate != ADMMessage::MessageType::TYPE_NONE;
    }

    void ArduinoDevice::receiveMessage(ADMMessage *message, ADMMessage *response){
        response->sender = getID();
        response->target = getID();
                
        switch ((ADMMessage::MessageType)message->type) {
            case ADMMessage::MessageType::TYPE_INITIALISE:
                initialise(message, response);
                break;

             case ADMMessage::MessageType::TYPE_CONFIGURE:
                configure(message, response);
                break;
        }
    }

    void ArduinoDevice::createMessage(ADMMessage::MessageType messageType, ADMMessage *message){
        message->type = messageType;
    }

    void ArduinoDevice::sendMessage(ADMMessage *message){     
        if(isMessageReady()){
            createMessage(messageTypeToCreate, message);
            //reset messageTypeToCreate as it is used by isMessageReady
            messageTypeToCreate = ADMMessage::MessageType::TYPE_NONE;
        }
    }

    void ArduinoDevice::loop(){
        if(reportInterval > 0 && millis() - lastMillis >= reportInterval){
            //Serial.print("Message ready at device "); Serial.print(ID); Serial.print(" after "); Serial.print(millis() - lastMillis); Serial.println("ms");
            lastMillis = millis();
            messageTypeToCreate = ADMMessage::MessageType::TYPE_DATA;
        }
    }

} //end namespace