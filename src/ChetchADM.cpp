#include "ChetchUtils.h"
#include "ChetchADM.h"
#include "ChetchMessageFrame.h"
#include <MemoryFree.h>

#if (INCLUDE_DEVICES & TEMPERATURE_DEVICES) == TEMPERATURE_DEVICES
#include "devices/ChetchDS18B20Array.h"
#endif

#if (INCLUDE_DEVICES & RANGE_FINDER_DEVICES) == RANGE_FINDER_DEVICES
#include "devices/ChetchJSN_SR04T.h"
#endif

#if (INCLUDE_DEVICES & IR_DEVICES) == IR_DEVICES
#include "devices/ChetchIRReceiver.h"
#include "devices/ChetchIRTransmitter.h"
#endif

#if (INCLUDE_DEVICES & ELECTRICITY_MEASURING_DEVICES) == ELECTRICITY_MEASURING_DEVICES
#include "devices/electricity/ChetchZMPT101B.h"
#endif

#if (INCLUDE_DEVICES & DIAGNOSTIC_DEVICES) == DIAGNOSTIC_DEVICES
#include "devices/diagnostics/ChetchTest01.h"
#endif

const char DS18B20[] PROGMEM = "DS18B20";
const char JSN_SR04T[] PROGMEM = "JSN-SR04T";
const char ZMPT101B[] PROGMEM = "ZMPT101B";
const char TEST01[] PROGMEM = "TEST01";
const char TEST02[] PROGMEM = "TEST02";

const char *const DEVICES_TABLE[] PROGMEM = {
	DS18B20,
	JSN_SR04T,
    ZMPT101B,
    TEST01,
    TEST02
};

#define DEVICE_TABLE_SIZE 5

namespace Chetch{
    ArduinoDeviceManager *ArduinoDeviceManager::ADM = NULL;
    MessageFrame ArduinoDeviceManager::frame(MessageFrame::FrameSchema::SMALL_SIMPLE_CHECKSUM, ADM_MESSAGE_SIZE);
    ADMMessage ArduinoDeviceManager::inMessage(ADM_MESSAGE_SIZE);
    ADMMessage ArduinoDeviceManager::outMessage(ADM_MESSAGE_SIZE);

    int ArduinoDeviceManager::inDevicesTable(char *dname){
        char stBuffer[ArduinoDevice::DEVICE_NAME_LENGTH];
        for(int i = 0; i < DEVICE_TABLE_SIZE; i++){
            if (strcmp(dname, Utils::getStringFromProgmem(stBuffer, i, DEVICES_TABLE)) == 0) {
			    return i;
		    } 
        }
        return -1;
    }

    /*
    * Create static instance because stream callbacks cannot (not easy to?) use instance member methods
    */
    ArduinoDeviceManager *ArduinoDeviceManager::create(StreamWithCTS *stream){
        if(ADM == NULL){
            stream->setCommandHandler(handleStreamCommand);
            stream->setEventHandlers(handleStreamLocalEvent, handleStreamRemoteEvent);
            stream->setReadyToReceiveHandler(handleStreamReadyToReceive);
            stream->setReceiveHandler(handleStreamReceive);
            stream->setSendHandler(handleStreamSend);
            stream->setMaxDatablockSize(frame.getMaxSize());
            ADM = new ArduinoDeviceManager(stream);
        }
        
        return ADM;
    }


    ArduinoDeviceManager *ArduinoDeviceManager::getInstance(){
        return ADM;
        
    }

    void ArduinoDeviceManager::handleStreamCommand(StreamWithCTS *stream, byte cmd){
        if(ADM == NULL){
            addErrorInfo(&outMessage, ErrorCode::NO_ADM_INSTANCE);
        } else {
            
        }
    }

