#include "ChetchUtils.h"
#include "ChetchServoController.h"
#include "ChetchADM.h"

namespace Chetch{
    
    ServoController::ServoController(byte id, byte cat, char *dn) : ArduinoDevice(id, cat, dn){
        //empty
    }

    ServoController::~ServoController(){
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
        rotationalSpeed = message->argumentAsUInt(getArgumentIndex(message, MessageField::ROTATIONAL_SPEED)); //in degrees per second

        servo.write(position);
        moveTo(position);
    }

    void ServoController::createMessageToSend(byte messageID, ADMMessage* message){
        ArduinoDevice::createMessageToSend(messageID, message);

        if(messageID == ArduinoDevice::MESSAGE_ID_REPORT){
            //message->addInt(position);
        }

        if(messageID == MESSAGE_ID_STOPPED_MOVING){
            //TODO: notification message goes here
        }
    }

	void ServoController::loop(){
        ArduinoDevice::loop();
        
        if(moving && millis() >= stopMoving){
            Serial.println("Stopped!");
            moving = false;
            enqueueMessageToSend(MESSAGE_ID_STOPPED_MOVING);
            if(ADM->isUsingTimer()){
                servo.detach();
                ADM->resumeTimer();
            }
        }

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
        if(moving)return;

        if(upperBound > lowerBound){
            if(pos < lowerBound){
                pos = lowerBound;
            } else if(pos > upperBound){
                pos = upperBound;
            }
        }

        moving = true;
        startedMoving = millis();
        int diff = abs(getPosition() - pos);
        //calculate stopped moving time ... add 100ms as grace period
        stopMoving = 100 + startedMoving + (int)(1000.0 * (double)(diff) / (double)rotationalSpeed);
        if(ADM->isUsingTimer()){
            ADM->pauseTimer();
        }
        
        if(!servo.attached()){
            servo.write(position);
            servo.attach(pin); 
        }
        Serial.print("Moving to: "); Serial.println(pos);
        servo.write(pos);
        
        //update position
        position = pos; 
    }

    void ServoController::rotateBy(int increment){
        moveTo(getPosition() + increment);
    }

} //end namespace
