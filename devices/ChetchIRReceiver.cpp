#include "ChetchUtils.h"
#include "ChetchIRReceiver.h"

namespace Chetch{

	IRReceiver::IRReceiver(byte tgt, byte cat, char *dn) : ArduinoDevice(tgt, cat, dn) {
		//empty
		
	}

	IRReceiver::~IRReceiver(){
		if(irReceiver != NULL)delete irReceiver;
	}

	void IRReceiver::configure(bool initial, ADMMessage *message, ADMMessage *response) {
		ArduinoDevice::configure(initial, message, response);

		receivePin = message->argumentAsByte(2);
		irReceiver = new IRrecv(receivePin);
		irReceiver->enableIRIn();
		recording = false;
	}

	bool IRReceiver::handleCommand(ADMMessage *message, ADMMessage *response) {
		switch ((ADMMessage::CommandType)message->command) {
			case ADMMessage::COMMAND_TYPE_START:
				irReceiver->enableIRIn();
				irReceiver->resume();
				recording = true;
				response->type = (byte)ADMMessage::TYPE_INFO;
				response->addByte(1);
				return true;

			case ADMMessage::COMMAND_TYPE_STOP:
				irReceiver->resume();
				recording = false;
				response->type = (byte)ADMMessage::TYPE_INFO;
				response->addByte(0);
				return true;

			default:
				return false;
		}
	}

	ADMMessage* IRReceiver::loop() {
		if (irReceiver == NULL || !recording)return NULL;

		static unsigned long elapsed = 0;
		if ((millis() - elapsed > 100) && irReceiver->decode(&irReceiverResults)) {
			elapsed = millis();
			
			ADMMessage *message = new ADMMessage(4);
			message->type = (byte)ADMMessage::TYPE_DATA;
			message->addLong(irReceiverResults.value); //Code
			message->addInt(irReceiverResults.decode_type); //Protocol
			message->addInt(irReceiverResults.bits); //Bits

			irReceiver->resume(); //ready for next result
			
			return message;
		} else {
			return NULL;
		}
	}

} //end namespace
