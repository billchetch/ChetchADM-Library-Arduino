#include <Arduino.h>
#include <IRremote.h>
#include <IRremoteInt.h>
#include <ChetchArduinoDevice.h>
#include <ChetchADMMessage.h>

namespace Chetch{
	class IRReceiver : public ArduinoDevice {
		public:
			enum MessageField {
				RECEIVE_PIN = 2,
			};

		private:
			byte receivePin;
			IRrecv *irReceiver = NULL;
			decode_results irReceiverResults;
			bool recording = false;
		
		public:
			IRReceiver(byte tgt, byte cat, char *dn);
			~IRReceiver() override;

			int getArgumentIndex(ADMMessage* message, MessageField field);

			bool configure(ADMMessage* message, ADMMessage* response) override;
			DeviceCommand executeCommand(ADMMessage* message, ADMMessage* response) override;
			void loop() override;
  };
} //end namespace	