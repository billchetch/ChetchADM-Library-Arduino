#include "ChetchUtils.h"
#include "ChetchTest01.h"

namespace Chetch{
    
    Test01::Test01(byte id, byte cat, char *dn) : ArduinoDevice(id, cat, dn){
        //empty
    }


    void Test01::configure(ADMMessage* message, ADMMessage* response){
        ArduinoDevice::configure(message, response);
        response->addInt(34);
    }

    void Test01::createMessage(ADMMessage::MessageType messageTypeToCreate, ADMMessage* message){
        ArduinoDevice::createMessage(messageTypeToCreate, message);

        message->addInt(testValue);
        if(testValue == 0)incrementTestValue = 1;
        if(testValue == 100)incrementTestValue = -1;
        testValue = testValue + incrementTestValue;
    }

	/*void Test01::loop(){
        ArduinoDevice::loop();
        
        
    }*/
} //end namespace
