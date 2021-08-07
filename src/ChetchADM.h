#ifndef CHETCH_ADM_ADM_H
#define CHETCH_ADM_ADM_H

#include <Arduino.h>
#include "ChetchStreamWithCTS.h"
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
#error Unsupported hardware
#endif

#define TEMPERATURE_DEVICES 1
#define RANGE_FINDER_DEVICES 2
#define IR_DEVICES 4
#define ELECTRICITY_MEASURING_DEVICES 8

//specify here devices beyond the default set that should be included ... combinations allowed by OR-ing
//#define INCLUDE_DEVICES TEMPERATURE_DEVICES
//#define INCLUDE_DEVICES RANGE_FINDER_DEVICES
#define INCLUDE_DEVICES ELECTRICITY_MEASURING_DEVICES

namespace Chetch{
    class ArduinoDeviceManager{
        private:
            char* id;
            StreamWithCTS *stream;
            ArduinoDevice *devices[MAX_DEVICES];
            byte deviceCount = 0;
            byte currentDevice = 0;
            bool initialised = false;
            bool configured = false;

            static ArduinoDeviceManager *ADM = NULL;

        public:
            enum ErrorCode{
                NO_ERROR = 0,
                NO_ADM_INSTANCE = 1,
                MESSAGE_FRAME_ERROR = 10,
                ADM_MESSAGE_ERROR = 11,
                ADM_MESSAGE_IS_EMPTY= 12,
                NO_DEVICE_ID = 20,
                DEVICE_LIMIT_REACHED = 21,
                DEVICE_ID_ALREADY_USED = 22,
                DEVICE_NOT_FOUND = 23,
            };
            ErrorCode error = NO_ERROR;
    
            static const byte ADM_MESSAGE_SIZE = 50;
            static const byte ADM_TARGET_ID = 0;

            static int inDevicesTable(char *dname);
            static ArduinoDeviceManager *create(char *id, StreamWithCTS *stream);
            static ArduinoDeviceManager *getInstance();
            static void handleStreamReset(StreamWithCTS *stream);
            static void handleStreamReceive(StreamWithCTS *stream, int bytesToRead);
            static void handleStreamSend(StreamWithCTS *stream);
            static void addErrorInfo(ADMMessage *message, ErrorCode errorCode, byte subCode = 0, ADMMessage *originalMessage = NULL);
            

            ArduinoDeviceManager(char *id, StreamWithCTS *stream);
            ~ArduinoDeviceManager();
            bool setup();

            void initialise(ADMMessage *message);
            void configure(ADMMessage *message);
            ArduinoDevice *addDevice(byte id, byte category, char *dname);
            ArduinoDevice *addDevice(ADMMessage *message);
            ArduinoDevice* getDevice(byte deviceID);
            void loop();
            void receiveMessage(ADMMessage* message, ADMMessage* response = NULL);
            void sendMessage(ADMMessage* message);
    }; //end class
} //end namespace

#endif