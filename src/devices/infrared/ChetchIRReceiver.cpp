#include "ChetchUtils.h"
#include "ChetchIRReceiver.h"


namespace Chetch{

	IRReceiver::IRReceiver(byte tgt, byte cat, char *dn) : ArduinoDevice(tgt, cat, dn) {
		//empty
		
	}

	IRReceiver::~IRReceiver(){
		//empty
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
		irReceiver.begin(receivePin);
		recording = false;

		response->addByte(receivePin);

		return true;
	}

	ArduinoDevice::DeviceCommand IRReceiver::executeCommand(ADMMessage* message, ADMMessage* response) {
		DeviceCommand deviceCommand = ArduinoDevice::executeCommand(message, response);


		switch (deviceCommand) {
			case START:
				irReceiver.resume();
				recording = true;
				response->addBool(recording);
				break;

			case STOP:
				irReceiver.resume();
				recording = false;
				response->addBool(recording);
				break;
		}

		return deviceCommand;
	}

	void IRReceiver::populateMessageToSend(byte messageID, ADMMessage* message) {
		ArduinoDevice::populateMessageToSend(messageID, message);

		if (messageID == MESSAGE_ID_IRCODERECEIVED) {
			populateMessage(ADMMessage::MessageType::TYPE_DATA, message);

			message->addUInt(irReceiver.decodedIRData.protocol); //Protocol
			message->addUInt(irReceiver.decodedIRData.address); //Address
			message->addUInt(irReceiver.decodedIRData.command); //Command
			
			irReceiver.resume(); //ready for next result	
		}
	} 

	

	void IRReceiver::loop() {
		
		static unsigned long elapsed = 0;
		if ((millis() - elapsed > 100) && irReceiver.decode()) {
			elapsed = millis();
			
			enqueueMessageToSend(MESSAGE_ID_IRCODERECEIVED);
		}
	}

} //end namespace
