#include "ChetchArduinoDevice.h"

namespace Chetch{

  ArduinoDevice::ArduinoDevice(){
  }
  
  ArduinoDevice::ArduinoDevice(byte tgt, byte cat, char *dn){
    target = tgt;
    category = cat;

    if(dn != NULL){
      for(int i = 0; i < strlen(dn); i++){
        name[i] = dn[i];
      }
      name[strlen(dn)] = 0;
    }
  }

  ArduinoDevice::~ArduinoDevice() {
	  //because declared virtual
  }

  void ArduinoDevice::handleStatusRequest(ADMMessage *message, ADMMessage *response) {
	  //A hook
  }

  void ArduinoDevice::configure(bool initial, ADMMessage *message, ADMMessage *response) {
	  //A hook
  }

  bool ArduinoDevice::handleCommand(ADMMessage *message, ADMMessage *response) {
	  //Return true to send response, false to cancel send (unless specified message type for response is DATA)
	  switch (message->commandType()) {
		  case ADMMessage::COMMAND_TYPE_TEST:
			  return true;
	  
		  default:
			  return false;
	  }
  }

  ADMMessage* ArduinoDevice::loop() {
	  return NULL;
  }

} //end namespace
