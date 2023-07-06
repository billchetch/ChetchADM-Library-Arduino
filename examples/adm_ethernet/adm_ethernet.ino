#include <ChetchEthernetManager.h>
#include <ChetchNetworkAPI.h>
#include <ChetchADMConfig.h>
#include <ChetchADM.h>

/*
* NOTE: If you encounter problems using a standalone power supply (i.e. something other than the USB connector to a computer)
* this may well be due to a 'bad' power supply.  Try different power supplies to ensure you get a smooth DC current and a fixed voltage.
*/

#define TRACE true 
#define DEV false //set to false for production values

#define HOSTNAME "crayfish" //change this per board
#define PORT 8091 //change this per board
#if DEV
  #define NETWORK_SERVICE_SERVER_IP "192.168.4.188"
#else
  #define NETWORK_SERVICE_SERVER_IP "192.168.2.188"
#endif

#define NETWORK_SERVICE_SERVER_PORT 8001
#define NETWORK_SERVICE_CONNECT_TIMEOUT 20000 //

//SFC values (these values are for Mega)
#define LOCAL_UART_BUFFER 256
#define REMOTE_UART_BUFFER 256
#define RECEIVE_BUFFER 512
#define SEND_BUFFER 512
#define CTS_TIMEOUT 4000

//change values below to fit your network
//NOTE: the Ethernet ENC 28J60works best with a fixed IP
byte mac[] = {  0x00, 0xAD, 0xBE, 0xEF, 0xFE, 0xEF };
#if DEV
byte ip[] = { 192, 168, 4, 10 };    
byte router[] = { 192, 168, 4, 1};
#else
byte ip[] = { 192, 168, 2, 10 };  
byte router[] = { 192, 168, 2, 1};
#endif

byte subnet[] = { 255, 255, 255, 0 };

EthernetServer server(PORT);
EthernetClient client;

using namespace Chetch;

StreamFlowController stream(LOCAL_UART_BUFFER, REMOTE_UART_BUFFER, RECEIVE_BUFFER, SEND_BUFFER);
ArduinoDeviceManager* ADM;

void setup() {
  Serial.begin(115200);

  pinMode(LED_BUILTIN, OUTPUT);
  
  EthernetManager::trace = TRACE;
  NetworkAPI::trace = TRACE;
  bool serverStarted = false;

  do{
    digitalWrite(LED_BUILTIN, HIGH); //light stays on until server has started
    if(TRACE){
      Serial.println("Configuring ethernet...");
    }
    if(EthernetManager::begin(mac, ip, router, subnet)){
      //ethernet hardware is setup so try and register service
      if(TRACE){
        Serial.println("Ethernet configured... attemting to register as a service...");
      }
      int statusCode = NetworkAPI::registerService(client, NETWORK_SERVICE_SERVER_IP, NETWORK_SERVICE_SERVER_PORT, HOSTNAME, PORT, NETWORK_SERVICE_CONNECT_TIMEOUT);
      if(statusCode == 200){ 
        //so we have registered this as a service
        if(TRACE){
          Serial.println("Successfully registered service");
        }
        if(ADM == NULL){ //create ADM
          stream.setCTSTimeout(CTS_TIMEOUT);
          ADM = ArduinoDeviceManager::create(&stream); 
          if(TRACE){
            Serial.println("Created ADM");
          }          
        }
        if(TRACE){
          Serial.print("Firing up server on port: "); 
          Serial.println(PORT);
        }
        server.begin();
        if(TRACE){
          Serial.println("Server has started ... Now waiting for client connection..."); 
        }
        serverStarted = true;
        digitalWrite(LED_BUILTIN, LOW);
      } else {
        if(TRACE){
          Serial.println("Failed to register service"); 
          digitalWrite(LED_BUILTIN, LOW);
          delay(2000);
        }
      }
    
    } else { //problem with ethernet hardware
      Serial.println("Ethernet Failure!!!");
      digitalWrite(LED_BUILTIN, LOW);
      delay(2000);
    }
  } while(!serverStarted);
}


bool clientConnected = false;
void loop() {
  
  ADM->loop();

  if(!EthernetManager::isLinked()){
    if(TRACE)Serial.println("Ethernet not linked");
    if(clientConnected){
      clientConnected = false;
      if(TRACE)Serial.println("Client still flagged as connected so setting to false and ending stream");
      stream.end();
    }
    delay(100);
    return;
  }

  if(EthernetManager::hardwareError()){
    if(TRACE)Serial.println("Hardware error");
    if(clientConnected){
      clientConnected = false;
      if(TRACE)Serial.println("Client still flagged as connected so setting to false and ending stream");
      stream.end();
    }
    delay(100);
    return;
  }

  if(!clientConnected){
    client = server.available();
    if(client){
      if(TRACE)Serial.println("Client connected");
      stream.begin(&client);
      clientConnected = true;
    } else {
      delay(100);
    }
  } else {
    //client is already connected so 
    clientConnected = client.connected();
    if(!clientConnected){
      if(TRACE)Serial.println("Client disconnected");
      stream.end();
      delay(100);
    }
  }
}
