#include "ChetchUtils.h"
#include "ChetchJSN_SR04T.h"

namespace Chetch{

	JSN_SR04T::JSN_SR04T(byte tgt, byte cat, char *dn) : ArduinoDevice(tgt, cat, dn) {
		//empty
		_sensorIndex = sensorCount;
		sensorCount++;
	}

	JSN_SR04T::~JSN_SR04T(){
		//empty
		sensorCount--;
	}

	byte JSN_SR04T::sensorCount = 0;
	byte JSN_SR04T::currentSensor = 0;
	int JSN_SR04T::sensorReadInterval = 5000;
	long JSN_SR04T::lastSensorRead = -1;


	void JSN_SR04T::configure(bool initial, ADMMessage *message, ADMMessage *response) {
		ArduinoDevice::configure(initial, message, response);

		_transmitPin = message->argumentAsByte(MSG_DEVICE_PARAMS_START_INDEX);
		_receivePin = message->argumentAsByte(MSG_DEVICE_PARAMS_START_INDEX + 1);

		//every time we configure a new device we 'reset' the reading
		lastSensorRead = millis(); 
		currentSensor = 0;

		response->addInt(_sensorIndex);
		response->addInt(sensorCount);
	}

	bool JSN_SR04T::handleCommand(ADMMessage *message, ADMMessage *response) {
		switch (message->commandType()) {
			case ADMMessage::COMMAND_TYPE_READ:
				response->type = (byte)ADMMessage::TYPE_DATA;
				response->addLong(_duration);
				return true;

			default:
				return false;
		}
	}

	void JSN_SR04T::readDuration(){
		for (int i = 0; i < 2; i++) { //2 attempts to read the distance
			digitalWrite(_transmitPin, LOW);

			delayMicroseconds(5);
			// Trigger the sensor by setting the transmitPin high for 10 microseconds:
			digitalWrite(_transmitPin, HIGH);
			delayMicroseconds(10);
			digitalWrite(_transmitPin, LOW);

			// Read the receivePin. pulseIn() returns the duration (length of the pulse) in microseconds:
			_duration = pulseIn(_receivePin, HIGH);
			if (_duration > 0)break;
		}
	}

	ADMMessage* JSN_SR04T::loop(){
		if(lastSensorRead < 0 || sensorCount == 0)return NULL; //

		//so if this instance is the 'currentSensor' to read temperature AND the interval has elapsed then...
		if((_sensorIndex == currentSensor) && (millis() - lastSensorRead >= sensorReadInterval)){
			//read duration and update currentSensor and last Read
			readDuration();
			currentSensor = (currentSensor + 1) % sensorCount;
			lastSensorRead = millis();
		}
		return NULL;
	}
} //end namespace
