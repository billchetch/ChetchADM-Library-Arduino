#ifndef CHETCH_ADM_ADM_H
#define CHETCH_ADM_ADM_H

#include <Arduino.h>
#include "ChetchADMMessage.h"
#include "ChetchArduinoDevice.h"

#define MAX_DEVICES 8

namespace Chetch{
    class ArduinoDeviceManager{
        private:
            ArduinoDevice *devices[MAX_DEVICES];
            byte deviceCount = 0;
            byte currentDevice = 0;
            bool initialised = false;

        public:
            enum ErrorCode{
                NO_ERROR = 1,
                NO_DEVICE_ID = 2,
                DEVICE_LIMIT_REACHED = 3,
                DEVICE_ID_ALREADY_USED = 4,
            };
    
            byte error = 0;

            ~ArduinoDeviceManager();
            void initialise(ADMMessage *message);
            ArduinoDevice *addDevice(byte id, byte category, char *dname);
            ArduinoDevice *addDevice(ADMMessage *message);
            ArduinoDevice* getDevice(byte deviceID);
            void loop();
            int receiveMessage(ADMMessage* message);
            int sendMessage(byte *b);
    }; //end class
} //end namespace

#endif