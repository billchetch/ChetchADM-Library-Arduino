#ifndef CHETCH_ADM_ADM_H
#define CHETCH_ADM_ADM_H

#include <Arduino.h>
#include "ChetchStreamFlowController.h"
#include "ChetchMessageFrame.h"
#include "ChetchADMMessage.h"
#include "ChetchArduinoDevice.h"

#if defined(ARDUINO_AVR_UNO)
	//Uno specific code
	#define MAX_DEVICES 8	
#elif defined(ARDUINO_AVR_MEGA2560)
	//Mega 2560 specific code
	#define MAX_DEVICES 32
#elif defined(ARDUINO_SAM_DUE)
	#define MAX_DEVICES 16
#else
    #define MAX_DEVICES 8
//#error Unsupported hardware
#endif

#define TEMPERATURE_DEVICES 1
#define RANGE_FINDER_DEVICES 2
#define IR_DEVICES 4
#define ELECTRICITY_MEASURING_DEVICES 8
#define DIAGNOSTIC_DEVICES 16

//!!!Specify here devices beyond the default set that should be included ... combinations allowed by OR-ing
//#define INCLUDE_DEVICES TEMPERATURE_DEVICES
//#define INCLUDE_DEVICES RANGE_FINDER_DEVICES
//#define INCLUDE_DEVICES ELECTRICITY_MEASURING_DEVICES
#define INCLUDE_DEVICES ELECTRICITY_MEASURING_DEVICES + DIAGNOSTIC_DEVICES

namespace Chetch{
    class ArduinoDeviceManager{
        private:
            //char* id;
            StreamFlowController *stream;
            ArduinoDevice *devices[MAX_DEVICES];
            byte deviceCount = 0;
            byte currentDevice = 0;
            bool initialised = false;
            bool configured = false;
            unsigned long unixTime = 0; //TODO: set in initialisation
            unsigned long ledMillis = 0;

            static ArduinoDeviceManager *ADM;
            static ADMMessage inMessage;
            static ADMMessage outMessage;
            static MessageFrame frame;

        public:
        
            enum class ErrorCode{
                NO_ERROR = 0,
                NO_ADM_INSTANCE = 1,
                MESSAGE_FRAME_ERROR = 10, //To indicate this is a Frame error
                ADM_MESSAGE_ERROR = 11,
                ADM_MESSAGE_IS_EMPTY= 12,
                NO_DEVICE_ID = 20,
                DEVICE_LIMIT_REACHED = 21,
                DEVICE_ID_ALREADY_USED = 22,
                DEVICE_NOT_FOUND = 23,
                DEVICE_CANNOT_BE_CREATED = 24,
                DEVICE_ERROR = 100, //To indicate this is an error from the device (not ADM)
            };
            ErrorCode error = ErrorCode::NO_ERROR;

            enum class MessageField{
                MILLIS = 0,
                MEMORY,
                DEVICE_COUNT,
                IS_READY,
                DEVICE_NAME,
                DEVICE_CATEGORY,
            };
    
            static const byte ADM_MESSAGE_SIZE = 50;
            static const byte ADM_TARGET_ID = 0;
            static const byte STREAM_TARGET_ID = 255;
            static const byte RESET_ADM_COMMAND = 201; //for use by ESP8266
            
            static int inDevicesTable(char *dname);
            static ArduinoDeviceManager *create(StreamFlowController *stream);
            static ArduinoDeviceManager *getInstance();
            static void handleStreamCommand(StreamFlowController *stream, byte cmd);
            static bool handleStreamLocalEvent(StreamFlowController *stream, byte cmd);
            static void handleStreamRemoteEvent(StreamFlowController *stream, byte cmd);
            static bool handleStreamReadyToReceive(StreamFlowController *stream, bool request4cts);
            static void handleStreamReceive(StreamFlowController *stream, int bytesToRead);
            static void handleStreamSend(StreamFlowController *stream, int sendBufferRemaining);
            static void addErrorInfo(ADMMessage *message, ErrorCode errorCode, byte subCode = 0, ADMMessage *originalMessage = NULL);
            static void send(StreamFlowController *stream, ADMMessage *message);
            static int getMaxFrameSize();

            ArduinoDeviceManager(StreamFlowController *stream);
            ~ArduinoDeviceManager();
            bool setup();

            void reset();
            virtual void initialise(ADMMessage *message, ADMMessage *response);
            virtual void configure(ADMMessage *message, ADMMessage *response);
            ArduinoDevice *addDevice(byte id, byte category, char *dname);
            ArduinoDevice *addDevice(ADMMessage *message);
            ArduinoDevice* getDevice(byte deviceID);
            void loop();
            virtual void receiveMessage(ADMMessage* message, ADMMessage* response);
            void sendMessage(ADMMessage* message);
            int getArgumentIndex(ADMMessage* message, MessageField field);

            bool isReady(); //connected, initialised and configured

            void flashLED(int interval, int diff, int blinkTime, int ledPin);
    }; //end class
} //end namespace

#endif