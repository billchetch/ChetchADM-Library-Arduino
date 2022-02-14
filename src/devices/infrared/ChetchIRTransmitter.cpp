#include "ChetchIRTransmitter.h"
#include "ChetchUtils.h"

const unsigned int LGHT_REPEAT[] PROGMEM = { 4500,4400,600,1600,600 };

const unsigned int IR_RAW_CODE_LENGTHS[] PROGMEM = { 5 };
const unsigned int *const IR_RAW_CODES[] PROGMEM = {
	LGHT_REPEAT
};


namespace Chetch{

	IRTransmitter::IRTransmitter(byte tgt, byte cat, char *dn) : ArduinoDevice(tgt, cat, dn) {
		//empty
		
	}

	IRTransmitter::~IRTransmitter(){
		if(irSender != NULL)delete irSender;
		if(repeatCommand != NULL)delete[] repeatCommand;
	}

	int IRTransmitter::getArgumentIndex(ADMMessage* message, MessageField field) {
		switch (field) {
		case IR_COMMAND:
			return 1;

		case BITS:
			return 2;

		case PROTOCOL:
			return 3;

		default:
			return (int)field;
		}
	}

	bool IRTransmitter::configure(ADMMessage *message, ADMMessage *response) {
		if (!ArduinoDevice::configure(message, response))return false;

		activatePin = message->argumentAsByte(getArgumentIndex(message, MessageField::ACTIVATE_PIN));
		if (activatePin > 0) {
			pinMode(activatePin, OUTPUT);
			digitalWrite(activatePin, LOW); 
		}
		transmitPin = message->argumentAsByte(getArgumentIndex(message, MessageField::TRANSMIT_PIN));

		irSender = new IRsend();
		response->addInt(SEND_PIN); //defined by the IRremote library

		//this is the repeat command
		/*int commandIdx = message->argumentAsInt(getArgumentIndex(message, REPEAT_COMMAND));
		if (commandIdx >= 0) {
			unsigned int raw[8];
			switch (commandIdx) { //we have to hard code the indices as the data is taken from progmem
				case 0:
					repeatLength = Utils::getUIntArrayFromProgmem(raw, 0, IR_RAW_CODES, IR_RAW_CODE_LENGTHS); break;
				case 1:
					repeatLength = Utils::getUIntArrayFromProgmem(raw, 1, IR_RAW_CODES, IR_RAW_CODE_LENGTHS); break;
			}
			repeatCommand = new unsigned int[repeatLength];
			for (int i = 0; i < repeatLength; i++) {
				repeatCommand[i] = raw[i];
			}
		}*/
	}

	ArduinoDevice::DeviceCommand IRTransmitter::executeCommand(ADMMessage* message, ADMMessage* response) {
		DeviceCommand deviceCommand = ArduinoDevice::executeCommand(message, response);

		if (irSender == NULL)return deviceCommand;

		unsigned long ircommand;
		int bits;
		int protocol; 

		switch (deviceCommand) {
			case SEND:
				ircommand = message->argumentAsULong(getArgumentIndex(message, MessageField::IR_COMMAND)); //
				bits = message->argumentAsInt(getArgumentIndex(message, MessageField::BITS));
				protocol = message->argumentAsInt(getArgumentIndex(message, MessageField::PROTOCOL));

				switch (protocol) {
					case SAMSUNG: //7
						irSender->sendSAMSUNG(ircommand, bits);
						break;
					case LG: //10
						irSender->sendLG(ircommand, bits);
						break;
					case NEC: //3
						irSender->sendNEC(ircommand, bits);
						break;

					case UNKNOWN: //we send as raw
						/*switch ((int)ircommand) {
						case 0:
							irSender->sendRaw(repeatCommand, repeatLength, 38); break;
						default:
							break;
						}*/
						break;
				} //end protocol switch
				response->addULong(ircommand);
				break;

			case ACTIVATE:
				digitalWrite(activatePin, HIGH);
				break;

			case DEACTIVATE:
				digitalWrite(activatePin, LOW);
				break;
		} //end command  switch
			
		return deviceCommand;
	}
} //end namespace
