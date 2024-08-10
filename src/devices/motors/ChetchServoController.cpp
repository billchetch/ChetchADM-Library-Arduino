#include "ChetchUtils.h"
#include "ChetchServoController.h"
#include "ChetchADM.h"

namespace Chetch{
    
    ServoController::ServoController(byte id, byte cat, char *dn) : ArduinoDevice(id, cat, dn){
        //empty
    }

    ServoController::~ServoController(){
        if (servo != NULL) {
            Servo::destroy(servo);
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

    void ServoController::setPin(byte pin){
        this->pin = pin;
    }

    void ServoController::createServo(Servo::ServoModel model, int pos, int trimFactor, unsigned int resolution = 1){
       servo = Servo::create(model);
        if(servo != NULL){
            servo->setTrim(trimFactor);
            servo->setResolution(resolution);
            moveTo(pos);
        }
    }

    void ServoController::setBounds(int lowerBound, int upperBound){
        this->lowerBound = lowerBound;
        this->upperBound = upperBound;
    }

    bool ServoController::configure(ADMMessage* message, ADMMessage* response){
        if(!ArduinoDevice::configure(message, response))return false;
        
        setPin(message->argumentAsByte(getArgumentIndex(message, MessageField::PIN)));
        byte model = message->argumentAsByte(getArgumentIndex(message, MessageField::SERVO_MODEL));
        int pos = message->argumentAsInt(getArgumentIndex(message, MessageField::POSITION));
        setBounds(
            message->argumentAsInt(getArgumentIndex(message, MessageField::LOWER_BOUND)),
            message->argumentAsInt(getArgumentIndex(message, MessageField::UPPER_BOUND))
        );
        int trimFactor = message->argumentAsInt(getArgumentIndex(message, MessageField::TRIM_FACTOR));
        unsigned int resolution = message->argumentAsUInt(getArgumentIndex(message, MessageField::RESOLUTION));
        
        //create the servo
        createServo((Servo::ServoModel)model, pos, trimFactor, resolution);
        if(servo == NULL)return false;

        return true;
    }

    
    void ServoController::populateMessageToSend(byte messageID, ADMMessage* message){
        ArduinoDevice::populateMessageToSend(messageID, message);

        if(messageID == ArduinoDevice::MESSAGE_ID_REPORT){
            message->addInt(getPosition());
        }

        if(messageID == MESSAGE_ID_STOPPED_MOVING){
            populateMessage(ADMMessage::MessageType::TYPE_NOTIFICATION, message);
            message->addInt(getPosition());
        }
    }

	void ServoController::loop(){
        ArduinoDevice::loop();
        
        //it's important to regularly call isMoving to prevent against a rare overflow problem (see Servo header class for more info)
        if(servo != NULL && moving && !servo->isMoving()){
            //Serial.println("Stopped!");
            moving = false;
            raiseEvent(EVENT_STOPPED_MOVING);
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
        return servo == NULL ? -1 : servo->read();
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
        raiseEvent(EVENT_STARTED_MOVING);

        if(!servo->attached()){
            //servo.write(position);
            servo->attach(pin); 
        }
        
        servo->write(pos);
    }

    void ServoController::rotateBy(int increment){
        moveTo(getPosition() + increment);
    }

} //end namespace
