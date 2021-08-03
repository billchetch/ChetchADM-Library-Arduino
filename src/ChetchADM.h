#ifndef CHETCH_ADM_ADM_H
#define CHETCH_ADM_ADM_H

#include <Arduino.h>
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
            ArduinoDevice *devices[MAX_DEVICES];
            byte deviceCount = 0;
            byte currentDevice = 0;
            bool initialised = false;
            bool configured = false;

        public:
            enum ErrorCode{
                NO_ERROR = 1,
                NO_DEVICE_ID = 2,
                DEVICE_LIMIT_REACHED = 3,
                DEVICE_ID_ALREADY_USED = 4,
            };
    
            static int inDevicesTable(char *dname);

            byte error = 0;

            ~ArduinoDeviceManager();
            void initialise(ADMMessage *message);
            void configure(ADMMessage *message);
            ArduinoDevice *addDevice(byte id, byte category, char *dname);
            ArduinoDevice *addDevice(ADMMessage *message);
            ArduinoDevice* getDevice(byte deviceID);
            void loop();
            int receiveMessage(ADMMessage* message);
            int sendMessage(byte *b);
    }; //end class
} //end namespace

#endif