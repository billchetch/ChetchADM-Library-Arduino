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
			USE_REPEAT,
		};

	private:
		IRsend irSender;
		byte transmitPin = 0; 
		bool sendFlag = false;
		bool repeatFlag = false;
		unsigned int protocol = 0;
		unsigned int address = 0;
		unsigned int command = 0;
		unsigned long lastSend = 0;
		unsigned int repeatCount = 0;

	public:
		IRTransmitter(byte tgt, byte cat, char *dn);
		~IRTransmitter() override;
		
		int getArgumentIndex(ADMMessage* message, MessageField field);

		bool configure(ADMMessage *message, ADMMessage *response) override;
		DeviceCommand executeCommand(ADMMessage* message, ADMMessage* response) override;
		void loop() override;
	};
} //end namespace	