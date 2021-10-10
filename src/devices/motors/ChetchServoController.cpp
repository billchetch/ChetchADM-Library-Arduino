#include "ChetchUtils.h"
#include "ChetchServoController.h"

namespace Chetch{
    
    ServoController::ServoController(byte id, byte cat, char *dn) : ArduinoDevice(id, cat, dn){
        //empty
    }

    ServoController::~ServoController(){
        /*if(servo != NULL){
           releaseServo(servo);
        }*/
        if(servo.attached()){
            servo.detach();
        }
        
    }

    int ServoController::getArgumentIndex(ADMMessage *message, MessageField field){
        switch(field){
            case POSITION:
                return message->type == ADMMessage::MessageType::TYPE_CONFIGURE ? (int)field : 1;

            case INCREMENT:
                return 1;

            default:
                return (int)field;
        }
    }

   void ServoController::configure(ADMMessage* message, ADMMessage* response){
        ArduinoDevice::configure(message, response);
        
        pin = message->argumentAsByte(getArgumentIndex(message, MessageField::PIN));
        position = message->argumentAsInt(getArgumentIndex(message, MessageField::POSITION));
        lowerBound = message->argumentAsInt(getArgumentIndex(message, MessageField::LOWER_BOUND));
        upperBound = message->argumentAsInt(getArgumentIndex(message, MessageField::UPPER_BOUND));
        
        moveTo(position);
        servo.attach(pin); 
        
    }

    void ServoController::createMessageToSend(byte messageID, ADMMessage* message){
        ArduinoDevice::createMessageToSend(messageID, message);

        if(messageID == ArduinoDevice::MESSAGE_ID_REPORT){
            //message->addInt(position);
        }
    }

	void ServoController::loop(){
        ArduinoDevice::loop();
        


    }

    ArduinoDevice::DeviceCommand ServoController::executeCommand(ADMMessage *message, ADMMessage *response){
        DeviceCommand deviceCommand = ArduinoDevice::executeCommand(message, response);

        int pos, inc; //important to keep var declerations out side scope of switch
        switch(deviceCommand){
            case ROTATE:
                inc = message->argumentAsInt(getArgumentIndex(message, MessageField::INCREMENT));
                rotateBy(inc);
                response->addInt(getPosition());
                break;

            case MOVE:
                pos = message->argumentAsInt(getArgumentIndex(message, MessageField::POSITION));
                moveTo(pos);
                response->addInt(getPosition());
                break;
        }
                
        return deviceCommand;
    } 


    int ServoController::getPosition(){
        return position;
    }

    void ServoController::moveTo(int pos){
        if(upperBound > lowerBound){
            if(pos < lowerBound){
                pos = lowerBound;
            } else if(pos > upperBound){
                pos = upperBound;
            }
        }

        servo.write(pos);
        //update position
        position = servo.read();
    }

    void ServoController::rotateBy(int increment){
        moveTo(getPosition() + increment);
    }

} //end namespace
