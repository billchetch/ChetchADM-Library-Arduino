#include "ChetchUtils.h"
#include "ChetchServoController.h"
#include "ChetchADM.h"

namespace Chetch{
    
    ServoController::ServoController(byte id, byte cat, char *dn) : ArduinoDevice(id, cat, dn){
        //empty
    }

    ServoController::~ServoController(){
        if(servo->attached()){
            servo->detach();
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
        byte model = message->argumentAsByte(getArgumentIndex(message, MessageField::SERVO_MODEL));
        int pos = message->argumentAsInt(getArgumentIndex(message, MessageField::POSITION));
        lowerBound = message->argumentAsInt(getArgumentIndex(message, MessageField::LOWER_BOUND));
        upperBound = message->argumentAsInt(getArgumentIndex(message, MessageField::UPPER_BOUND));
        trimFactor = message->argumentAsInt(getArgumentIndex(message, MessageField::TRIM_FACTOR));

        servo = Servo::create((Servo::ServoModel)model);
        if(servo != NULL){
            servo->setTrim(trimFactor);
            moveTo(pos);
        } else {
            //TODO: add error info
        }
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
        
        //it's important to regularly call isMoving to prevent against a rare overflow problem (see Servo header class for more info)
        if(servo != NULL && moving && !servo->isMoving()){
            //Serial.println("Stopped!");
            moving = false;
            enqueueMessageToSend(MESSAGE_ID_STOPPED_MOVING);
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
        if(servo == NULL)return;

        if(upperBound > lowerBound){
            if(pos < lowerBound){
                pos = lowerBound;
            } else if(pos > upperBound){
                pos = upperBound;
            }
        }
        
        moving = true;
        
        if(!servo->attached()){
            //servo.write(position);
            servo->attach(pin); 
        }
        Serial.print("Attempting to move to: "); Serial.println(pos);
        position = servo->write(pos);
    }

    void ServoController::rotateBy(int increment){
        moveTo(getPosition() + increment);
    }

} //end namespace
