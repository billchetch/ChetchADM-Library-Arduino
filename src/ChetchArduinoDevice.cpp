#include "ChetchUtils.h"
#include "ChetchArduinoDevice.h"
#include "ChetchADM.h"

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

        for(int i = 0; i < MESSAGE_QUEUE_LENGTH; i++){
            messageQueue[i] = 0;
        }
    }

    ArduinoDevice::~ArduinoDevice(){
        //empty ... to allow for polymorphic destructors
    }

    bool ArduinoDevice::initialise(ADMMessage *message, ADMMessage *response){
        initialised = true;
        
        response->type = ADMMessage::MessageType::TYPE_INITIALISE_RESPONSE;
        response->addByte(category);
        response->addString(this->name);

        return initialised;
    }

    bool ArduinoDevice::configure(ADMMessage *message, ADMMessage *response){
        configured = true;
        int argIdx = getArgumentIndex(message, MessageField::ENABLED);
        enable(message->argumentAsBool(argIdx));
        argIdx = getArgumentIndex(message, MessageField::REPORT_INTERVAL); //in millis
        setReportInterval(message->argumentAsInt(argIdx));
        
        
        response->type = ADMMessage::MessageType::TYPE_CONFIGURE_RESPONSE;
        response->addBool(enabled);
        response->addInt(reportInterval);

        return configured;
    }

    void ArduinoDevice::status(ADMMessage *message, ADMMessage *response){
        response->type = ADMMessage::MessageType::TYPE_STATUS_RESPONSE;
        response->addBool(enabled);
        response->addInt(reportInterval);
    }

    ArduinoDevice::DeviceCommand ArduinoDevice::executeCommand(ADMMessage *message, ADMMessage *response){
        int argIdx = getArgumentIndex(message, MessageField::DEVICE_COMMAND);
        DeviceCommand deviceCommand = (DeviceCommand)message->argumentAsByte(argIdx);

        //prepare for normal response
        response->type = ADMMessage::MessageType::TYPE_COMMAND_RESPONSE;
        response->addByte(deviceCommand);
        switch(deviceCommand){
            case ENABLE:
                argIdx = getArgumentIndex(message, MessageField::ENABLED);
                enable(message->argumentAsBool(argIdx));
                response->addBool(enabled);
                break;

            case DISABLE:
                enable(false);
                response->addBool(enabled);
                break;

            case SET_REPORT_INTERVAL:
                argIdx = getArgumentIndex(message, MessageField::REPORT_INTERVAL);
                setReportInterval(message->argumentAsInt(argIdx));
                response->addInt(reportInterval);
                break;
        }
                
        return deviceCommand;
    }

    bool ArduinoDevice::isReady(){
        return initialised && configured;
    }
    
    void ArduinoDevice::setAsReady(){
        initialised = true;
        configured = true;
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

    void ArduinoDevice::receiveMessage(ADMMessage *message, ADMMessage *response){
        response->sender = getID();
        response->target = getID();
                
        switch ((ADMMessage::MessageType)message->type) {
            case ADMMessage::MessageType::TYPE_INITIALISE:
                if(!initialise(message, response)){
                    addErrorInfo(response, ErrorCode::FAILED_TO_INITIALISE, 0, message);
                }
                break;

            case ADMMessage::MessageType::TYPE_CONFIGURE:
                if(!configure(message, response)){
                    addErrorInfo(response, ErrorCode::FAILED_TO_CONFIGURE, 0, message);
                }
                break;

            case ADMMessage::MessageType::TYPE_STATUS_REQUEST:
                status(message, response);
                break;

            case ADMMessage::MessageType::TYPE_COMMAND:
                executeCommand(message, response);
                break;
        }
    }

    void ArduinoDevice::populateMessage(ADMMessage::MessageType messageType, ADMMessage *message){
        message->type = messageType;
        message->target = getID();
        message->sender = getID();
    }

    void ArduinoDevice::addErrorInfo(ADMMessage * message, ErrorCode errorCode, int subCode, ADMMessage *originalMessage){
        message->clear();
        populateMessage(ADMMessage::MessageType::TYPE_ERROR, message);
        message->addByte((byte)ArduinoDeviceManager::ErrorCode::DEVICE_ERROR);
        message->addByte((byte)errorCode);
        message->addInt(subCode);
        if(originalMessage != NULL){
            message->addByte(originalMessage->type);
            message->addByte(originalMessage->target);
            message->addByte(originalMessage->sender);
        }
    }

    bool ArduinoDevice::hasMessageToSend(){
        return messageCount > 0;
    }

    bool ArduinoDevice::enqueueMessageToSend(byte messageID){
        if(messageCount >= MESSAGE_QUEUE_LENGTH){
            return false;
        } else {
            messageQueue[messageCount] = messageID;
            messageCount++;
            return true;
        }
    }

    byte ArduinoDevice::dequeueMessageToSend(){
        if(messageCount == 0){
            return 0;
        } else {
            byte messageID = messageQueue[0];
            messageCount--;
            for(byte i = 0; i < messageCount; i++){
                messageQueue[i] = messageQueue[i + 1];
            }
            messageQueue[messageCount] = 0; //
            return messageID;
        }
    }

    //Note: if you do not wish the message to be sent then you should clear it'
    void ArduinoDevice::populateMessageToSend(byte messageID, ADMMessage *message){
        switch(messageID){
            case MESSAGE_ID_REPORT:
                populateMessage(ADMMessage::MessageType::TYPE_DATA, message);
                break;
        }
    }

    void ArduinoDevice::sendMessage(ADMMessage *message){     
        if(hasMessageToSend()){
            populateMessageToSend(dequeueMessageToSend(), message);
        }
    }

    int ArduinoDevice::getArgumentIndex(ADMMessage *message, MessageField field){
        switch(field){
            case MessageField::DEVICE_COMMAND:
                return 0;

            case MessageField::ENABLED:
                return message->type == ADMMessage::MessageType::TYPE_COMMAND ? 1 : 0;

            case MessageField::REPORT_INTERVAL:
                return message->type == ADMMessage::MessageType::TYPE_COMMAND ? 1 : (int)field;
            
            default:
                return (int)field;
        }
    }

    void ArduinoDevice::loop(){
        if(reportInterval > 0 && millis() - lastMillis >= reportInterval){
            //Serial.print("Message ready at device "); Serial.print(ID); Serial.print(" after "); Serial.print(millis() - lastMillis); Serial.println("ms");
            lastMillis = millis();
            enqueueMessageToSend(MESSAGE_ID_REPORT);
        }
    }
} //end namespace