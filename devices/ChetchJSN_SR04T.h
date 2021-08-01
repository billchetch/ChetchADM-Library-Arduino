#include <Arduino.h>
#include "../ChetchArduinoDevice.h"
#include "../ChetchADMMessage.h"

namespace Chetch{
  class JSN_SR04T : public ArduinoDevice {
	private:
		static byte sensorCount;
		static byte currentSensor;
		static int sensorReadInterval;
		static long lastSensorRead;

		byte _sensorIndex = 0;
		byte _transmitPin;
		byte _receivePin;
		long _duration = 0;

    public:
		JSN_SR04T(byte tgt, byte cat, char *dn);
		~JSN_SR04T();
		void configure(bool initial, ADMMessage *message, ADMMessage *response);
		bool handleCommand(ADMMessage *message, ADMMessage *response);
		void readDuration();
		ADMMessage *loop();
  };
} //end namespace	