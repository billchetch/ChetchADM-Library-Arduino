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

    bool DS18B20Array::configure(ADMMessage* message, ADMMessage* response){
        if(!ArduinoDevice::configure(message, response))return false;

        oneWirePin = message->argumentAsByte(getArgumentIndex(message, MessageField::ONE_WIRE_PIN));
        oneWire = new OneWire(oneWirePin);
        dallasTemp = new DallasTemperature(oneWire);
        dallasTemp->begin();
        int resolution = message->argumentAsInt(getArgumentIndex(message, MessageField::SENSOR_RESOLUTION));
        dallasTemp->setResolution(resolution);
        dallasTemp->setWaitForConversion(false);

        numberOfSensors = dallasTemp->getDeviceCount();

        if (numberOfSensors == 0)return false;

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

        readTemperatures = true;
        
        response->addByte(numberOfSensors);
        
        return true;
    }

    void DS18B20Array::createMessageToSend(byte messageID, ADMMessage* message){
        ArduinoDevice::createMessageToSend(messageID, message);

        if(messageID == ArduinoDevice::MESSAGE_ID_REPORT){
            readTemperatures = true;
            bool diff = false;
            for (byte i = 0; i < numberOfSensors; i++) {
                float t = dallasTemp->getTempC(deviceAddresses[i]);
                if (t != temperatures[i]) {
                    temperatures[i] = t;
                    diff = true;
                }
                message->addFloat(t);
            }
            //if (!diff)message->clear();
        }
    }

    void DS18B20Array::loop(){
        ArduinoDevice::loop();

        if (readTemperatures) {
            dallasTemp->requestTemperatures();
            readTemperatures = false;
        }
    }

} //end namespace
