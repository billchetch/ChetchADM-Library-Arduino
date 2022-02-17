#include <Arduino.h>
#include <IRremote.h>
#include <IRremoteInt.h>
#include <ChetchArduinoDevice.h>
#include <ChetchADMMessage.h>

namespace Chetch{
	class IRTransmitter : public ArduinoDevice {
	public:
		enum MessageField {
			ACTIVATE_PIN = 2,
			TRANSMIT_PIN,
			REPEAT_COMMAND,
			IR_COMMAND,
			BITS,
			PROTOCOL,
			RAW_LENGTH,
			RAW,
		};

	private:
		IRsend *irSender = NULL;
		unsigned int repeatLength = 0;
		unsigned int *repeatCommand = NULL;
		byte activatePin = 0;
		byte transmitPin = 0; //Redundant for IRremote.h libraries < 3.0

	public:
		IRTransmitter(byte tgt, byte cat, char *dn);
		~IRTransmitter() override;
		
		int getArgumentIndex(ADMMessage* message, MessageField field);

		bool configure(ADMMessage *message, ADMMessage *response) override;
		DeviceCommand executeCommand(ADMMessage* message, ADMMessage* response) override;
	};
} //end namespace	