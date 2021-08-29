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
                ALARM = 7,
                VAC_SENSOR = 8,
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
            ADMMessage::MessageType messageTypeToCreate = ADMMessage::MessageType::TYPE_NONE;

            bool enabled = false;
            bool configured = false;

        public:
            ArduinoDevice(byte id, byte category, char* dname);
            //~ArduinoDevice();

            virtual void initialise(ADMMessage *message, ADMMessage *response);
            virtual void configure(ADMMessage *message, ADMMessage *response);
            byte getID();
            char *getName();
            void enable(bool enable);
            bool isActive();
            void setReportInterval(int interval);
            bool isMessageReady();
            virtual void receiveMessage(ADMMessage *message, ADMMessage *response);
            virtual void createMessage(ADMMessage::MessageType messageType, ADMMessage *message);
            void sendMessage(ADMMessage *message);
            virtual void loop();
    };

} //end namespace

#endif