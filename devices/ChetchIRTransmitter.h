#include <Arduino.h>
#include <IRremote.h>
#include <IRremoteInt.h>
#include "../ChetchArduinoDevice.h"
#include "../ChetchADMMessage.h"


namespace Chetch{
  class IRTransmitter : public ArduinoDevice {
	private:
		IRsend *irSender = NULL;
		unsigned int repeatLength = 0;
		unsigned int *repeatCommand = NULL;

    public:
		IRTransmitter(byte tgt, byte cat, char *dn);
		~IRTransmitter();
		void configure(bool initial, ADMMessage *message, ADMMessage *response);
		bool handleCommand(ADMMessage *message, ADMMessage *response);
  };
} //end namespace	