#include "ChetchUtils.h"
#include "ChetchADM.h"

const char DS18B20[] PROGMEM = "DS18B20";
const char JSN_SR04T[] PROGMEM = "JSN-SR04T";

const char *const DEVICES_TABLE[] PROGMEM = {
	DS18B20,
	JSN_SR04T
};

namespace Chetch{
	ArduinoDeviceManager::~ArduinoDeviceManager(){
        for(int i = 0; i < deviceCount; i++){
            delete devices[i];
        }
    }
    
    void ArduinoDeviceManager::initialise(ADMMessage *message){
        initialised = true;
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

        ArduinoDevice *device = new ArduinoDevice(id, category, dname);
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
            devices[i]->loop();
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
                Serial.print("Initialised device "); Serial.println(device->getName());
            }
            break;

        case ADMMessage::TYPE_CONFIGURE:
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