    void ArduinoDeviceManager::handleStreamLocalEvent(StreamWithCTS *stream, byte evt){
        if(ADM == NULL){
            addErrorInfo(&outMessage, ErrorCode::NO_ADM_INSTANCE);
        } else {
            switch(evt){
                case (byte)StreamWithCTS::Event::CTS_TIMEOUT: //local has waited too long for a CTS from remote
                    break;

                case (byte)StreamWithCTS::Event::RECEIVE_BUFFER_FULL:
                case (byte)StreamWithCTS::Event::MAX_DATABLOCK_SIZE_EXCEEDED:
                case (byte)StreamWithCTS::Event::CTS_REQUEST_TIMEOUT:
                    break;

            } //end switch
        }
    }

    void ArduinoDeviceManager::handleStreamRemoteEvent(StreamWithCTS *stream, byte evt){
        if(ADM == NULL){
            addErrorInfo(&outMessage, ErrorCode::NO_ADM_INSTANCE);
        } else {
            switch(evt){
                case (byte)StreamWithCTS::Event::RESET: //remote has reset
                    break;

                case (byte)StreamWithCTS::Event::CTS_TIMEOUT: //remote has waited too long for a CTS from local
                    break;

            } //end switch
            
        }
    }

    bool ArduinoDeviceManager::handleStreamReadyToReceive(StreamWithCTS *stream, bool request4cts){
        if(request4cts){
            return stream->canReceive(StreamWithCTS::UART_LOCAL_BUFFER_SIZE);
        } else {
            return stream->bytesToRead() == 0 && stream->canSend(frame.getMaxSize());
        }
    }

    void ArduinoDeviceManager::handleStreamReceive(StreamWithCTS *stream, int bytesToRead){
        
        //very first thing we do is ensure we have an instance
        if(ADM == NULL){
            addErrorInfo(&outMessage, ErrorCode::NO_ADM_INSTANCE);
        } else if(!outMessage.isEmpty()){
            return; //we already have an outgoing message so we defer to that
        } else {
            //we copy the 'tag' to the outmessage for identification at target
            outMessage.tag = inMessage.tag;

            //So we have an instance we can proeeed b first adding  all bytes to a frame 
            //(also removes from stream buffer so next byte block can be received)
            frame.reset();
            for(int i = 0; i < bytesToRead; i++){
                frame.add(stream->read());
            }

            //now we validate the frame and act depending on result
            if(frame.validate()){
                //ok so the data is valid now we convert to an ADM message
                inMessage.deserialize(frame.getPayload(), frame.getPayloadSize());
                if(inMessage.hasError()){
                    //Process error... we know message frames are ok so we return
                    addErrorInfo(&outMessage, ErrorCode::ADM_MESSAGE_ERROR, inMessage.error, &inMessage);
                } else { //ok so everything checks out ... let's get on with it by passing this to ADM
                    ADM->receiveMessage(&inMessage, &outMessage);
                }
            } else {
                //not valid so return an error...
                addErrorInfo(&outMessage, ErrorCode::MESSAGE_FRAME_ERROR, frame.error);
            }

        } //end check if there is an ADM instance
    }


    void ArduinoDeviceManager::handleStreamSend(StreamWithCTS *stream, int sendBufferRemaining){
        if(ADM == NULL){
            addErrorInfo(&outMessage, ErrorCode::NO_ADM_INSTANCE);
        } else if(outMessage.isEmpty()){
            ADM->sendMessage(&outMessage); //TODO: this is causing a crash so need to look into it
        }

        if(!outMessage.isEmpty()){
            outMessage.serialize();
            frame.reset();
            frame.setPayload(outMessage.getBytes(), outMessage.getByteCount());

            if(frame.getSize() <= sendBufferRemaining){
                stream->write(frame.getBytes(), frame.getSize(), true);
                outMessage.clear();
                frame.reset();
            } else {
                //TODO: something?
            }
        }
    }

    void ArduinoDeviceManager::addErrorInfo(ADMMessage *message, ErrorCode errorCode, byte subCode, ADMMessage *originalMessage){
        message->clear();
        message->type = ADMMessage::MessageType::TYPE_ERROR;
        message->addByte((byte)errorCode);
        message->addByte(subCode);
        if(originalMessage != NULL){
            message->addByte(originalMessage->type);
            message->addByte(originalMessage->target);
            message->addByte(originalMessage->sender);
        }
    }

