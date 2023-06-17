#ifndef CHETCH_ADM_ADM_H
#define CHETCH_ADM_ADM_H

#include <Arduino.h>
#include "ChetchADMConfig.h"
#include "ChetchStreamFlowController.h"
#include "ChetchMessageFrame.h"
#include "ChetchADMMessage.h"
#include "ChetchArduinoDevice.h"
#include "ChetchADC.h"

namespace Chetch{

    class ArduinoDeviceManager{
        public:
        
            enum class ErrorCode{
                NO_ERROR = 0,
                NO_ADM_INSTANCE = 1,
                MESSAGE_FRAME_ERROR = 10, //To indicate this is a Frame error
                ADM_MESSAGE_ERROR = 11,
                ADM_MESSAGE_IS_EMPTY= 12,
                ADM_FAILED_TO_INITIALISE = 13,
                ADM_FAILED_TO_CONFIUGRE = 14,
                NO_DEVICE_ID = 20,
                DEVICE_LIMIT_REACHED = 21,
                DEVICE_ID_ALREADY_USED = 22,
                DEVICE_NOT_FOUND = 23,
                DEVICE_CANNOT_BE_CREATED = 24,
                DEVICE_ERROR = 100, //To indicate this is an error from the device (not ADM)
            };
            ErrorCode error = ErrorCode::NO_ERROR;

            enum class AttachmentMode{
                NOT_SET,
                MASTER_SLAVE,
                OBSERVER_OBSERVED,
            };

            enum class MessageField{
                MILLIS = 0,
                MEMORY,
                DEVICE_COUNT,
                IS_READY,
                DEVICE_NAME,
                DEVICE_CATEGORY,
                ATTACH_MODE,
                TOTAL_DEVICES,
                ANALOG_REFERENCE,
            };
    
            
            static const byte ADM_MESSAGE_SIZE = 50;
            static const byte ADM_TARGET_ID = 0;
            static const byte STREAM_TARGET_ID = 255;
            static const byte RESET_ADM_COMMAND = 201; //for use by ESP8266
            
            static byte statusIndicatorPin;

        private:
            
            
            static ArduinoDeviceManager *ADM;
            static ADMMessage inMessage;
            static ADMMessage outMessage;
            static MessageFrame frame;
        
        public:
            static int inDevicesTable(char *dname);
            static ArduinoDeviceManager *create(StreamFlowController *stream);
            static ArduinoDeviceManager *getInstance();
            static void handleStreamCommand(StreamFlowController *stream, byte cmd);
            static bool handleStreamLocalEvent(StreamFlowController *stream, byte cmd);
            static void handleStreamRemoteEvent(StreamFlowController *stream, byte cmd);
            static bool handleStreamReadyToReceive(StreamFlowController *stream);
            static void handleStreamReceive(StreamFlowController *stream, int bytesToRead);
            static void handleStreamSend(StreamFlowController *stream, int sendBufferRemaining);
            static void addErrorInfo(ADMMessage *message, ErrorCode errorCode, byte subCode = 0, ADMMessage *originalMessage = NULL);
            static void send(StreamFlowController *stream, ADMMessage *message);
            static int getMaxFrameSize();
        
        private:
            StreamFlowController *stream = NULL;
            ArduinoDevice *devices[MAX_DEVICES];
            
            //set in initialise by remote ADM
            AttachmentMode attachMode = AttachmentMode::MASTER_SLAVE;
            byte totalDevices = 0; //device count should reach this value indicating end of device initi+config process
            CADC::AnalogReference aref = CADC::AnalogReference::AREF_INTERNAL;

            byte deviceCount = 0;
            byte currentDevice = 0;
            bool initialised = false;
            bool configured = false;
            unsigned long unixTime = 0; //TODO: set in initialisation
            unsigned long ledMillis = 0;
            unsigned long loopDuration = 0; //the duration in micros of the last loop

            unsigned long messagesReceived = 0;
            unsigned long messagesSent = 0;

        public:
            ArduinoDeviceManager(StreamFlowController *stream);
            ~ArduinoDeviceManager();
            bool setup();

            void reset();
            virtual void initialise(AttachmentMode attachMode, byte totalDevices, CADC::AnalogReference aref);
            void initialise(ADMMessage *message, ADMMessage *response);
            virtual void configure();
            void configure(ADMMessage *message, ADMMessage *response);
            void onDevicesReady();

            ArduinoDevice *addDevice(byte id, byte category, char *dname);
            ArduinoDevice *addDevice(ADMMessage *message);
            ArduinoDevice* getDevice(byte deviceID);
            byte getDeviceCount();
            
            void loop();
            virtual void receiveMessage(ADMMessage* message, ADMMessage* response);
            void receivedMessage(ADMMessage* message);
            
            void sendMessage(ADMMessage* message);
            void sentMessage(ADMMessage* message);

            int getArgumentIndex(ADMMessage* message, MessageField field);

            bool isReady(); //connected, initialised and configured

            void indicateStatus();
            void flashLED(int interval, int diff, int blinkTime, int ledPin);

            int getFreeMemory();

            StreamFlowController *getStream();

            unsigned long getBytesReceived();
            unsigned long getBytesSent();

            unsigned long getMessagesReceived();
            unsigned long getMessagesSent();
    }; //end class
} //end namespace

#endif