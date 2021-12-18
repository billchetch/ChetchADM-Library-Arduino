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

        private:
            byte oneWirePin = 0;
            byte numberOfSensors = 0;
            bool readTemperatures = false;

            OneWire* oneWire = NULL;
            DallasTemperature* dallasTemp = NULL;
            uint8_t** deviceAddresses = NULL;
            float* temperatures = NULL;

        public: 
            DS18B20Array(byte id, byte cat, char* dn);
            ~DS18B20Array() override;

            int getArgumentIndex(ADMMessage* message, MessageField field);
            bool configure(ADMMessage* message, ADMMessage* response) override;
            void createMessageToSend(byte messageID, ADMMessage* message) override;
            void loop() override;
            

    }; //end class
} //end namespae
#endif