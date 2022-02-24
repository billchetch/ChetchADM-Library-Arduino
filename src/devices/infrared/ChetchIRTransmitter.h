#include <Arduino.h>
#define USE_IRREMOTE_HPP_AS_PLAIN_INCLUDE
#include <IRremote.hpp>
#include <ChetchArduinoDevice.h>
#include <ChetchADMMessage.h>

namespace Chetch{
	class IRTransmitter : public ArduinoDevice {
	public:
		enum MessageField {
			TRANSMIT_PIN,
			PROTOCOL,
			ADDRESS,
			COMMAND,
			REPEATS,
			REPEAT_COMMAND,
		};

	private:
		IRsend irSender;
		byte transmitPin = 0; 

	public:
		IRTransmitter(byte tgt, byte cat, char *dn);
		~IRTransmitter() override;
		
		int getArgumentIndex(ADMMessage* message, MessageField field);

		bool configure(ADMMessage *message, ADMMessage *response) override;
		DeviceCommand executeCommand(ADMMessage* message, ADMMessage* response) override;
	};
} //end namespace	