#include <ChetchEthernetManager.h>
#include <ChetchNetworkAPI.h>
#include <ChetchADMConfig.h>
#include <ChetchADM.h>

/*
* NOTE: If you encounter problems using a standalone power supply (i.e. something other than the USB connector to a computer)
* this may well be due to a 'bad' power supply.  Try different power supplies to ensure you get a smooth DC current and a fixed voltage.
*/

#define TRACE true 
#define DEVPIN 12 //set to false for production values
#define RESETPIN 49 //pin to control hardware reset on ethernet
#define STATUSPIN 48
#define RESET_IF_NO_CLIENT_TIMEOUT 10000 //how long in millis to wait before trying a reset if client

#define DEVIPCOMPONENT 4
#define PROIPCOMPONENT 2

#define ETHERNET_BEGIN_TIMEOUT 5000 //this is so we get the opportunity to do an led flash or something 
#define HOSTNAME "crayfish" //change this per board
#define PORT 8091 //change this per board

//#define NETWORK_SERVICE_SERVER_IP "192.168.0.188"

#define NETWORK_SERVICE_SERVER_PORT 8001
#define NETWORK_SERVICE_CONNECT_TIMEOUT 20000 //

//SFC values (these values are for Mega)
#define LOCAL_UART_BUFFER 256
#define REMOTE_UART_BUFFER 256
#define RECEIVE_BUFFER 512
#define SEND_BUFFER 512
#define CTS_TIMEOUT 4000

//change values per board and to below to fit your network
//NOTE: the Ethernet ENC 28J60works best with a fixed IP
byte mac[] = {  0x00, 0xAD, 0xBE, 0xEF, 0xFE, 0xEF };
byte ip[] = { 192, 168, 0, 10 };    
byte router[] = { 192, 168, 0, 1};
byte subnet[] = { 255, 255, 255, 0 };
byte networkServiceServerIP[] = {192, 168, 0, 188};

EthernetServer server(PORT);
EthernetClient client;

using namespace Chetch;

StreamFlowController stream(LOCAL_UART_BUFFER, REMOTE_UART_BUFFER, RECEIVE_BUFFER, SEND_BUFFER);
ArduinoDeviceManager* ADM;

bool statusPinState = false;
void statusPin(bool val){
  digitalWrite(STATUSPIN, val);
  statusPinState = val;
}

void setup() {
  Serial.begin(115200);

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(STATUSPIN, OUTPUT);
  
  EthernetManager::trace = TRACE;
  NetworkAPI::trace = TRACE;
  bool serverStarted = false;
  
  pinMode(DEVPIN, INPUT_PULLUP);
  bool dev = digitalRead(DEVPIN) == LOW;
  byte ipComponent = dev ? DEVIPCOMPONENT : PROIPCOMPONENT;
  if(TRACE){
    Serial.print(dev ? "Dev mode" : "Production mode");
    Serial.print(" use ip component ");
    Serial.println(ipComponent);
  }

  ip[2] = ipComponent;
  router[2] = ipComponent;
  networkServiceServerIP[2] = ipComponent;

  do{
    statusPin(HIGH); //light stays on until server has started

    //Peform a hardware reset before trying anything else as this setup may be run as a result of a board or power reset
    EthernetManager::resetHardware(RESETPIN);
    
    //Wait an additional period just to be sure
    delay(1000);

    //Now try and begin...
    if(EthernetManager::begin(mac, ip, router, subnet, ETHERNET_BEGIN_TIMEOUT)){
      //ethernet hardware is setup so try and register service
      if(TRACE){
        Serial.println("Ethernet successfully configured... attemting to register this board as a service...");
      }
      
      int statusCode = NetworkAPI::registerService(client, networkServiceServerIP, NETWORK_SERVICE_SERVER_PORT, HOSTNAME, PORT, NETWORK_SERVICE_CONNECT_TIMEOUT);
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
      } else {
        if(TRACE){
          Serial.println("Failed to register service"); 
          delay(2000);
        }
      }
      statusPin(LOW);
      
    } else { //problem with ethernet hardware
      Serial.println("Ethernet Failure!!!");
      statusPin(LOW);
      delay(2000);
    }
  } while(!serverStarted);

}


bool clientConnected = false;
bool resetHardware = false;

void loop() {
  static unsigned long lastHardwareReset = millis();
  if(!clientConnected && millis() - lastHardwareReset > RESET_IF_NO_CLIENT_TIMEOUT){
    resetHardware = true;
  }


  if(!EthernetManager::isLinked() || EthernetManager::hardwareError() || resetHardware){
    resetHardware = false;
    lastHardwareReset = millis();

    statusPin(HIGH);

    if(TRACE && !EthernetManager::isLinked())Serial.println("Ethernet not linked");
    if(TRACE && EthernetManager::hardwareError())Serial.println("Ethernet hardware error");
    if(TRACE && resetHardware)Serial.println("Reset hardware requested");

    //stop the server
    server.end();

    //end stream if client connected flag still open and then to false
    if(clientConnected){
      clientConnected = false;
      if(TRACE)Serial.println("Client still flagged as connected so setting to false and ending stream");
      stream.end();
    }

    //perform a hardware reset
    EthernetManager::resetHardware(RESETPIN);

    //now try and begin ... keep trying begin forever
    if(EthernetManager::begin(mac, ip, router, subnet, ETHERNET_BEGIN_TIMEOUT)){
      //ok successful ethernet begin so fire up server
      if(TRACE)Serial.println("Ethernet begun so firing up server...");
      server.begin();
    } else {
      if(TRACE)Serial.println("Ethernet failed to begin so requesting a hardware reset...");
      lastHardwareReset = millis() - RESET_IF_NO_CLIENT_TIMEOUT;
    }

    statusPin(LOW);
  }
  
  //will indicate status using built in LED only if the client is connected ... hence no led activity indicates no client connected
  ADM->loop(); 

  if(!clientConnected){
    client = server.available();
    if(client){
      if(TRACE)Serial.println("Client connected");
      statusPin(HIGH);
      stream.begin(&client);
      delay(100);
      statusPin(LOW);
      clientConnected = true;
    } else {
      statusPin(!statusPinState);
      delay(100);
    }
  } else {
    //client is already connected so 
    clientConnected = client.connected();
    if(!clientConnected){
      if(TRACE)Serial.println("Client disconnected");
      statusPin(HIGH);
      stream.end();
      delay(100);
      statusPin(LOW);

      //start counting from here
      lastHardwareReset = millis();
    }
  }
}
