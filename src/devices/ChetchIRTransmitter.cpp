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

	void IRTransmitter::configure(bool initial, ADMMessage *message, ADMMessage *response) {
		ArduinoDevice::configure(initial, message, response);

		irSender = new IRsend();
		response->addInt(SEND_PIN); //defined by the IRremote library

		//this is the repeat command
		int commandIdx = message->argumentAsInt(2);
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
		}
	}

	bool IRTransmitter::handleCommand(ADMMessage *message, ADMMessage *response) {
		if (irSender == NULL)return false;

		if(message->command == (byte)ADMMessage::COMMAND_TYPE_SEND){
			unsigned long ircommand = message->argumentAsULong(0);
			int bits = message->argumentAsInt(1);
			int protocol = message->argumentAsInt(2);

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
				switch ((int)ircommand) {
				case 0:
					irSender->sendRaw(repeatCommand, repeatLength, 38); break;
				default:
					break;
				}
				break;
			} //end protocol switch
		} //end command type conditional

		return false;
	}
} //end namespace
