#ifndef CHETCH_ADM_COUNTER_H
#define CHETCH_ADM_COUNTER_H

#include <Arduino.h>
#include "../ChetchArduinoDevice.h"
#include "../ChetchADMMessage.h"

namespace Chetch{
  class Counter : public ArduinoDevice {
	private:
		byte _countPin;
		byte _countState;
		byte _prevState;
		unsigned long _counter = 0;
		float _rate = 0.0; //count per __rateInteval
		int _rateInterval = 1000; //by default we measure rate over 1 second interavls
		unsigned long _lastCount = 0;
		unsigned long _lastMillis = 0;
		
    public:
		Counter(byte tgt, byte cat, char *dn);
		~Counter();
		void configure(bool initial, ADMMessage *message, ADMMessage *response);
		bool handleCommand(ADMMessage *message, ADMMessage *response);
		ADMMessage* loop();
  };
} //end namespace	
#endif