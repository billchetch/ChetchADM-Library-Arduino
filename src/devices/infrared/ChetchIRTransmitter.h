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
			REPEAT_THRESHOLD,
			PROTOCOL,
			ADDRESS,
			COMMAND,
			REPEATS,
			REPEAT_COMMAND,
		};

	private:
		IRsend irSender;
		byte transmitPin = 0; 
		unsigned int repeatThreshold = 100; //in millis ... if the same command is sent shorter than this interval then assume it is a repeat
		unsigned int lastProtocol = 0;
		unsigned int lastAddress = 0;
		unsigned int lastCommand = 0;
		unsigned int lastSend = 0;

	public:
		IRTransmitter(byte tgt, byte cat, char *dn);
		~IRTransmitter() override;
		
		int getArgumentIndex(ADMMessage* message, MessageField field);

		bool configure(ADMMessage *message, ADMMessage *response) override;
		DeviceCommand executeCommand(ADMMessage* message, ADMMessage* response) override;
	};
} //end namespace	