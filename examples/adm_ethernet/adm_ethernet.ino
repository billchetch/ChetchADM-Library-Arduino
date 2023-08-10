

#include <ChetchEthernetManager.h>
#include <ChetchNetworkAPI.h>
#include <ChetchADMConfig.h>
#include <ChetchADM.h>

/*
* NOTE: If you encounter problems using a standalone power supply (i.e. something other than the USB connector to a computer)
* this may well be due to a 'bad' power supply.  Try different power supplies to ensure you get a smooth DC current and a fixed voltage.
* Similarly EMF/EMI issues occur with a lot of ethernet devices and something as simple as a drill (electric motor) nearby can destabilise the connection.
*/

//Change this header include per board
#include "crayfish.h"

#define TRACE true 

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

  if(ETHERNET_INIT_PIN >= 0){
    Ethernet.init(ETHERNET_INIT_PIN);
  }
  do{
    statusPin(HIGH); //light stays on until server has started

    //Peform a hardware reset before trying anything else as this setup may be run as a result of a board or power reset
    if(RESETPIN > 0){
      EthernetManager::resetHardware(RESETPIN);
      //Wait an additional period just to be sure
      statusPin(LOW);
      delay(1000);
      statusPin(HIGH);
    }

    //Now try and begin...
    if(EthernetManager::begin(mac, ip, router, subnet, ETHERNET_BEGIN_TIMEOUT)){
      //ethernet hardware is setup so try and register service
      if(TRACE){
        Serial.println("Ethernet successfully configured... attemting to register this board as a service...");
      }
      statusPin(LOW);
      delay(500);
      statusPin(HIGH);
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
        }
      }
      statusPin(LOW);
    } else { //problem with ethernet begin
      Serial.println("Ethernet Failure!!!");
    }
  } while(!serverStarted);
}


bool clientConnected = false;
bool resetHardware = false;

void loop() {
  static unsigned long lastHardwareReset = millis();
  if(RESETPIN > 0 && !clientConnected && millis() - lastHardwareReset > RESET_IF_NO_CLIENT_TIMEOUT){
    resetHardware = true;
  }


  if(!EthernetManager::isLinked() || EthernetManager::hardwareError() || resetHardware){

    if(TRACE && !EthernetManager::isLinked())Serial.println("Ethernet not linked");
    if(TRACE && EthernetManager::hardwareError())Serial.println("Ethernet hardware error");
    if(TRACE && resetHardware)Serial.println("Reset hardware requested");
    
    resetHardware = false;
    lastHardwareReset = millis();

    statusPin(HIGH);


    //end stream if client connected flag still open and then to false
    if(clientConnected){
      clientConnected = false;
      if(TRACE)Serial.println("Client still flagged as connected so setting to false");
    }
    stream.end();
    
    //perform a hardware reset
    EthernetManager::resetHardware(RESETPIN);

    //now try and begin ... keep trying begin forever
    if(EthernetManager::begin(mac, ip, router, subnet, ETHERNET_BEGIN_TIMEOUT)){
      //ok successful ethernet begin so fire up server
      if(TRACE)Serial.println("Ethernet begun so firing up server...");
      server.begin();
      statusPin(LOW);
    } else {
      if(TRACE)Serial.println("Ethernet failed to begin so requesting a hardware reset...");
      lastHardwareReset = millis() - RESET_IF_NO_CLIENT_TIMEOUT;
      statusPin(LOW);
      delay(1000);
    }
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
    } else {
      /*while(client.available()){
        client.read();
      }*/
    }
  }
}
