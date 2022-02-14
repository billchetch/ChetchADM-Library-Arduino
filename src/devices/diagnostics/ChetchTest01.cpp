#include "ChetchUtils.h"
#include "ChetchTest01.h"

namespace Chetch{
    
    Test01::Test01(byte id, byte cat, char *dn) : ArduinoDevice(id, cat, dn){
        //empty
    }


    bool Test01::configure(ADMMessage* message, ADMMessage* response){
        if(!ArduinoDevice::configure(message, response))return false;
        response->addInt(34);
        return true;
    }

    void Test01::populateMessageToSend(byte messageID, ADMMessage* message){
        ArduinoDevice::populateMessageToSend(messageID, message);

        if(messageID == ArduinoDevice::MESSAGE_ID_REPORT){
            message->addInt(testValue);
            message->addULong(millis());
            if(testValue == 0)incrementTestValue = 1;
            if(testValue == 1000)incrementTestValue = -1;
            testValue = testValue + incrementTestValue;
        }
    }

    void Test01::loop(){
        ArduinoDevice::loop();

        if(millis() - ms >= 1000){
            //Serial.println(count);
            ms = millis();
        }
    }

} //end namespace
