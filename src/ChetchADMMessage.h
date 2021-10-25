#ifndef CHETCH_ADM_ADMMESSAGE_H
#define CHETCH_ADM_ADMMESSAGE_H

#include <Arduino.h>

namespace Chetch{
    class ADMMessage{
        public:
            enum MessageType {
                TYPE_NONE = 0, //used as a 'zero' value
                TYPE_CUSTOM = 2,
	            TYPE_INFO = 3,
	            TYPE_WARNING,
	            TYPE_ERROR,
	            TYPE_PING,
	            TYPE_PING_RESPONSE,
	            TYPE_STATUS_REQUEST,
	            TYPE_STATUS_RESPONSE,
	            TYPE_COMMAND,
	            TYPE_ERROR_TEST,
	            TYPE_ECHO,
	            TYPE_ECHO_RESPONSE,
	            TYPE_CONFIGURE,
	            TYPE_CONFIGURE_RESPONSE,
	            TYPE_RESET,
	            TYPE_INITIALISE,
	            TYPE_DATA,
	            TYPE_COMMAND_RESPONSE = 24,
                TYPE_NOTIFICATION = 26,
	            TYPE_INITIALISE_RESPONSE = 28
            };
	  
            enum ErrorCode {
                NO_ERROR = 0,
                ERROR_UNRECOGNISED_MESSAGE_TYPE = 2,
                ERROR_INSUFFICIENT_BYTES = 3,
                ERROR_BADLY_FORMED = 4,
                ERROR_ADM_NOT_INITIALISED = 10,
                ERROR_UNKNOWN = 20,
            };
     

            static const byte HEADER_SIZE = 4; //type, tag, target, sender
            

        private:
            byte maxBytes = 0;
            byte *bytes; //an array of all the bytes the message uses
            byte byteCount = HEADER_SIZE; //header size (includes type, tag, target and sender)
            byte argumentCount = 0;
            byte *argumentIndices;
            bool littleEndian = true;
    
        public:  
            static ErrorCode error;
            static bool hasError();
            
            unsigned long id = 0; 
            byte type = 0; //should take messsage type value
            byte tag = 0; //tagging data sent from computer ... can be re-used to send back to make comms linked
            byte target = 0; //used to select a 'device'
            byte sender = 0; //should take the ID of the ADM that sends the message
            
            //Constructor/Destructor
            ADMMessage(byte maxBytes);
            ~ADMMessage();

            void clear();
            bool isEmpty(); //type is NONE and no arguments
            void copy(ADMMessage *message);

            byte getByteCount();
            byte getArgumentCount();
            byte getArgumentSize(byte argIdx);
            byte *getArgument(byte argIdx);
            bool hasArgument(byte argIdx);
            

            long argumentAsLong(byte argIdx);
            unsigned long argumentAsULong(byte argIdx);
            int argumentAsInt(byte argIdx);
            unsigned int argumentAsUInt(byte argIdx);
            char *argumentAsCharArray(byte argIdx, char *s);
            byte argumentAsByte(byte argIdx);
            bool argumentAsBool(byte argIdx);
            float argumentAsFloat(byte argIdx);
            double argumentAsDouble(byte argIdx);

            void addBytes(byte *bytev, byte bytec);
            void addByte(byte argv);
            void addBool(bool argv);
            void addInt(int argv);
            void addUInt(unsigned int argv);
            void addLong(long argv);
            void addULong(unsigned long argv);
            void addString(const char *argv);
            void addFloat(float argv);
            void addDouble(double argv);

            bool deserialize(byte* source, byte bCount);
            byte serialize(byte *destination = NULL);
            byte *getBytes();
          

        private:
            void newID();
    };
}  //end namepace

#endif