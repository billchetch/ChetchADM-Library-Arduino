#ifndef CHETCH_ADM_DEVICE_H
#define CHETCH_ADM_DEVICE_H

#include <Arduino.h>
#include "ChetchADMMessage.h"


namespace Chetch{
	class ArduinoDevice{
        public:
            enum Category {
                DIAGNOSTICS = 1,
                IR_TRANSMITTER = 2,
                IR_RECEIVER = 3,
                TEMPERATURE_SENSOR = 4,
                COUNTER = 5,
                RANGE_FINDER = 6,
                ALARM = 7
            };

            static const byte DEVICE_NAME_LENGTH = 10;
            static const byte DEVICE_CATEGROY_INDEX = 0;
            static const byte DEVICE_NAME_INDEX = 1;
            static const byte DEVICE_ENABLED_INDEX = 2;
            static const byte REPORT_INTERVAL_INDEX = 3;
            
        private:
            byte ID;
            byte category;
            char name[DEVICE_NAME_LENGTH];
    
            unsigned long lastMillis = 0;
            int reportInterval = -1;
            byte messageToCreate = 0;

            bool enabled = false;

        public:
            ArduinoDevice(byte id, byte category, char* dname);
            //~ArduinoDevice();

            virtual void initialise(ADMMessage* message);
            byte getID();
            void enable(bool enable);
            void setReportInterval(int interval);
            bool isMessageReady();
            int receiveMessage(byte *message, byte *response);
            virtual int createMessage(byte messageType, byte *message);
            int sendMessage(byte *message);
            virtual void loop();
    };

} //end namespace

#endif