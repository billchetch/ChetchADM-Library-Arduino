#include "ChetchUtils.h"
#include "ChetchADMMessage.h"

namespace Chetch{

    ADMMessage::ErrorCode ADMMessage::error = ADMMessage::NO_ERROR;
    bool ADMMessage::hasError(){
        return ADMMessage::error != ADMMessage::NO_ERROR;
    }

    
    /*
    * Constructor
    */

    ADMMessage::ADMMessage(byte maxBytes){
        this->maxBytes = maxBytes;
        
        bytes = new byte[maxBytes];
        argumentIndices = new byte[(maxBytes - HEADER_SIZE) / 2];
        newID();
    };

    ADMMessage::~ADMMessage() {
        delete[] bytes;
        delete[] argumentIndices;
    }

    void ADMMessage::newID(){
        id = millis();
    }

    void ADMMessage::clear(){
        type = (byte)MessageType::TYPE_NONE;
        tag = 0;
        target = 0;
        sender = 0;

        for(int i = 0; i < byteCount; i++)bytes[i] = 0;
        for(int i = 0; i < argumentCount; i++)argumentIndices[i] = 0;

        byteCount = ADMMessage::HEADER_SIZE;
        argumentCount = 0;
        newID();
    }

    bool ADMMessage::isEmpty(){
        return type == (byte)MessageType::TYPE_NONE && argumentCount == 0;
    }

    void ADMMessage::copy(ADMMessage *message){
        clear();
        byte *mbytes = message->getBytes();
        for(int i = 0; i < message->getByteCount(); i++){
            bytes[i] = mbytes[i];
        }
        argumentCount = message->getArgumentCount();
        byteCount = message->getByteCount();
    }

    bool ADMMessage::deserialize(byte* source, byte bCount){
        clear();
        ADMMessage::error = ADMMessage::NO_ERROR;

        int argByteCountIdx = ADMMessage::HEADER_SIZE;
        if(bCount < argByteCountIdx){
            ADMMessage::error = ADMMessage::ERROR_INSUFFICIENT_BYTES;
            return false;
        }

        //move everything to the bytes array
        byteCount = bCount;
        for(int i = 0; i < byteCount; i++){
            bytes[i] = source[i];
        }

        //Member vars (= HEADER)
        type = bytes[0];
        tag = bytes[1];
        target = bytes[2];
        sender = bytes[3];

        //Arguments
        int idx = argByteCountIdx;
        while(idx < byteCount){
            argumentIndices[argumentCount++] = idx;
            idx += bytes[idx] + 1;
        }
        if(idx != byteCount){
            ADMMessage::error = ADMMessage::ERROR_BADLY_FORMED;
            return false;
        }
        return true;
    }

    byte ADMMessage::getArgumentCount(){
        return argumentCount;
    }

    bool ADMMessage::hasArgument(byte argIdx){
        return argIdx < argumentCount;
    }

    byte *ADMMessage::getArgument(byte argIdx){
        if(hasArgument(argIdx)){
            return &bytes[argumentIndices[argIdx] + 1]; //add 1 cos the first byte designates the argument size
        } else {
            return NULL;
        }       
    }

    byte ADMMessage::getArgumentSize(byte argIdx){
        if(hasArgument(argIdx)){
            return bytes[argumentIndices[argIdx]];
        } else {
            return 0;
        }
    }

    long ADMMessage::argumentAsLong(byte argIdx){
        if(hasArgument(argIdx)){
            return Utils::bytesTo<long>(getArgument(argIdx), getArgumentSize(argIdx), littleEndian);
        } else {
            return 0;
        }
    }

    unsigned long ADMMessage::argumentAsULong(byte argIdx){
        if(hasArgument(argIdx)){
            return Utils::bytesTo<unsigned long>(getArgument(argIdx), getArgumentSize(argIdx), littleEndian);
        } else {
            return 0;
        }
    }

    int ADMMessage::argumentAsInt(byte argIdx){
        return (int)argumentAsLong(argIdx);
    }

    unsigned int ADMMessage::argumentAsUInt(byte argIdx){
        return (unsigned int)argumentAsLong(argIdx);
    }

    char *ADMMessage::argumentAsCharArray(byte argIdx, char *s){
        if(hasArgument(argIdx)){
            byte *arg = getArgument(argIdx);
            int i = 0;
            for(i = 0; i < getArgumentSize(argIdx); i++){
                s[i] = arg[i];
            }
            s[i] = 0;
            return s;
        } else {
            return NULL;
        }
    }

    byte ADMMessage::argumentAsByte(byte argIdx){
        if(hasArgument(argIdx)){
            byte *arg = getArgument(argIdx);
            return arg[0];
        } else {
            return 0;
        }
    }

    bool ADMMessage::argumentAsBool(byte argIdx){
        return argumentAsByte(argIdx) > 0;
    }

    float ADMMessage::argumentAsFloat(byte argIdx){
        if(hasArgument(argIdx)){
            return Utils::bytesTo<float>(getArgument(argIdx), getArgumentSize(argIdx), littleEndian);
        } else {
            return 0;
        }
    }

    double ADMMessage::argumentAsDouble(byte argIdx){
        return (double)argumentAsFloat(argIdx);
    }

    void ADMMessage::addBytes(byte *bytev, byte bytec){
        if(byteCount + bytec + 1 > maxBytes)return;
        if(byteCount < ADMMessage::HEADER_SIZE)byteCount = ADMMessage::HEADER_SIZE;

        argumentIndices[argumentCount++] = byteCount; //record the index
        bytes[byteCount++] = bytec; //add the number of bytes this argument requires
        for(int i = 0; i < bytec; i++){
            bytes[byteCount++] = bytev[i]; //add the bytes
        }
    }

    void ADMMessage::addByte(byte argv){
        addBytes((byte*)&argv, sizeof(argv));
    }

    void ADMMessage::addBool(bool argv){
        addByte((byte)argv);
    }
  
    void ADMMessage::addInt(int argv){
        addBytes((byte*)&argv, sizeof(argv));
    }

    void ADMMessage::addUInt(unsigned int argv){
        addBytes((byte*)&argv, sizeof(argv));
    }

    void ADMMessage::addLong(long argv){
        addBytes((byte*)&argv, sizeof(argv));
    }

    void ADMMessage::addULong(unsigned long argv){
        addBytes((byte*)&argv, sizeof(argv));
    }

    void ADMMessage::addString(const char *argv){
        addBytes((byte*)argv, strlen(argv));
    }
  
    void ADMMessage::addFloat(float argv){
        addBytes((byte*)&argv, sizeof(argv));
    }

    void ADMMessage::addDouble(double argv){
        addFloat((float)argv);
    }
  
    byte* ADMMessage::getBytes(){
        //Add all the message data to the byte array
        bytes[0] = type;
        bytes[1] = tag;
        bytes[2] = target;
        bytes[3] = sender;

        return bytes;
    }

    byte ADMMessage::getByteCount(){
        return byteCount;
    }

    byte ADMMessage::serialize(byte* destination){
        getBytes(); // will add everything to byte array
        
        if(destination != NULL){
            for(int i = 0; i < byteCount; i++){
                destination[i] = bytes[i];
            }
        }

        return byteCount;
    }
} //end of namespace