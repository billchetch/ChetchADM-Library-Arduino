#include <ChetchADMConfig.h>
#include <ChetchADM.h>

//SFC values (these values are for UNO)
#define LOCAL_UART_BUFFER 64
#define REMOTE_UART_BUFFER 64
#define RECEIVE_BUFFER 128
#define SEND_BUFFER 128
#define CTS_TIMEOUT 4000

using namespace Chetch;

StreamFlowController stream(LOCAL_UART_BUFFER, REMOTE_UART_BUFFER, RECEIVE_BUFFER, SEND_BUFFER);
ArduinoDeviceManager* ADM;

void setup() {
  Serial.begin(115200);
  
  stream.setCTSTimeout(CTS_TIMEOUT);
  stream.begin(&Serial, true);
  
  ADM = ArduinoDeviceManager::create(&stream); 
}

void loop() {
  ADM->loop();
}
