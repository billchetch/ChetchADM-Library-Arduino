#include "ChetchIRTransmitter.h"
#include "ChetchUtils.h"


namespace Chetch{

	IRTransmitter::IRTransmitter(byte tgt, byte cat, char *dn) : ArduinoDevice(tgt, cat, dn) {
		//empty
		
	}

	IRTransmitter::~IRTransmitter(){
		//empty
	}

	int IRTransmitter::getArgumentIndex(ADMMessage* message, MessageField field) {
		switch (field) {
			case TRANSMIT_PIN:
				return 2;

			case REPEAT_THRESHOLD:
				return 3;

			case PROTOCOL:
				return 1;

			case ADDRESS:
				return 2;

			case COMMAND:
				return 3;

			case REPEATS:
				return 4;

			default:
				return (int)field;
		}
	}

	bool IRTransmitter::configure(ADMMessage *message, ADMMessage *response) {
		//configure base
		if (!ArduinoDevice::configure(message, response))return false;

		//configure
		transmitPin = message->argumentAsByte(getArgumentIndex(message, MessageField::TRANSMIT_PIN));
		repeatThreshold = message->argumentAsByte(getArgumentIndex(message, MessageField::REPEAT_THRESHOLD));

		//fire things up
		irSender.begin(transmitPin);

		//respond
		response->addInt(transmitPin); 

	}

	ArduinoDevice::DeviceCommand IRTransmitter::executeCommand(ADMMessage* message, ADMMessage* response) {
		DeviceCommand deviceCommand = ArduinoDevice::executeCommand(message, response);

		unsigned int protocol;
		unsigned int address;
		unsigned int command;
		unsigned int repeats;
		bool usedRepeat = false;
		unsigned int millisSinceLastSend = 0;
		
		switch (deviceCommand) {
			case SEND:
				protocol = message->argumentAsUInt(getArgumentIndex(message, MessageField::PROTOCOL));
				address = message->argumentAsUInt(getArgumentIndex(message, MessageField::ADDRESS));
				command = message->argumentAsUInt(getArgumentIndex(message, MessageField::COMMAND)); 
				repeats = message->argumentAsUInt(getArgumentIndex(message, MessageField::REPEATS));
				millisSinceLastSend = millis() - lastSend;

				switch (protocol) {
					case SAMSUNG: //17
						if (lastProtocol == protocol && lastAddress == address && lastCommand == command && millisSinceLastSend < repeatThreshold) {
							if (millisSinceLastSend < repeatThreshold)delay(repeatThreshold - millisSinceLastSend);
							irSender.sendSamsungRepeat();
							usedRepeat = true;
						}
						else {
							irSender.sendSamsung(address, command, repeats);
						}
						break;
					
					default:
						addErrorInfo(response, ErrorCode::INVALID_COMMAND, 1, message);
						return deviceCommand;

				} //end protocol switch

				lastProtocol = protocol;
				lastAddress = address;
				lastCommand = command;
				lastSend = millis();


				response->addUInt(protocol);
				response->addUInt(address);
				response->addUInt(command);
				response->addBool(usedRepeat);
				response->addUInt(millisSinceLastSend);
				break;

		} //end command  switch
			
		return deviceCommand;
	}
} //end namespace
