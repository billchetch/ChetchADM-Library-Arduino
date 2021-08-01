#ifndef CHETCH_ADM_ARDUINODEVICE_h
#define CHETCH_ADM_ARDUINODEVICE_h

#include <Arduino.h>
#include "ChetchADMMessage.h"

#define DEVICE_NAME_LENGTH 10
#define MSG_DEVICE_CATEGROY_INDEX 0
#define MSG_DEVICE_NAME_INDEX 1
#define MSG_DEVICE_PARAMS_START_INDEX 2

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

	  byte target = 0;
      byte category = 0; //we keep this a byte (rather than enum type) so as to make it fit easier with messaging
      char name[DEVICE_NAME_LENGTH];

	  ArduinoDevice();
      ArduinoDevice(byte target, byte category, char *dn = NULL);
	  virtual ~ArduinoDevice();

	  virtual void handleStatusRequest(ADMMessage *message, ADMMessage *response);
	  virtual void configure(bool initial, ADMMessage *message, ADMMessage *response);
	  virtual bool handleCommand(ADMMessage *message, ADMMessage *response);
	  virtual ADMMessage* loop();
  };
} //end namespace

#endif