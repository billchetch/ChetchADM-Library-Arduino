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

			case REPEAT_THRESHOLD_LOWER:
				return 3;

			case REPEAT_THRESHOLD_UPPER:
				return 4;

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
		repeatThresholdLower = message->argumentAsByte(getArgumentIndex(message, MessageField::REPEAT_THRESHOLD_LOWER));
		repeatThresholdUpper = message->argumentAsByte(getArgumentIndex(message, MessageField::REPEAT_THRESHOLD_UPPER));

		//fire things up
		irSender.begin(transmitPin);

		//respond
		response->addInt(transmitPin); 

	}

	ArduinoDevice::DeviceCommand IRTransmitter::executeCommand(ADMMessage* message, ADMMessage* response) {
		DeviceCommand deviceCommand = ArduinoDevice::executeCommand(message, response);

		switch (deviceCommand) {
			case SEND:
				protocol = message->argumentAsUInt(getArgumentIndex(message, MessageField::PROTOCOL));
				address = message->argumentAsUInt(getArgumentIndex(message, MessageField::ADDRESS));
				command = message->argumentAsUInt(getArgumentIndex(message, MessageField::COMMAND)); 
				//repeats = message->argumentAsUInt(getArgumentIndex(message, MessageField::REPEATS));

				switch (protocol) {
				case SAMSUNG: //17
					sendFlag = true;
					break;

				default:
					addErrorInfo(response, ErrorCode::INVALID_COMMAND, 1, message);
					return deviceCommand;
				}
				
				response->addUInt(protocol);
				response->addUInt(address);
				response->addUInt(command);
				break;

		} //end command  switch
			
		return deviceCommand;
	}

	void IRTransmitter::loop() {
		ArduinoDevice::loop();

		if (sendFlag) {
			unsigned int millisSinceLastSend = millis() - lastSend;
			switch (protocol) {
				case SAMSUNG: //17
					if (lastProtocol == protocol && lastAddress == address && lastCommand == command && millisSinceLastSend < repeatThresholdUpper) {
						if (millisSinceLastSend < repeatThresholdLower)return;

						irSender.sendSamsungRepeat();
					}
					else {
						irSender.sendSamsung(address, command, 0);
					}
					break;

				default:
					break;

			} //end protocol switch

			lastProtocol = protocol;
			lastAddress = address;
			lastCommand = command;
			lastSend = millis();

			sendFlag = false;
		}
	}
} //end namespace
