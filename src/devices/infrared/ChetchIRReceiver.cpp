#include "ChetchUtils.h"
#include "ChetchIRReceiver.h"

namespace Chetch{

	IRReceiver::IRReceiver(byte tgt, byte cat, char *dn) : ArduinoDevice(tgt, cat, dn) {
		//empty
		
	}

	IRReceiver::~IRReceiver(){
		if(irReceiver != NULL)delete irReceiver;
	}

	int IRReceiver::getArgumentIndex(ADMMessage* message, MessageField field) {
		switch (field) {
			

			default:
				return (int)field;
		}
	}

	bool IRReceiver::configure(ADMMessage *message, ADMMessage *response) {
		if (!ArduinoDevice::configure(message, response))return false;

		receivePin = message->argumentAsByte(getArgumentIndex(message, MessageField::RECEIVE_PIN));
		irReceiver = new IRrecv(receivePin);
		irReceiver->enableIRIn();
		recording = false;

		response->addByte(receivePin);

		return true;
	}

	ArduinoDevice::DeviceCommand IRReceiver::executeCommand(ADMMessage* message, ADMMessage* response) {
		DeviceCommand deviceCommand = ArduinoDevice::executeCommand(message, response);


		switch (deviceCommand) {
			case START:
				irReceiver->enableIRIn();
				irReceiver->resume();
				recording = true;
				response->addBool(recording);
				break;

			case STOP:
				irReceiver->resume();
				recording = false;
				response->addBool(recording);
				break;

			case RESUME:
				irReceiver->enableIRIn();
				irReceiver->resume();
				break;
		}

		return deviceCommand;
	}

	void IRReceiver::populateMessageToSend(byte messageID, ADMMessage* message) {
		ArduinoDevice::populateMessageToSend(messageID, message);

		if (messageID == MESSAGE_ID_IRCODERECEIVED) {
			populateMessage(ADMMessage::MessageType::TYPE_DATA, message);
			message->addLong(irReceiverResults.value); //Code
			message->addInt(irReceiverResults.decode_type); //Protocol
			message->addInt(irReceiverResults.bits); //Bits
			irReceiver->resume(); //ready for next result	
		}
	} 

	/*bool IRReceiver::handleCommand(ADMMessage* message, ADMMessage* response) {
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
	}*/

	void IRReceiver::loop() {
		if (irReceiver == NULL)return;

		static unsigned long elapsed = 0;
		if ((millis() - elapsed > 100) && irReceiver->decode(&irReceiverResults)) {
			elapsed = millis();
			
			enqueueMessageToSend(MESSAGE_ID_IRCODERECEIVED);
		}
	}

} //end namespace
