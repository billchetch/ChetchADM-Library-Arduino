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

			case USE_REPEAT:
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
				bool repeat = message->argumentAsBool(getArgumentIndex(message, MessageField::USE_REPEAT));
				if (repeatFlag && !repeat) { //so this is an end to repeating so we just set flags to false
					sendFlag = false;
					repeatFlag = false;
				}
				else { //otherwise here is a normal send or a send with repeat
					switch (protocol) {
					case SAMSUNG: //17
						sendFlag = true;
						break;

					default:
						addErrorInfo(response, ErrorCode::INVALID_COMMAND, 1, message);
						return deviceCommand;
					}
					repeatFlag = repeat;
				}
				
				response->addUInt(protocol);
				response->addUInt(address);
				response->addUInt(command);
				response->addBool(sendFlag);
				response->addBool(repeatFlag);
				break;

		} //end command  switch
			
		return deviceCommand;
	}

	void IRTransmitter::loop() {
		ArduinoDevice::loop();

		if (sendFlag) {
			if (millis() - lastSend < SEND_INTERVAL_THRESHOLD)return;

			switch (protocol) {
				case SAMSUNG: //17
					irSender.sendSamsung(address, command, 0);
					break;

				default:
					break;

			} //end protocol switch

			sendFlag = false;
			lastSend = millis();
		}
		
		if (repeatFlag) {
			switch (protocol) {
				case SAMSUNG: //17
					if ( millis() - lastSend >= 60) {
						irSender.sendSamsungRepeat();
						lastSend = millis();
					}
					break;
			}
		}
	}
} //end namespace
