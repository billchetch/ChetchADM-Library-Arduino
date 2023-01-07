#include "ChetchLoadCell.h"


namespace Chetch{

	LoadCell::LoadCell(byte tgt, byte cat, char *dn) : ArduinoDevice(tgt, cat, dn) {
		//empty
		
	}

	LoadCell::~LoadCell(){
		//empty
	}

	int LoadCell::getArgumentIndex(ADMMessage* message, MessageField field) {
		switch (field) {
			case DOUT_PIN:
				return 2;

			case SCK_PIN:
				return 3;

			case READ_INTERVAL:
				return 4;

			case SAMPLE_SIZE:
				return 5;

			default:
				return (int)field;
		}
	}

	bool LoadCell::configure(ADMMessage *message, ADMMessage *response) {
		//configure base
		if (!ArduinoDevice::configure(message, response))return false;
		
		//configure
		doutPin = message->argumentAsByte(getArgumentIndex(message, MessageField::DOUT_PIN));
		sckPin = message->argumentAsByte(getArgumentIndex(message, MessageField::SCK_PIN));
		readInterval = message->argumentAsULong(getArgumentIndex(message, MessageField::READ_INTERVAL));
		sampleSize = message->argumentAsLong(getArgumentIndex(message, MessageField::SAMPLE_SIZE));
		
		//fire things up
		hx711.begin(doutPin, sckPin);

		if(hx711.wait_ready_timeout(1000)){
			hx711.set_offset(offset);
			hx711.set_scale(scale);
		} else {
			response->addInt(1);
			return false;
		}

		//respond
		response->addInt(doutPin); 
		response->addInt(sckPin);
		response->addULong(readInterval);
		response->addLong(sampleSize);

		return true;
	}

	ArduinoDevice::DeviceCommand LoadCell::executeCommand(ADMMessage* message, ADMMessage* response) {
		DeviceCommand deviceCommand = ArduinoDevice::executeCommand(message, response);

		return deviceCommand;
	}

	void LoadCell::populateMessageToSend(byte messageID, ADMMessage* message){
        ArduinoDevice::populateMessageToSend(messageID, message);

        switch(messageID){
			case ArduinoDevice::MESSAGE_ID_REPORT:
				message->addLong(readValue);
				break;
        }
    }

	void LoadCell::loop() {
		ArduinoDevice::loop();

		if(readInterval > 0 &&  millis() - lastRead >= readInterval && hx711.is_ready()){
            long v = hx711.read();
			sampleCount++;
			sampleSum += v;
			if(sampleCount >= sampleSize){
				readValue = sampleSum / sampleCount;
				sampleCount = 0;
				sampleSum = 0;
			}
			lastRead = millis();
        }

	}
}