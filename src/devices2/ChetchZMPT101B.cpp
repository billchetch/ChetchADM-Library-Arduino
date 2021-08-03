#include "ChetchUtils.h"
#include "devices2/ChetchZMPT101B.h"

namespace Chetch{
    ZMPT101B::ZMPT101B(byte id, byte cat, char* dname) : ArduinoDevice(id, cat, dname){
        //empthy
    }

} //end namespae