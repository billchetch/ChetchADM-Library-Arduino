#include <ChetchADM.h>

#define LOCAL_UART_BUFFER 64
#define REMOTE_UART_BUFFER 64
#define RECEIVE_BUFFER 2*LOCAL_UART_BUFFER
#define SEND_BUFFER 2*LOCAL_UART_BUFFER

using namespace Chetch;

StreamFlowController stream(LOCAL_UART_BUFFER, REMOTE_UART_BUFFER, RECEIVE_BUFFER, SEND_BUFFER);
ArduinoDeviceManager* ADM;

void setup() {
  Serial.begin(115200);
  
  stream.setCTSTimeout(2000);
  stream.begin(&Serial);
  
  ADM = ArduinoDeviceManager::create(&stream); 
}

void loop() {
  ADM->loop();
}
