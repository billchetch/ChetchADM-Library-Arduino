#include "ChetchUtils.h"
#include "ChetchADM.h"
#include "ChetchMessageFrame.h"
#include <MemoryFree.h>

//Generic devices to always include here
#include "devices/ChetchSwitchDevice.h"


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

#if (INCLUDE_DEVICES & MOTOR_DEVICES) == MOTOR_DEVICES
#include "devices/motors/ChetchServoController.h"
#endif

#if (INCLUDE_DEVICES & DISPLAY_DEVICES) == DISPLAY_DEVICES
#include "devices/displays/ChetchLCD.h"
#endif

const char DS18B20[] PROGMEM = "DS18B20";
const char JSN_SR04T[] PROGMEM = "JSN-SR04T";
const char ZMPT101B[] PROGMEM = "ZMPT101B";
const char LCD[] PROGMEM = "LCD";
const char TEST01[] PROGMEM = "TEST01";
const char TEST02[] PROGMEM = "TEST02";

const char *const DEVICES_TABLE[] PROGMEM = {
	DS18B20,
	JSN_SR04T,
    ZMPT101B,
    LCD,
    TEST01,
    TEST02
};

#define DEVICE_TABLE_SIZE 5

namespace Chetch{
    ArduinoDeviceManager *ArduinoDeviceManager::ADM = NULL;
    MessageFrame ArduinoDeviceManager::frame(MessageFrame::FrameSchema::SMALL_SIMPLE_CHECKSUM, ADM_MESSAGE_SIZE);
    ADMMessage ArduinoDeviceManager::inMessage(ADM_MESSAGE_SIZE);
    ADMMessage ArduinoDeviceManager::outMessage(ADM_MESSAGE_SIZE);
    byte ArduinoDeviceManager::statusIndicatorPin = LED_BUILTIN;

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
    ArduinoDeviceManager *ArduinoDeviceManager::create(StreamFlowController *stream){
        if(ADM == NULL){
            stream->setCommandHandler(handleStreamCommand);
            stream->setEventHandlers(handleStreamLocalEvent, handleStreamRemoteEvent);
            stream->setReadyToReceiveHandler(handleStreamReadyToReceive);
            stream->setReceiveHandler(handleStreamReceive);
            stream->setSendHandler(handleStreamSend);
            stream->setMaxDatablockSize(frame.getMaxSize());
            ADM = new ArduinoDeviceManager(stream);
            if(statusIndicatorPin > 0){
                pinMode(statusIndicatorPin, OUTPUT);
            }
        }
        

        return ADM;
    }


    ArduinoDeviceManager *ArduinoDeviceManager::getInstance(){
        return ADM;
        
    }

    void ArduinoDeviceManager::handleStreamCommand(StreamFlowController *stream, byte cmd){
        if(ADM == NULL){
            addErrorInfo(&outMessage, ErrorCode::NO_ADM_INSTANCE);
        } else {
            switch(cmd){
                case RESET_ADM_COMMAND: //for use by ESP8266
                    ADM->reset();
                    break;
            }
        }
    }

    bool ArduinoDeviceManager::handleStreamLocalEvent(StreamFlowController *stream, byte evt){
        if(ADM == NULL){
            addErrorInfo(&outMessage, ErrorCode::NO_ADM_INSTANCE);
        } else {
            switch(evt){
                case (byte)StreamFlowController::Event::RESET:
                    outMessage.clear();
                    break;

                case (byte)StreamFlowController::Event::CTS_TIMEOUT: //local has waited too long for a CTS from remote
                    break;

                case (byte)StreamFlowController::Event::RECEIVE_BUFFER_FULL:
                case (byte)StreamFlowController::Event::MAX_DATABLOCK_SIZE_EXCEEDED:
                case (byte)StreamFlowController::Event::CTS_REQUEST_TIMEOUT:
                    break;

            } //end switch
        }
        return true; //false will stop the event being sent
    }

    void ArduinoDeviceManager::handleStreamRemoteEvent(StreamFlowController *stream, byte evt){
        if(ADM == NULL){
            addErrorInfo(&outMessage, ErrorCode::NO_ADM_INSTANCE);
        } else {
            switch(evt){
                case (byte)StreamFlowController::Event::RESET: //remote has reset
                    break;

                case (byte)StreamFlowController::Event::CTS_TIMEOUT: //remote has waited too long for a CTS from local
                    break;

            } //end switch
            
        }
    }

