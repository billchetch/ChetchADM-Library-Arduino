#include "ChetchUtils.h"
#include "ChetchLCD.h"

namespace Chetch{
    
    LCD::LCD(byte id, byte cat, char *dn) : ArduinoDevice(id, cat, dn){
        //empty
    }

    LCD::~LCD(){
        if(lcd != NULL){
            lcd->clear();
            //lcd->noDisplay();
            delete lcd;
        }
    }

    int LCD::getArgumentIndex(ADMMessage *message, MessageField field){
        switch(field){
            case TEXT_TO_PRINT:
            case CURSOR_POS_X:
                return 1;

            case CURSOR_POS_Y:
                return 2;

            default:
                return (int)field;
        }
    }

   void LCD::configure(ADMMessage* message, ADMMessage* response){
        ArduinoDevice::configure(message, response);
        
        int argIdx = getArgumentIndex(message, MessageField::DATA_PINS);
        dataPins = (DataPinSequence)message->argumentAsByte(argIdx);
        argIdx = getArgumentIndex(message, MessageField::ENABLE_PIN);
        enablePin = message->argumentAsByte(argIdx);
        argIdx = getArgumentIndex(message, MessageField::REGISTER_SELECT_PIN);
        registerSelectPin = message->argumentAsByte(argIdx);
        argIdx = getArgumentIndex(message, MessageField::DISPLAY_DIMENSIONS);
        dimensions = (DisplayDimensions)message->argumentAsByte(argIdx);

        switch(dataPins){
            case DataPinSequence::Pins_5_2:
                lcd = new LiquidCrystal(registerSelectPin, enablePin, 5, 4, 3, 2);
                response->addByte(99);
                break;

            case DataPinSequence::Pins_2_5:
                lcd = new LiquidCrystal(registerSelectPin, enablePin, 2, 3, 4, 5);
                response->addByte(100);
                break;

            default:
                //TODO: add error
                break;
        }

        //lcd = new LiquidCrystal(registerSelectPin, enablePin, 5, 4, 3, 2);
        if(lcd != NULL){
            switch(dimensions){
                case DisplayDimensions::D16x2:
                    lcd->begin(16, 2);
                    break;
            }        
        } else {
            //TODO: add error
        }

        response->addByte((byte)dataPins);
        response->addByte(enablePin);
        response->addBool(registerSelectPin);
        response->addInt((byte)dimensions);
    }

    void LCD::createMessageToSend(byte messageID, ADMMessage* message){
        ArduinoDevice::createMessageToSend(messageID, message);

        
    }

	void LCD::loop(){
        ArduinoDevice::loop();
       

    }

    ArduinoDevice::DeviceCommand LCD::executeCommand(ADMMessage *message, ADMMessage *response){
        DeviceCommand deviceCommand = ArduinoDevice::executeCommand(message, response);

        char output[64];
        switch(deviceCommand){
            case PRINT:
                message->argumentAsCharArray(getArgumentIndex(message, MessageField::TEXT_TO_PRINT), output);
                lcd->print(output);
                break;

            case CLEAR:
                lcd->clear();
                break;

             case SET_CURSOR:
                lcd->setCursor(
                        message->argumentAsInt(getArgumentIndex(message, MessageField::CURSOR_POS_X)),
                        message->argumentAsInt(getArgumentIndex(message, MessageField::CURSOR_POS_Y))
                    );
                    
        }
                
        return deviceCommand;
    } 
} //end namespace
