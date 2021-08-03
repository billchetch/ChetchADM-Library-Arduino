#include "ChetchUtils.h"
#include "ChetchCounter.h"

namespace Chetch{

	Counter::Counter(byte tgt, byte cat, char *dn) : ArduinoDevice(tgt, cat, dn) {
		//empty
	}

	Counter::~Counter(){
		//empty
	}

	void Counter::configure(bool initial, ADMMessage *message, ADMMessage *response) {
		ArduinoDevice::configure(initial, message, response);

		_countPin = message->argumentAsByte(MSG_DEVICE_PARAMS_START_INDEX);
		_countState = message->argumentAsByte(MSG_DEVICE_PARAMS_START_INDEX + 1);
		_rateInterval = message->argumentAsInt(MSG_DEVICE_PARAMS_START_INDEX + 2); 

		_prevState = digitalRead(_countPin);

		_lastMillis = millis();
		
	}

	bool Counter::handleCommand(ADMMessage *message, ADMMessage *response) {
		unsigned long interval = 0;
		float rate = 0;
		switch (message->commandType()) {
			case ADMMessage::COMMAND_TYPE_READ:
				switch(message->commandIndex()){
					case 0: //Count
						response->addLong(_counter);
						response->addLong(millis());
						break;
					case 1: //Rate
						response->addFloat(_rate);
						break;

					default:
						break;

				}
				return true;

			default:
				return false;
		}
	}

	ADMMessage* Counter::loop() {
		if (category > 0) { // means already configured
			byte state = digitalRead(_countPin);
			if (state != _prevState) {
				if (state == _countState)_counter++;
				_prevState = state;
			}

			long interval = millis() - _lastMillis;
			if(interval >= _rateInterval){
				_rate = (float)_rateInterval * (float)(_counter - _lastCount) / (float)interval;
				_lastMillis = millis();
				_lastCount = _counter;
			}
		}
		return NULL;
	}
} //end namespace
