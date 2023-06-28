#ifndef CHETCH_ADM_DS18B20_H
#define CHETCH_ADM_DS18B20_H

#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <ChetchArduinoDevice.h>
#include <ChetchADMMessage.h>

namespace Chetch{
    class DS18B20Array : public ArduinoDevice {
        public:
            enum MessageField {
                ONE_WIRE_PIN = 2,
                SENSOR_RESOLUTION = 3,
            };

            enum class ErrorSubCode{
                NO_SENSORS_CONNECTED = 101,
            };

        private:
            byte oneWirePin = 0;
            byte numberOfSensors = 0;
            bool temperatureHasChanged = false; //if at least one temperature has changed since last read
            bool requestTemperatures = false;
            
            OneWire* oneWire = NULL;
            DallasTemperature* dallasTemp = NULL;
            uint8_t** deviceAddresses = NULL;
            float* temperatures = NULL;

        public: 
            DS18B20Array(byte id, byte cat, char* dn);
            ~DS18B20Array() override;

            bool createDallasTemperature(byte owPin, byte resolution, bool waitForConversion);
            int getArgumentIndex(ADMMessage* message, MessageField field);
            bool configure(ADMMessage* message, ADMMessage* response) override;
            void populateMessageToSend(byte messageID, ADMMessage* message) override;

            byte getNumberOfSensors();
            float* getTemperatures();
            void readTemperatures();
            void loop() override;
            

    }; //end class
} //end namespae
#endif