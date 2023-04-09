#include <ChetchEthernetManager.h>
#include <ChetchNetworkAPI.h>
#include <ChetchADMConfig.h>
#include <ChetchADM.h>

/*
* NOTE: If you encounter problems using a standalone power supply (i.e. something other than the USB connector to a computer)
* this may well be due to a 'bad' power supply.  Try different power supplies to ensure you get a smooth DC current and a fixed voltage.
*/

#define TRACE true 

#define HOSTNAME "crayfish9"
#define PORT 8091
#define NETWORK_SERVICE_SERVER_IP "192.168.4.188"
#define NETWORK_SERVICE_SERVER_PORT 8001
#define NETWORK_SERVICE_CONNECT_TIMEOUT 10000

//SFC values (these values are for Mega)
#define LOCAL_UART_BUFFER 256
#define REMOTE_UART_BUFFER 256
#define RECEIVE_BUFFER 2*LOCAL_UART_BUFFER
#define SEND_BUFFER 2*LOCAL_UART_BUFFER
#define CTS_TIMEOUT 1000

//change values below to fit your network
//NOTE: the Ethernet ENC 28J60works best with a fixed IP
byte mac[] = {  0x00, 0xAD, 0xBE, 0xEF, 0xFE, 0xEF };
byte ip[] = { 192, 168, 4, 10 };    
byte router[] = { 192, 168, 4, 1};
byte subnet[] = { 255, 255, 255, 0 };

EthernetServer server(PORT);
EthernetClient client;

using namespace Chetch;

StreamFlowController stream(LOCAL_UART_BUFFER, REMOTE_UART_BUFFER, RECEIVE_BUFFER, SEND_BUFFER);
ArduinoDeviceManager* ADM;

void setup() {
  Serial.begin(115200);

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  EthernetManager::trace = TRACE;
  NetworkAPI::trace = TRACE;

  if(EthernetManager::begin(mac, ip, router, subnet)){
    int statusCode = NetworkAPI::registerService(client, NETWORK_SERVICE_SERVER_IP, NETWORK_SERVICE_SERVER_PORT, HOSTNAME, PORT, NETWORK_SERVICE_CONNECT_TIMEOUT);
    if(TRACE){
      Serial.println(statusCode == 200 ? "Successfully registered service" : "Failed to register service");
    }
    stream.setCTSTimeout(CTS_TIMEOUT);
    ADM = ArduinoDeviceManager::create(&stream); 
    if(TRACE){
      Serial.print("Firing up server on port: "); 
      Serial.println(PORT);
    }
    server.begin();
    
  } else {
    Serial.println("Ethernet failure!!!");
    while(1){ delay(1); }
  }
}

void loop() {
  ADM->indicateStatus();
  
  client = server.available();
  if(client){
    if(TRACE)Serial.println("Client connected");
    stream.begin(&client);
    
    while(client.connected()){
      ADM->loop();
    }
    if(TRACE)Serial.println("Client disconnected");
    delay(100);
  }
}
