#include <Arduino.h>
#define USE_IRREMOTE_HPP_AS_PLAIN_INCLUDE
#include <IRremote.hpp>
#include <ChetchArduinoDevice.h>
#include <ChetchADMMessage.h>

namespace Chetch{
	class IRReceiver : public ArduinoDevice {
		public:
			enum MessageField {
				RECEIVE_PIN = 2,
			};

			static const byte MESSAGE_ID_IRCODERECEIVED = 200;

		private:
			byte receivePin;
			IRrecv irReceiver;
			bool recording = false;
		
		public:
			IRReceiver(byte tgt, byte cat, char *dn);
			~IRReceiver() override;

			int getArgumentIndex(ADMMessage* message, MessageField field);

			bool configure(ADMMessage* message, ADMMessage* response) override;
			DeviceCommand executeCommand(ADMMessage* message, ADMMessage* response) override;
			void populateMessageToSend(byte messageID, ADMMessage* message) override;
			void loop() override;
  };
} //end namespace	