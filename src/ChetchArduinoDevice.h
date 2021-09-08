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
            
            enum MessageField{
                ENABLED = 0,
                REPORT_INTERVAL,
                DEVICE_NAME,
                DEVICE_CATEGORY,
                DEVICE_COMMAND,
            };

            enum DeviceCommand{
                NONE = 0,
                TEST = 1,
                ENABLE,
                DISABLE,
                START,
                STOP,
                PAUSE,
                RESET,
            };

        private:
            byte ID;
            byte category;
            char name[DEVICE_NAME_LENGTH];
    
            unsigned long lastMillis = 0;
            int reportInterval = -1; //negative or zero means no reporting
            ADMMessage::MessageType messageTypeToCreate = ADMMessage::MessageType::TYPE_NONE;

            bool initialised = false;
            bool configured = false;
            bool enabled = false;
            
        public:
            ArduinoDevice(byte id, byte category, char* dname);
            //~ArduinoDevice();

            virtual void initialise(ADMMessage *message, ADMMessage *response);
            virtual void configure(ADMMessage *message, ADMMessage *response);
            virtual DeviceCommand executeCommand(ADMMessage *message, ADMMessage *response);
            byte getID();
            char *getName();
            void enable(bool enable);
            bool isReady(); //initialised AND configured
            bool isActive(); //isReady AND enabled
            void setReportInterval(int interval);
            int getReportInterval();
            bool isMessageReady();
            virtual void receiveMessage(ADMMessage *message, ADMMessage *response);
            virtual void createMessage(ADMMessage::MessageType messageType, ADMMessage *message);
            void sendMessage(ADMMessage *message);
            int getArgumentIndex(ADMMessage *message, MessageField field);
            virtual void loop();
    };

} //end namespace

#endif