#include "ChetchUtils.h"
#include "ChetchADM.h"

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
    * Constructor
    */
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
    }

    int ArduinoDeviceManager::receiveMessage(ADMMessage* message){
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
            Serial.println("Fuck yes");
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
        return 0;
    }

    int ArduinoDeviceManager::sendMessage(byte *b){
        ArduinoDevice *dev = devices[currentDevice];
        int n = 0;
        if(dev->isMessageReady()){
            n = dev->sendMessage(b); 
        }
        currentDevice = (currentDevice + 1) % deviceCount;
        return n;
    }

} //end namespace