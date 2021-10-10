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

    void Test01::createMessageToSend(byte messageID, ADMMessage* message){
        ArduinoDevice::createMessageToSend(messageID, message);

        if(messageID == ArduinoDevice::MESSAGE_ID_REPORT){
            message->addInt(testValue);
            message->addULong(millis());
            if(testValue == 0)incrementTestValue = 1;
            if(testValue == 1000)incrementTestValue = -1;
            testValue = testValue + incrementTestValue;
        }
    }

	/*void Test01::loop(){
        ArduinoDevice::loop();
        
        
    }*/
} //end namespace
