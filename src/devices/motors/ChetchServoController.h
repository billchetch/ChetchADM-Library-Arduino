#ifndef CHETCH_ADM_SERVO_CONTROLLER_H
#define CHETCH_ADM_SERVO_CONTROLLER_H

#include <Arduino.h>
#include <ChetchArduinoDevice.h>
#include <ChetchADMMessage.h>
#include <Servo.h>

namespace Chetch{
    
    class ServoController : public ArduinoDevice {
        public:
            enum MessageField{
                PIN = 2,
                POSITION,
                LOWER_BOUND,
                UPPER_BOUND,
                TRIM_FACTOR,
                ROTATIONAL_SPEED,
                INCREMENT,
            };

            static const byte MESSAGE_ID_STOPPED_MOVING = 200;
            
        private: 
            Servo servo; 
            byte pin = 0;
            int position = 90;
            int lowerBound = 0;
            int upperBound = 180; //set to -1 for continuous
            int trimFactor = 0;
            unsigned int rotationalSpeed = 0; //degrees per second (assumes servo rotates at same speed in either direction)

            bool moving = false;
            unsigned long startedMoving = 0;
            unsigned long stopMoving = 0;
            
        public: 
            
            ServoController(byte id, byte cat, char *dn);
            ~ServoController() override;

            int getArgumentIndex(ADMMessage *message, MessageField field);

            void configure(ADMMessage* message, ADMMessage* response) override;
            void createMessageToSend(byte messageID, ADMMessage* message) override;
            void loop() override;
            DeviceCommand executeCommand(ADMMessage *message, ADMMessage *response) override;

            int getPosition();
            void moveTo(int angle);
            void rotateBy(int increment);

            bool isMoving();

    }; //end class
} //end namespae
#endif