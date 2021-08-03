#include <Arduino.h>
#include "../ChetchArduinoDevice.h"
#include "../ChetchADMMessage.h"
#include <OneWire.h>
#include <DallasTemperature.h>

namespace Chetch{
  class DS18B20Array : public ArduinoDevice {
	private:
		OneWire *oneWire = NULL;
		DallasTemperature *tempSensors = NULL;
		int numberOfTempSensors = 0;
		uint8_t **deviceAddresses = NULL;
		int requestInterval = 20000;
		float *temperatures;
		byte currentIndex = 0;
		int millis4conversion = 0;
		bool wait4conversion = false;
		unsigned long lastRequested = 0;

    public:
		DS18B20Array(byte tgt, byte cat, char *dn);
		~DS18B20Array();
		void configure(bool initial, ADMMessage *message, ADMMessage *response);
		bool handleCommand(ADMMessage *message, ADMMessage *response);
		ADMMessage* loop();
  };
} //end namespace	