    bool ArduinoDeviceManager::handleStreamReadyToReceive(StreamFlowController *stream, bool request4cts){
        if(request4cts){
            return stream->canReceive(StreamFlowController::UART_LOCAL_BUFFER_SIZE);
        } else {
            return stream->bytesToRead() == 0 && stream->canSend(frame.getMaxSize());
        }
    }

    void ArduinoDeviceManager::handleStreamReceive(StreamFlowController *stream, int bytesToRead){
        
        //very first thing we do is ensure we have an instance
        if(ADM == NULL){
            addErrorInfo(&outMessage, ErrorCode::NO_ADM_INSTANCE);
        } else if(!outMessage.isEmpty()){
            return; //we already have an outgoing message so we defer to that
        } else {
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
                    //we copy the 'tag' to the outmessage for identification at target
                    outMessage.tag = inMessage.tag;
                    ADM->receiveMessage(&inMessage, &outMessage);
                }
            } else {
                //not valid so return an error...
                addErrorInfo(&outMessage, ErrorCode::MESSAGE_FRAME_ERROR, frame.error);
            }

        } //end check if there is an ADM instance
    }


    void ArduinoDeviceManager::handleStreamSend(StreamFlowController *stream, int sendBufferRemaining){
        if(ADM == NULL){
            addErrorInfo(&outMessage, ErrorCode::NO_ADM_INSTANCE);
        } else if(outMessage.isEmpty()){
            ADM->sendMessage(&outMessage);
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
                outMessage.clear();
                frame.reset();
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
    ArduinoDeviceManager::ArduinoDeviceManager(StreamFlowController *stream){
        this->stream = stream;
    }

	ArduinoDeviceManager::~ArduinoDeviceManager(){
        for(int i = 0; i < deviceCount; i++){
            delete devices[i];
        }
    }

    void ArduinoDeviceManager::reset(){
        for(int i = 0; i < deviceCount; i++){
            delete devices[i];
        }
        totalDevices = 0;
        deviceCount = 0;
        configured = false;
        initialised = false;
        outMessage.clear();
        inMessage.clear();
        stream->end();
    }

    void ArduinoDeviceManager::initialise(ADMMessage *message, ADMMessage *response){
        AttachmentMode amode = (AttachmentMode)message->argumentAsByte(getArgumentIndex(message, MessageField::ATTACH_MODE));
        if(amode != attachMode)return;

        if(attachMode != AttachmentMode::OBSERVER_OBSERVED){
            initialise(
                    attachMode,
                    message->argumentAsByte(getArgumentIndex(message, MessageField::TOTAL_DEVICES)),
                    (CADC::AnalogReference)message->argumentAsByte(getArgumentIndex(message, MessageField::ANALOG_REFERENCE))
                );
        }

        if(!initialised)return;

        response->type = ADMMessage::MessageType::TYPE_INITIALISE_RESPONSE;
        response->addString(BOARD_NAME);
        response->addByte(MAX_DEVICES);

        //we add this data incase the attachment mode is not MASTER_SLAVE
        response->addByte((byte)attachMode);
        response->addByte((byte)CADC::aref());
    }

    void ArduinoDeviceManager::initialise(AttachmentMode attachMode, byte totalDevices, CADC::AnalogReference aref){
        this->attachMode = attachMode;
        this->totalDevices = totalDevices;
        this->aref = aref;

        for(int i = 0; i < deviceCount; i++){
            delete devices[i];
        }
        deviceCount = 0;
        configured = false;
        initialised = true;
        
        CADC::init(aref);
    }
  
    void ArduinoDeviceManager::configure(){
        configured = true;
    }
        
    void ArduinoDeviceManager::configure(ADMMessage *message, ADMMessage *response){
        if(attachMode != AttachmentMode::OBSERVER_OBSERVED){
            configure();
        }

        if(!configured)return;

        response->type = ADMMessage::MessageType::TYPE_CONFIGURE_RESPONSE;
    }

    void ArduinoDeviceManager::onDevicesReady(){
        //Not yet used
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
                device = ZMPT101B::create(id, category, dname); //use factory method because ZMPT101B uses ISR so instances need to be regulated
		        break;
#endif
#if (INCLUDE_DEVICES & DISPLAY_DEVICES) == DISPLAY_DEVICES
            case 3:
                device = new LCD(id, category, dname);
                break;
#endif
#if (INCLUDE_DEVICES & DIAGNOSTIC_DEVICES) == DIAGNOSTIC_DEVICES
            case 4:
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
#if (INCLUDE_DEVICES & MOTOR_DEVICES) == MOTOR_DEVICES
                    case ArduinoDevice::SERVO:
                        device = new ServoController(id, category, dname);
                        break;
#endif
	                case ArduinoDevice::COUNTER:
		                //device = new Counter(id, category, dname);
		                break;

                    case ArduinoDevice::SWITCH:
		                device = new SwitchDevice(id, category, dname);
		                break;

                    default:
                        device = NULL;
                }
                break;
	    } //end device name switch
	
	    if (device != NULL) {
            devices[deviceCount] = device;
            deviceCount++;
        }
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

    byte ArduinoDeviceManager::getDeviceCount(){
        return deviceCount;
    }

    void ArduinoDeviceManager::flashLED(int interval, int diff, int blinkTime, int ledPin){
        if(diff >= interval + blinkTime){
            digitalWrite(ledPin, LOW);
        } else if(diff >= interval){
            digitalWrite(ledPin, HIGH);
        }
    }

    void ArduinoDeviceManager::indicateStatus(){
        if(statusIndicatorPin > 0){
            unsigned long diff = millis() - ledMillis;
            flashLED(0, diff, 500, statusIndicatorPin);
            if(stream == NULL || !stream->isReady())flashLED(750, diff, 100, statusIndicatorPin);
            if(!initialised)flashLED(1500, diff, 100, statusIndicatorPin);
            if(!configured)flashLED(2250, diff, 100, statusIndicatorPin);
            if(diff > 5000){
                ledMillis = millis();
            }
        }
    }

    void ArduinoDeviceManager::loop(){
        //led for status
        indicateStatus();
        
        //loop each active device
        if(isReady()){
            for(int i = 0; i < deviceCount; i++){
                if(devices[i]->isActive()){
                    devices[i]->loop();
                }
            }
        }

        //loop stream: Here is where the receiveMessage and sendMessage instance methods will be called via
        //the static handleStreamReceive and handStreamSend callback meethods given to the Stream in create above
        if(stream->hasBegun()){
            stream->loop();
        }
    }

    void ArduinoDeviceManager::receiveMessage(ADMMessage* message, ADMMessage* response){
        //find the device targeted by the message
        ErrorCode error = ErrorCode::NO_ERROR;
      
        if(message->target == ADM_TARGET_ID){ //targetting ADM
            response->sender = ADM_TARGET_ID;
            response->target = ADM_TARGET_ID;
            switch ((ADMMessage::MessageType)message->type) {
                case ADMMessage::MessageType::TYPE_RESET:
                    reset();
                    break;

                case ADMMessage::MessageType::TYPE_INITIALISE:
                    initialise(message, response);
                    if(!initialised)error = ErrorCode::ADM_FAILED_TO_INITIALISE;
                    break;

                 case ADMMessage::MessageType::TYPE_CONFIGURE:
                    configure(message, response);
                    if(!configured)error = ErrorCode::ADM_FAILED_TO_CONFIUGRE;
                    break;

                case ADMMessage::MessageType::TYPE_STATUS_REQUEST:
                    response->type = ADMMessage::MessageType::TYPE_STATUS_RESPONSE;
                    response->addULong(millis());
                    response->addInt(freeMemory());
                    response->addBool(initialised);
                    response->addBool(configured);
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
                    if(device == NULL && deviceCount < totalDevices){ //we are creating a new device
                        device = addDevice(message);
                        device->ADM = this;
                    }
                    if(device == NULL){
                        error = ErrorCode::DEVICE_CANNOT_BE_CREATED;
                    }
                    break;
             }

             if(device != NULL){
                //Let device process the message
                device->receiveMessage(message, response);

                //If the device responds with a configure response then see if it is the last one and if it is
                //then all devices are ready so we call onDevicesReady
                if(response->type == ADMMessage::MessageType::TYPE_CONFIGURE_RESPONSE && deviceCount == totalDevices){
                   bool allReady = true;
                   for(byte i = 0; i < deviceCount; i++){
                        if(!devices[i]->isReady()){
                            allReady = false;
                            break;
                        }
                    }
                    if(allReady)onDevicesReady();
                }
             } else if(error == ErrorCode::NO_ERROR) {
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
            if(dev->isReady() && dev->hasMessageToSend()){
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
            case MessageField::ATTACH_MODE:
                return 0;
            case MessageField::TOTAL_DEVICES:
                return 1;
            case MessageField::ANALOG_REFERENCE:
                return 2;

            default:
                return (int)field;
        }
    }


    bool ArduinoDeviceManager::isReady(){
        if(attachMode == AttachmentMode::OBSERVER_OBSERVED){
            return initialised && configured;
        } else {
            return stream->isReady() && initialised && configured;
        }
    }

} //end namespace