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
		if (!ArduinoDevice::configure(message, response))return false;

		transmitPin = message->argumentAsByte(getArgumentIndex(message, MessageField::TRANSMIT_PIN));
		irSender.begin(transmitPin);
		response->addInt(transmitPin); 

	}

	ArduinoDevice::DeviceCommand IRTransmitter::executeCommand(ADMMessage* message, ADMMessage* response) {
		DeviceCommand deviceCommand = ArduinoDevice::executeCommand(message, response);

		int protocol;
		unsigned int address;
		unsigned int command;
		unsigned int repeats;
		
		switch (deviceCommand) {
			case SEND:
				protocol = message->argumentAsInt(getArgumentIndex(message, MessageField::PROTOCOL));
				address = message->argumentAsULong(getArgumentIndex(message, MessageField::ADDRESS)); //
				command = message->argumentAsULong(getArgumentIndex(message, MessageField::COMMAND)); //
				repeats = message->argumentAsInt(getArgumentIndex(message, MessageField::REPEATS));
				

				switch (protocol) {
					case SAMSUNG: //7
						//irSender.sendSamsung(address, command, repeats);
						break;
					
					default:
						addErrorInfo(response, ErrorCode::INVALID_COMMAND, 1, message);
						return deviceCommand;

				} //end protocol switch
				response->addInt(protocol);
				response->addInt(address);
				response->addInt(command);
				break;

		} //end command  switch
			
		return deviceCommand;
	}
} //end namespace
