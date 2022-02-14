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

    void LCD::setPins(DataPinSequence dataPins, byte enablePin, byte registerSelectPin){
        this->dataPins = dataPins;
        this->enablePin = enablePin;
        this->registerSelectPin = registerSelectPin;

        switch(dataPins){
            case DataPinSequence::Pins_5_2:
                lcd = new LiquidCrystal(registerSelectPin, enablePin, 5, 4, 3, 2);
                break;

            case DataPinSequence::Pins_2_5:
                lcd = new LiquidCrystal(registerSelectPin, enablePin, 2, 3, 4, 5);
                break;

            default:
                lcd = NULL;
                break;
        }
    }

    void LCD::setDimensions(DisplayDimensions dimensions){
        this->dimensions = dimensions;

        if(lcd != NULL){
            switch(dimensions){
                case DisplayDimensions::D16x2:
                    columns = 16;
                    rows = 2;
                    break;
            }       
            
            lcd->begin(columns, rows);        
        }
    }

    bool LCD::configure(ADMMessage* message, ADMMessage* response){
        if(!ArduinoDevice::configure(message, response))return false;
        
        setPins(
            (DataPinSequence)message->argumentAsByte(getArgumentIndex(message, MessageField::DATA_PINS)),
            message->argumentAsByte(getArgumentIndex(message, MessageField::ENABLE_PIN)),
            message->argumentAsByte(getArgumentIndex(message, MessageField::REGISTER_SELECT_PIN))
        );

        if(lcd == NULL)return false;

        setDimensions((DisplayDimensions)message->argumentAsByte(getArgumentIndex(message, MessageField::DISPLAY_DIMENSIONS)));


        response->addByte((byte)dataPins);
        response->addByte(enablePin);
        response->addBool(registerSelectPin);
        response->addInt((byte)dimensions);

        return true;
    }

    void LCD::populateMessageToSend(byte messageID, ADMMessage* message){
        ArduinoDevice::populateMessageToSend(messageID, message);

        
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
                print(output);
                break;

            case CLEAR:
                clear();
                break;

             case SET_CURSOR:
                setCursor(
                        message->argumentAsInt(getArgumentIndex(message, MessageField::CURSOR_POS_X)),
                        message->argumentAsInt(getArgumentIndex(message, MessageField::CURSOR_POS_Y))
                    );

             case RESET:
                reset();
                break;
                    
        }
                
        return deviceCommand;
    } 


    void LCD::reset(){
        if(columns > 0 && rows > 0){
            lcd->begin(columns, rows);
            startedPause = 0;
            pauseDuration = 0;
        }
    }

    void LCD::clear(){
        if(canUpdate()){
            //NOTE: beware of this as the library function introduces a delay of 2000 microseconds
            lcd->clear();
        }
    }

    void LCD::setCursor(int x, int y){
        lcd->setCursor(x, y);
    }

    void LCD::print(char *s){
        if(canUpdate()){
            lcd->print(s);
        }
    }

    void LCD::printLine(char* s, byte line, bool pad){
        if(!canUpdate())return;

        lcd->setCursor(0, line);
        lcd->print(s);
        if(pad){
            for(byte i = strlen(s); i < columns; i++){
                lcd->print(" ");
            }
        }
    }

    void LCD::pauseUpdates(unsigned int duration){
        pauseDuration = duration;
        if(pauseDuration > 0){
            startedPause = millis();
        } else {
            startedPause = 0;
        }
    }

    bool LCD::canUpdate(){
        if(pauseDuration > 0){
            bool update = (millis() - startedPause > pauseDuration);
            if(update)pauseDuration = 0; //reset if can update
            return update;
        } else {
            return true;
        }
    }
} //end namespace
