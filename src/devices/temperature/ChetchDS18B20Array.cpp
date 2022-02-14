#include "ChetchUtils.h"
#include "ChetchDS18B20Array.h"

namespace Chetch{
    
    DS18B20Array::DS18B20Array(byte id, byte cat, char *dn) : ArduinoDevice(id, cat, dn){
        //empty
    }

    DS18B20Array::~DS18B20Array() {
        if (dallasTemp != NULL)delete dallasTemp;
        if (oneWire != NULL)delete oneWire;
        if (numberOfSensors > 0) {
            for (byte i = 0; i < numberOfSensors; i++) {
                delete[] deviceAddresses[i];
            }
            delete[] deviceAddresses;
            delete[] temperatures;
        }
    }

    int DS18B20Array::getArgumentIndex(ADMMessage* message, MessageField field) {
        switch (field) {
            default:
                return (int)field;
        }
    }

    bool DS18B20Array::createDallasTemperature(byte owPin, byte resolution, bool waitForConversion){
        oneWirePin = owPin;
        oneWire = new OneWire(oneWirePin);
        dallasTemp = new DallasTemperature(oneWire);
        dallasTemp->begin();

        dallasTemp->setResolution(resolution);
        dallasTemp->setWaitForConversion(waitForConversion);

        numberOfSensors = dallasTemp->getDeviceCount();

        if(numberOfSensors == 0)return false;

        deviceAddresses = new uint8_t * [numberOfSensors];
        temperatures = new float[numberOfSensors];

        DeviceAddress tempDeviceAddress;
        for (byte i = 0; i < numberOfSensors; i++) {
            dallasTemp->getAddress(tempDeviceAddress, i);
            deviceAddresses[i] = new uint8_t[8];
            for (byte j = 0; j < 8; j++) {
                deviceAddresses[i][j] = tempDeviceAddress[j];
            }
            temperatures[i] = 0.0f;
        }
        return true;
    }

    bool DS18B20Array::configure(ADMMessage* message, ADMMessage* response){
        if(!ArduinoDevice::configure(message, response))return false;

        if(!createDallasTemperature(
            message->argumentAsByte(getArgumentIndex(message, MessageField::ONE_WIRE_PIN)),
            message->argumentAsByte(getArgumentIndex(message, MessageField::SENSOR_RESOLUTION)),
            false
            ))return false;

        requestTemperatures = true;
        
        response->addByte(numberOfSensors);
        response->addBool(false); //this is wait for conversion (always false)
        
        return true;
    }

    void DS18B20Array::populateMessageToSend(byte messageID, ADMMessage* message){
        ArduinoDevice::populateMessageToSend(messageID, message);

        if(messageID == ArduinoDevice::MESSAGE_ID_REPORT){
            requestTemperatures = true; //request again
            readTemperatures();
            for(byte i = 0; i < numberOfSensors; i++) {
                message->addFloat(temperatures[i]);
            }
            //if (!diff)message->clear();
        }
    }

    byte DS18B20Array::getNumberOfSensors(){
        return numberOfSensors;
    }

    float* DS18B20Array::getTemperatures(){
        return temperatures;
    }

    void DS18B20Array::readTemperatures(){
        temperatureHasChanged = false;
        for (byte i = 0; i < numberOfSensors; i++) {
            float t = dallasTemp->getTempC(deviceAddresses[i]);
            if (t != temperatures[i]) {
                temperatures[i] = t;
                temperatureHasChanged = true;
            }
        }
    }

    void DS18B20Array::loop(){
        ArduinoDevice::loop();

        if (requestTemperatures) {
            dallasTemp->requestTemperatures();
            requestTemperatures = false;
        }
    }

} //end namespace
