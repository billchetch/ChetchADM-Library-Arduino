#include <Arduino.h>
#include <HX711.h>
#include <ChetchArduinoDevice.h>
#include <ChetchADMMessage.h>

namespace Chetch{

class LoadCell : public ArduinoDevice{
	public:
		enum MessageField {
			DOUT_PIN = 2,
			SCK_PIN,
			READ_INTERVAL,
			SAMPLE_SIZE,
		};

	private:
		HX711 hx711;
		byte doutPin;
		byte sckPin;
		long offset = 1;
		float scale = 1.0f;
		long readValue = 0; //the averaged value
		long sampleSum = 0;
		long sampleCount = 0;
		long sampleSize = 1; // how many reads to do before updating the average
		unsigned long readInterval = 500; //in millis so read every half second by default
		unsigned long lastRead = 0;

	public:
		LoadCell(byte tgt, byte cat, char *dn);
		~LoadCell() override;
		
		int getArgumentIndex(ADMMessage* message, MessageField field);

		bool configure(ADMMessage *message, ADMMessage *response) override;
		DeviceCommand executeCommand(ADMMessage* message, ADMMessage* response) override;
		void populateMessageToSend(byte messageID, ADMMessage* message) override;
		void loop() override;
		

};

}