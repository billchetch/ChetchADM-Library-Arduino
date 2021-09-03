#include "ChetchUtils.h"
#include "ChetchTest01.h"

namespace Chetch{
    
    Test01::Test01(byte id, byte cat, char *dn) : ArduinoDevice(id, cat, dn){
        //empty
    }


    int Test01::configure(ADMMessage* message, ADMMessage* response){
        int argIdx = ArduinoDevice::configure(message, response);
        return argIdx;
    }

    void Test01::createMessage(ADMMessage::MessageType messageTypeToCreate, ADMMessage* message){
        message->type = messageTypeToCreate;
        message->addInt(120);
    }

	/*void Test01::loop(){
        ArduinoDevice::loop();
        
        
    }*/
} //end namespace