    int ArduinoDeviceManager::getMaxFrameSize(){
        return frame.getMaxSize();
    }
     
    /*
    * Constructor
    */
    ArduinoDeviceManager::ArduinoDeviceManager(StreamWithCTS *stream){
        this->stream = stream;
    }

	ArduinoDeviceManager::~ArduinoDeviceManager(){
        for(int i = 0; i < deviceCount; i++){
            delete devices[i];
        }
    }

    void ArduinoDeviceManager::initialise(ADMMessage *message, ADMMessage *response){
        for(int i = 0; i < deviceCount; i++){
            delete devices[i];
        }
        deviceCount = 0;
        configured = false;
        initialised = true;

        response->type = ADMMessage::MessageType::TYPE_INITIALISE_RESPONSE;
        
    }
  
    void ArduinoDeviceManager::configure(ADMMessage *message, ADMMessage *response){
        configured = true;
        response->type = ADMMessage::MessageType::TYPE_CONFIGURE_RESPONSE;
    }

    ArduinoDevice *ArduinoDeviceManager::addDevice(byte id, byte category, char *dname){
        if(id == 0){
            error = ErrorCode::NO_DEVICE_ID;
            return NULL;
        }
        if(deviceCount >= MAX_DEVICES){
            error = ErrorCode::DEVICE_LIMIT_REACHED;
            return NULL;
        }
        if(getDevice(id) != NULL){
            error = ErrorCode::DEVICE_ID_ALREADY_USED;
            return NULL;
        }

        ArduinoDevice *device = NULL;
        int deviceIndex = inDevicesTable(dname);
        
	    switch (deviceIndex) {
#if (INCLUDE_DEVICES & TEMPERATURE_DEVICES) == TEMPERATURE_DEVICES
	        case 0:
		        device = new DS18B20Array(id, category, dname);
		        break;
#endif
#if (INCLUDE_DEVICES & RANGE_FINDER_DEVICES) == RANGE_FINDER_DEVICES
	        case 1:
		        device = new JSN_SR04T(id, category, dname);
		        break;
#endif
#if (INCLUDE_DEVICES & ELECTRICITY_MEASURING_DEVICES) == ELECTRICITY_MEASURING_DEVICES
	        case 2:
                device = new ZMPT101B(id, category, dname);
		        break;
#endif
#if (INCLUDE_DEVICES & DIAGNOSTIC_DEVICES) == DIAGNOSTIC_DEVICES
            case 3:
                device = new Test01(id, category, dname);
                break;
#endif

            default:
                switch(category){
#if (INCLUDE_DEVICES & IR_DEVICES) == IR_DEVICES
	                case ArduinoDevice::IR_RECEIVER:
		                device = new IRReceiver(id, category, dname);
		                break;

	                case ArduinoDevice::IR_TRANSMITTER:
		                device = new IRTransmitter(id, category, dname);
		                break;
#endif
	                case ArduinoDevice::COUNTER:
		                //device = new Counter(id, category, dname);
		                break;

                    default:
                        device = NULL;
                }
                break;
	    } //end device name switch
	
	    if (device == NULL) {
            device = new ArduinoDevice(id, category, dname);
	    }
        
        devices[deviceCount] = device;
        deviceCount++;
      
        return device;
    }

    ArduinoDevice *ArduinoDeviceManager::addDevice(ADMMessage *message){
        //TODO: name of device
        char deviceName[ArduinoDevice::DEVICE_NAME_LENGTH];
        message->argumentAsCharArray(getArgumentIndex(message, MessageField::DEVICE_NAME), deviceName);
        byte deviceCategory = message->argumentAsByte(getArgumentIndex(message, MessageField::DEVICE_CATEGORY)); 
        return addDevice(message->target, deviceCategory, deviceName);
    }

    ArduinoDevice* ArduinoDeviceManager::getDevice(byte deviceID){
        for(int i = 0; i < deviceCount; i++){
            if(devices[i]->getID() == deviceID)return devices[i];
        }
        return NULL;
    }

