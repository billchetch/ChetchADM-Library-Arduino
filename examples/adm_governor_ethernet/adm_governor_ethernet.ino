

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
//#include "crayfish.h"
//#include "lobster.h"
//#include "mollusc.h"
#include "plankton.h"

#include "governor.h"

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

bool serverStarted = false;
  
void setup() {
  Serial.begin(115200);

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(STATUSPIN, OUTPUT);
  
  delay(500);

  //ADM creating
  stream.setCTSTimeout(CTS_TIMEOUT);
  ADM = ArduinoDeviceManager::create(&stream); 
  if(TRACE){
    Serial.println("Created ADM");
  }
  setupGovernor(ADM);
  //end ADM creation

  EthernetManager::trace = TRACE;
  NetworkAPI::trace = TRACE;
  bool registeredAsService = false;
  int setupAttempts = 0;
  bool setupComplete = false;
  
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

  //prep lcd for use
  lcd->reset();
  do{
    setupAttempts++;
    if(TRACE){
      Serial.print("Attempting setup ... ");
      Serial.println(setupAttempts);
      lcd->printLine("Attempt setup...");
    }
    statusPin(HIGH); //light stays on until server has started

    //Peform a hardware reset before trying anything else as this setup may be run as a result of a board or power reset
    if(RESETPIN > 0){
      EthernetManager::resetHardware(RESETPIN);
      lcd->printLine("Reset hardware");
      //Wait an additional period just to be sure
      statusPin(LOW);
      delay(1000);
      statusPin(HIGH);
    }

    //Now try and begin...
    serverStarted = false;
    lcd->printLine("Begin Ethernet");
    if(EthernetManager::begin(mac, ip, router, subnet, ETHERNET_BEGIN_TIMEOUT)){
      lcd->printLine("Begun Ethernet");
      //ethernet hardware is setup so try and register service
      if(TRACE){
        Serial.println("Ethernet successfully configured ... firing up server...");
      }
      server.begin();
      serverStarted = true;

      statusPin(LOW);
      delay(500);
      statusPin(HIGH);
      int statusCode = NetworkAPI::registerService(client, networkServiceServerIP, NETWORK_SERVICE_SERVER_PORT, HOSTNAME, PORT, NETWORK_SERVICE_CONNECT_TIMEOUT);
      if(statusCode == 200){ 
        //so we have registered this as a service
        if(TRACE){
          Serial.println("Successfully registered service");
        }
        lcd->printLine("Registered!");
        registeredAsService = true;
      } else {
        if(TRACE){
          Serial.println("Failed to register service"); 
        }
        lcd->printLine("Failed Reg.!");
      }
      statusPin(LOW);
    } else { //problem with ethernet begin
      if(TRACE){
        Serial.println("Ethernet Failure!!!");
      }
      lcd->printLine("Ethernet Fail!");
    }
    setupComplete = serverStarted && registeredAsService;
  } while(!setupComplete && setupAttempts < MAX_SETUP_ATTEMPTS);

  if(TRACE){
    if(!setupComplete){
      Serial.println("Setup failed to complete!");
      lcd->printLine("Failed!");
    } else {
      Serial.println("Setup completed successfully!");
      lcd->printLine("Setup!");
      if(mode == MODE_NONE){ //for testing purposes
        zmpt->pauseSampling(true);
        lcd->printLine("Select a mode...", 1);
      }
    }
  }
}


bool clientConnected = false;
bool resetHardware = false;

void loop() {
  //If there is no client connected for a given period of time then do a hardware reset as this may be what is stopping the client from resetting
  static unsigned long lastHardwareReset = millis();
  if(RESETPIN > 0 && RESET_IF_NO_CLIENT_TIMEOUT > 8 && !clientConnected && millis() - lastHardwareReset > RESET_IF_NO_CLIENT_TIMEOUT){
    resetHardware = true;
  }

  //if there is a genuine hardware failure and it's been RESET_IF_HARDWARE_FAILURE ms since last hardware reset then set the flag (or if the timeout value is 0 then never)
  if(RESETPIN > 0 && RESET_IF_HARDWARE_FAILURE_TIMEOUT > 0 && (!EthernetManager::isLinked() || EthernetManager::hardwareError()) && millis() - lastHardwareReset > RESET_IF_HARDWARE_FAILURE_TIMEOUT){
    resetHardware = true;
  }

  //if the server hasn't started and it's been RESET_IF_NO_SERVER_TIMEOUT ms since last hardware reset then set flag (or if the timeout value is 0 then never try a reset)
  if(RESETPIN > 0 && RESET_IF_NO_SERVER_TIMEOUT > 0 && !serverStarted && millis() - lastHardwareReset > RESET_IF_NO_SERVER_TIMEOUT){
    resetHardware = true;
  }

  //variety of conditions that trigger an reinitialising of the ethernet connection
  
  if(resetHardware){
    lcd->reset();
    lcd->printLine("Reset Ethernet");
    if(TRACE && !EthernetManager::isLinked())Serial.println("Ethernet not linked");
    if(TRACE && EthernetManager::hardwareError())Serial.println("Ethernet hardware error");
    if(TRACE && resetHardware)Serial.println("Reset hardware requested");
    
    statusPin(HIGH);
    
    //end stream if client connected flag still open and then to false
    if(clientConnected){
      clientConnected = false;
      if(TRACE)Serial.println("Client still flagged as connected so setting to false");
    }
    stream.end();
    
    //perform a hardware reset
    EthernetManager::resetHardware(RESETPIN);
    lastHardwareReset = millis();
    resetHardware = false;

    //now try and begin ...
    lcd->printLine("Begin Ethernet");
    if(EthernetManager::begin(mac, ip, router, subnet, ETHERNET_BEGIN_TIMEOUT)){
      //ok successful ethernet begin so fire up server
      lcd->printLine("Begun Ethernet");
      if(TRACE)Serial.println("Ethernet begun so firing up server...");
      server.begin();
      serverStarted = true;
      statusPin(LOW);
    } else {
      statusPin(LOW);
      serverStarted = false;
      delay(1000);
    }
  }
  
  //will indicate status using built in LED only if the client is connected ... hence no led activity indicates no client connected
  ADM->loop(); 
  loopGovernor();
  
  if(!clientConnected){
    client = server.available();
    if(client){
      if(TRACE)Serial.println("Client connected");
      statusPin(HIGH);
      stream.begin(&client);
      delay(100);
      statusPin(LOW);
      clientConnected = true;
      resetHardware = false; //cancel all resets
    } else {
      //statusPin(!statusPinState); //flashes
      //delay(100);
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
    } 
  } //end client connected check
}
