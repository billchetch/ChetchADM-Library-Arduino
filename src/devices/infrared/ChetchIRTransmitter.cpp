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

				if (!repeatFlag) {
					switch (protocol) {
						case SAMSUNG: //17
							sendFlag = true;
							break;

						default:
							addErrorInfo(response, ErrorCode::INVALID_COMMAND, 1, message);
							return deviceCommand;
					}
				}
				repeatFlag = repeat;
				repeatCount = 0;
				
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
		else if (repeatFlag) {
			bool sendRepeat = true;
			switch (protocol) {
				case SAMSUNG: //17
					/*if (repeatCount == 0 && millis() - lastSend >= 55) {
						sendRepeat = true;
					}
					else if (repeatCount > 0 && millis() - lastSend >= 110) {
						sendRepeat = true;
					}*/
					if (sendRepeat) {
						lastSend = millis();
						delay(110);
						irSender.sendSamsungRepeat();
						repeatCount++;
					}
					break;
			}
		}
	}
} //end namespace