    void ArduinoDeviceManager::loop(){
        for(int i = 0; i < deviceCount; i++){
            if(devices[i]->isActive())devices[i]->loop();
        }
        
        stream->loop();
        /*stream->receive();
        stream->process();
        stream->send();*/
    }

    void ArduinoDeviceManager::receiveMessage(ADMMessage* message, ADMMessage* response){
        //find the device targeted by the message
        ErrorCode error = ErrorCode::NO_ERROR;
      
        if(message->target == ADM_TARGET_ID){ //targetting ADM
            response->sender = ADM_TARGET_ID;
            response->target = ADM_TARGET_ID;
            switch ((ADMMessage::MessageType)message->type) {
                case ADMMessage::MessageType::TYPE_INITIALISE:
                    initialise(message, response);
                    break;

                 case ADMMessage::MessageType::TYPE_CONFIGURE:
                    configure(message, response);
                    break;

                case ADMMessage::MessageType::TYPE_STATUS_REQUEST:
                    response->type = ADMMessage::MessageType::TYPE_STATUS_RESPONSE;
                    response->addULong(millis());
                    response->addInt(freeMemory());
                    response->addByte(deviceCount);
                    break;
            
                case ADMMessage::MessageType::TYPE_ECHO:
                    response->copy(message);
                    response->type = ADMMessage::MessageType::TYPE_ECHO_RESPONSE;
                    break;
            }
        } else if (message->target == STREAM_TARGET_ID){ //targetting stream
            response->sender = STREAM_TARGET_ID;
            response->target = STREAM_TARGET_ID;
            switch ((ADMMessage::MessageType)message->type) {
                case ADMMessage::MessageType::TYPE_STATUS_REQUEST:
                    response->type = ADMMessage::MessageType::TYPE_STATUS_RESPONSE;
                    response->sender = STREAM_TARGET_ID;
                    response->addBool(stream->isClearToSend());
                    response->addInt(stream->getBytesReceived());
                    response->addInt(stream->getBytesSent());
                    response->addInt(stream->bytesToRead());
                    response->addInt(stream->receiveBuffer->remaining());
                    response->addInt(stream->receiveBuffer->getMarkerCount());
                    response->addInt(stream->sendBuffer->remaining());
                    response->addInt(stream->sendBuffer->getMarkerCount());
                    response->addBool(stream->localRequestedCTS);
                    response->addBool(stream->remoteRequestedCTS);
                    break;
            }
        } else { //targetting device
            ArduinoDevice *device = getDevice(message->target);
            switch ((ADMMessage::MessageType)message->type) {
                case ADMMessage::MessageType::TYPE_INITIALISE:
                    if(device == NULL){ //we are creating a new device
                        device = addDevice(message);
                        if(device == NULL){
                            error = ErrorCode::DEVICE_CANNOT_BE_CREATED;
                            break;
                        }
                    }
                    break;
             }
             if(device != NULL){
                device->receiveMessage(message, response);
             } else {
                error = ErrorCode::DEVICE_NOT_FOUND;
             }
        }
        
        if(error != ErrorCode::NO_ERROR){
            response->clear(); //an error overrides any other response
            response->sender = ADM_TARGET_ID;
            response->target = ADM_TARGET_ID;
            addErrorInfo(response, error, 0, message);
        }
    }

    void ArduinoDeviceManager::sendMessage(ADMMessage* message){
        if(deviceCount > 0){
            ArduinoDevice *dev = devices[currentDevice];
            if(dev->isReady() && dev->isMessageReady()){
                dev->sendMessage(message); 
            }
            currentDevice = (currentDevice + 1) % deviceCount;
        }
    }

    int ArduinoDeviceManager::getArgumentIndex(ADMMessage* message, MessageField field){
        switch(field){
            case MessageField::DEVICE_NAME:
                return 0;
             case MessageField::DEVICE_CATEGORY:
                return 1;
            default:
                return (int)field;
        }
    }


    bool ArduinoDeviceManager::isReady(){
        return stream->isReady() && initialised && configured;
    }
} //end namespace