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

    void ArduinoDevice::initialise(ADMMessage *message, ADMMessage *response){
        initialised = true;
        
        response->type = ADMMessage::MessageType::TYPE_INITIALISE_RESPONSE;
        response->addByte(category);
        response->addString(this->name);
    }

    void ArduinoDevice::configure(ADMMessage *message, ADMMessage *response){
        configured = true;
        int argIdx = getArgumentIndex(message, MessageField::ENABLED);
        enable(message->argumentAsBool(argIdx));
        argIdx = getArgumentIndex(message, MessageField::REPORT_INTERVAL);
        setReportInterval(message->argumentAsInt(argIdx));
        
        response->type = ADMMessage::MessageType::TYPE_CONFIGURE_RESPONSE;
        response->addBool(enabled);
        response->addInt(reportInterval);
    }

    bool ArduinoDevice::isReady(){
        return initialised && configured;
    }

    bool ArduinoDevice::isActive(){
        return enabled && isReady();
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

    int ArduinoDevice::getReportInterval(){
        return reportInterval;
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

              case ADMMessage::MessageType::TYPE_STATUS_REQUEST:
                response->type = ADMMessage::TYPE_STATUS_RESPONSE;
                response->addBool(enabled);
                response->addInt(reportInterval);
                break;
        }
    }

    void ArduinoDevice::createMessage(ADMMessage::MessageType messageType, ADMMessage *message){
        message->type = messageType;
        message->target = getID();
        message->sender = getID();
    }

    void ArduinoDevice::sendMessage(ADMMessage *message){     
        if(isMessageReady()){
            createMessage(messageTypeToCreate, message);
            //reset messageTypeToCreate as it is used by isMessageReady
            messageTypeToCreate = ADMMessage::MessageType::TYPE_NONE;
        }
    }

    int ArduinoDevice::getArgumentIndex(ADMMessage *message, MessageField field){
        switch(field){
            default:
                return (int)field;
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