#include "ChetchUtils.h"
#include "ChetchADM.h"
#include "ChetchMessageFrame.h"

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
#include "devices/ChetchZMPT101B.h"
#endif

const char DS18B20[] PROGMEM = "DS18B20";
const char JSN_SR04T[] PROGMEM = "JSN-SR04T";
const char ZMPT101B[] PROGMEM = "ZMPT101B";

const char *const DEVICES_TABLE[] PROGMEM = {
	DS18B20,
	JSN_SR04T,
    ZMPT101B
};

namespace Chetch{
    ArduinoDeviceManager *ArduinoDeviceManager::ADM;    

    int ArduinoDeviceManager::inDevicesTable(char *dname){
        char stBuffer[ArduinoDevice::DEVICE_NAME_LENGTH];
        for(int i = 0; i < 3; i++){
            if (strcmp(dname, Utils::getStringFromProgmem(stBuffer, i, DEVICES_TABLE)) == 0) {
			    return i;
		    } 
        }
        return -1;
    }

    /*
    * Create static instance because stream callbacks cannot (not easy to?) use instance member methods
    */
    ArduinoDeviceManager *ArduinoDeviceManager::create(char *id, StreamWithCTS *stream){
        if(ADM == NULL){
            stream->setResetHandler(handleStreamReset);
            stream->setReceiveHandler(handleStreamReceive);
            stream->setSendHandler(handleStreamSend);
            ADM = new ArduinoDeviceManager(id, stream);
        }
        return ADM;
    }


    ArduinoDeviceManager *ArduinoDeviceManager::getInstance(){
        return ADM;
    }

    void ArduinoDeviceManager::handleStreamReset(StreamWithCTS *stream){
        
    }

    void ArduinoDeviceManager::handleStreamReceive(StreamWithCTS *stream, int bytesToRead){
        MessageFrame f(MessageFrame::FrameSchema::SMALL_SIMPLE_CHECKSUM, ADM_MESSAGE_SIZE);
        ADMMessage message(ADM_MESSAGE_SIZE);

        //add all bytes to a frame (also removes from stream buffer so next byte block can be received)
        for(int i = 0; i < bytesToRead; i++){
            f.add(stream->read());
        }

        //now we validate the frame and act depending on result
        if(f.validate()){
            //ok so the data is valid now we convert to an ADM message
            message.deserialize(f.getPayload(), f.getPayloadSize());
            if(message.hasError()){
                //Process error... we know message frames are ok so we return

            } else {
               //ok so everything checks out ... let's get on with it'
               stream->sendEvent(StreamWithCTS::Event::ALL_OK);
            }
        } else {
            //not valid so return an error...
            //TODO: return an ADMMessage...
            byte eventByte = 0;
            switch(f.error){
                case MessageFrame::FrameError::CHECKSUM_FAILED:
                    eventByte = StreamWithCTS::Event::CHECKSUM_FAILED;
                    break;

                default:
                    eventByte = StreamWithCTS::Event::UNKNOWN_ERROR;
                    break;
                    
            }
            stream->sendEvent(eventByte);
        }
    }

    void ArduinoDeviceManager::handleStreamSend(StreamWithCTS *stream){

    }
     
    /*
    * Constructor
    */
    ArduinoDeviceManager::ArduinoDeviceManager(char* id, StreamWithCTS *stream){
        this->id = id;
        this->stream = stream;
    }

	ArduinoDeviceManager::~ArduinoDeviceManager(){
        for(int i = 0; i < deviceCount; i++){
            delete devices[i];
        }
    }

    void ArduinoDeviceManager::initialise(ADMMessage *message){
        initialised = true;
    }
  
    void ArduinoDeviceManager::configure(ADMMessage *message){
        configured = true;
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
        if(message->hasArgument(ArduinoDevice::DEVICE_NAME_INDEX)){
            char deviceName[ArduinoDevice::DEVICE_NAME_LENGTH];
            message->argumentAsCharArray(ArduinoDevice::DEVICE_NAME_INDEX, deviceName);
            return addDevice(message->target, message->argumentAsByte(ArduinoDevice::DEVICE_CATEGROY_INDEX), deviceName);
        } else {
            return addDevice(message->target, message->argumentAsByte(ArduinoDevice::DEVICE_CATEGROY_INDEX), NULL);
        }
        
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

        stream->receive();
        stream->process();
        stream->send();
    }

    void ArduinoDeviceManager::receiveMessage(ADMMessage* message){
        //find the device targeted by the message
        ArduinoDevice *device = message->target == 0 ? NULL : getDevice(message->target);
      
        switch ((ADMMessage::MessageType)message->type) {
        case ADMMessage::TYPE_INITIALISE:
            if(message->target == 0){ //means we are targetting the board
                initialise(message);
            } else {
                if(device == NULL){ //we are creating a new device
                    device = addDevice(message);
                    if(device == NULL){
                    //TODO: handle error
                    break;
                    }
                } 
                device->initialise(message);
            }
            break;

        case ADMMessage::TYPE_CONFIGURE:
             if(message->target == 0){ //means we are targetting the board
                configure(message);
            } else {
                if(device == NULL){ //TODO: this is an error condition so handle
                    break;
                } 
                device->configure(message);
            }
            break;
          
        }
    }

    void ArduinoDeviceManager::sendMessage(ADMMessage* message){
        ArduinoDevice *dev = devices[currentDevice];
        if(dev->isMessageReady()){
            dev->sendMessage(message); 
        }
        currentDevice = (currentDevice + 1) % deviceCount;
    }

} //end namespace