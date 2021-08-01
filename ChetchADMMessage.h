#ifndef CHETCH_ADM_ADMMESSAGE_H
#define CHETCH_ADM_ADMMESSAGE_H

#include <Arduino.h>

namespace Chetch{
    class ADMMessage{
        public:
            enum MessageType {
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
	            TYPE_INITIALISE_RESPONSE = 28
            };
	  
            enum CommandType {
	            COMMAND_TYPE_SEND = 1,
	            COMMAND_TYPE_RESET,
	            COMMAND_TYPE_INITIALISE,
	            COMMAND_TYPE_RECEIVE,
	            COMMAND_TYPE_START,
	            COMMAND_TYPE_STOP,
	            COMMAND_TYPE_OPEN,
	            COMMAND_TYPE_CLOSE,
	            COMMAND_TYPE_RECORD,
	            COMMAND_TYPE_SAVE,
	            COMMAND_TYPE_READ,
	            COMMAND_TYPE_TEST
            };

            enum ErrorCode {
                NO_ERROR = 0,
                ERROR_UNRECOGNISED_MESSAGE_TYPE = 2,
                ERROR_BADLY_FORMED = 3,
                ERROR_UNKNOWN = 4,
                ERROR_ADM_NOT_INITIALISED = 5
            };
     

            static const byte HEADER_SIZE = 5;

        private:
            byte maxBytes = 0;
            byte *bytes; //an array of all the bytes the message uses
            byte byteCount = HEADER_SIZE; //header size (includes type, tag, target, command and sender)
            byte argumentCount = 0;
            byte *argumentIndices;
            bool littleEndian = true;
    
        public:  
            static ErrorCode error;

            unsigned long id = 0; 
            byte type = 0; //should take messsage type value
            byte tag = 0; //tagging data sent from computer ... can be re-used to send back to make comms linked
            byte target = 0; //used to select a 'device'
            byte command = 0; //should take a command Type value
            byte sender = 0; //should take the ID of the ADM that sends the message
      

            static long bytesToLong(byte *bytes, int numberOfBytes, bool littleEndian = true);
            static unsigned long bytesToULong(byte *bytes, int numberOfBytes, bool littleEndian = true);
            static int bytesToInt(byte *bytes, int numberOfBytes, bool littleEndian = true);
      
            ADMMessage(byte maxBytes);
            ~ADMMessage();

            bool deserialize(byte* source, byte bCount);

            byte getByteCount();
            byte getArgumentCount();
            byte getArgumentSize(byte argIdx);
            byte *getArgument(byte argIdx);

            long argumentAsLong(byte argIdx);
            unsigned long argumentAsULong(byte argIdx);
            int argumentAsInt(byte argIdx);
            char *argumentAsCharArray(byte argIdx, char *s);
            byte argumentAsByte(byte argIdx);

            void addBytes(byte *bytev, byte bytec);
            void addByte(byte argv);
            void addBool(bool argv);
            void addInt(int argv);
            void addLong(long argv);
            void addString(const char *argv);
            void addFloat(float argv);

            byte serialize(byte *destination);
            byte *getBytes();
            
            //these extract values from the 'command' byte
            CommandType commandType();
            byte commandIndex();

        private:
            void newID();
    };
}  //end namepace

#endif