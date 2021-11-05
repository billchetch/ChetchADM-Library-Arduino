#ifndef CHETCH_ADM_DEVICE_H
#define CHETCH_ADM_DEVICE_H

#include <Arduino.h>
#include "ChetchADMMessage.h"


namespace Chetch{
    class ArduinoDeviceManager;

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
                SWITCH = 9,
                SERVO = 10,
                MOTOR = 11,
            };

            enum MessageField{
                ENABLED = 0,
                REPORT_INTERVAL = 1, //measured in millis
                DEVICE_COMMAND,
            };

            enum  DeviceCommand{
                NONE = 0,
                COMPOUND,
                TEST,
                ENABLE,
                DISABLE,
                SET_REPORT_INTERVAL,
                START,
                STOP,
                PAUSE,
                RESET,
                ON,
                OFF,
                MOVE,
                ROTATE,
                PRINT,
                SET_CURSOR,
                DIZPLAY, //changed S to Z to avoid a define constant name clash
                CLEAR,
            };

            enum class ErrorCode{
                INVALID_COMMAND = 1,
            };


            static const byte DEVICE_NAME_LENGTH = 10;
            static const byte MESSAGE_QUEUE_LENGTH = 8;
            static const byte MESSAGE_ID_REPORT = 1;

        private:
            byte ID;
            byte category;
            char name[DEVICE_NAME_LENGTH];
    
            unsigned long lastMillis = 0;
            int reportInterval = -1; //measured in millis. negative or zero means no reporting
            unsigned long timerInterval = 0; //in micros. set this to a positive value to register with timer events
            int timerTicks = 0; //this is calculated and set by ADm based on timer Hz and timerInterval value
            
            byte messageQueue[MESSAGE_QUEUE_LENGTH];
            

            bool initialised = false;
            bool configured = false;
            bool enabled = false;

        public:
            ArduinoDeviceManager *ADM = NULL;
            byte messageCount = 0;
            
            ArduinoDevice(byte id, byte category, char* dname);
            virtual ~ArduinoDevice(); //to allow for polymorphic deletion

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
            bool hasMessageToSend(); //if there is a message in the queue or not
            virtual void receiveMessage(ADMMessage *message, ADMMessage *response);
            virtual void createMessage(ADMMessage::MessageType messageType, ADMMessage *message);
            virtual void createMessageToSend(byte messageID, ADMMessage *message);
            bool enqueueMessageToSend(byte messageID);
            byte dequeueMessageToSend();
            void addErrorInfo(ADMMessage *message, ErrorCode errorCode, ADMMessage *originalMessage = NULL);
            void sendMessage(ADMMessage *message);
            int getArgumentIndex(ADMMessage *message, MessageField field);
            virtual void loop();
            virtual void onTimer();
            virtual void onPauseTimer();
            virtual void onResumeTimer();
    };

} //end namespace

#endif