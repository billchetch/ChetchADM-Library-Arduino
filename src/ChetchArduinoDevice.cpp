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

    void ArduinoDevice::initialise(ADMMessage* message){
        if(message->hasArgument(DEVICE_ENABLED_INDEX)){
            enable(message->argumentAsBool(DEVICE_ENABLED_INDEX));
        }
        if(message->hasArgument(REPORT_INTERVAL_INDEX)){
            setReportInterval(message->argumentAsInt(REPORT_INTERVAL_INDEX));
        }
    }

    void ArduinoDevice::configure(ADMMessage* message){
        configured = true;
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
        
    }

    void ArduinoDevice::createMessage(ADMMessage::MessageType messageType, ADMMessage *message){
        
    }

    void ArduinoDevice::sendMessage(ADMMessage *message){     
        if(isMessageReady()){
            createMessage(messageTypeToCreate, message);
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