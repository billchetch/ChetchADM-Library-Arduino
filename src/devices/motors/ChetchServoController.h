#ifndef CHETCH_ADM_SERVO_CONTROLLER_H
#define CHETCH_ADM_SERVO_CONTROLLER_H

#include <Arduino.h>
#include <ChetchArduinoDevice.h>
#include <ChetchADMMessage.h>
#include <ChetchServo.h>

namespace Chetch{
    
    class ServoController : public ArduinoDevice {
        public:
            enum MessageField{
                PIN = 2,
                SERVO_MODEL,
                POSITION,
                LOWER_BOUND,
                UPPER_BOUND,
                TRIM_FACTOR,
                RESOLUTION,
                INCREMENT,
            };



            static const byte MESSAGE_ID_STOPPED_MOVING = 200;
            static const int EVENT_STARTED_MOVING = 1;
            static const int EVENT_STOPPED_MOVING = 2;
            
        private: 
            Servo* servo = NULL; 
            byte pin = 0;
            int lowerBound = 0;
            int upperBound = 180; 
            
            bool moving = false; //flag for sending stopped moving event message
            
        public: 
            
            ServoController(byte id, byte cat, char *dn);
            ~ServoController() override;

            int getArgumentIndex(ADMMessage *message, MessageField field);

            void setPin(byte pin);
            void setBounds(int lowerBound, int upperBound);
            void createServo(Servo::ServoModel, int pos, int trimFactor, unsigned int resolution);
            bool configure(ADMMessage* message, ADMMessage* response) override;
            void populateMessageToSend(byte messageID, ADMMessage* message) override;
            void loop() override;
            DeviceCommand executeCommand(ADMMessage *message, ADMMessage *response) override;

            int getPosition();
            void moveTo(int angle);
            void rotateBy(int increment);
    }; //end class
} //end namespae
#endif