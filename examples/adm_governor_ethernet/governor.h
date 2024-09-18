#include "devices/displays/ChetchLCD.h"
#include "devices/electricity/ChetchZMPT101B.h"
#include "devices/motors/ChetchServoController.h"
#include "devices/ChetchSwitchDevice.h"


//Setup defs
//testing and debugging
#define ALLOW_SERIAL_PRINT false //change to false for production

//modes
#define CMD_SET_MODE 1
#define MODE_NONE 0 //used for start up when in testing mode
#define MODE_TEST 1
#define MODE_MANUAL 2
#define MODE_AUTO 3
#define MODE_CONFIG 4
#define MODE_MONITOR 5 //for just reading the results and other info

#define CMD_RESET_LCD 2


//if in test mode
#define CMD_PAUSE_SAMPLING 10
#define CMD_MODIFY_HZ 11
#define CMD_MODIFY_VOLTAGE 12
#define TEST_VOLTAGE 230.0
#define TEST_HZ 50.31
#define TEST_HZ_INC 0.1
#define TEST_VOLTAGE_INC 0.1

//if in manual mode
#define CMD_ROTATE_SERVO 20

//in in config mode
#define CMD_CHOOSE_CONFIG_OPTION 30
#define CMD_UPDATE_CONFIG_OPTION_VALUE 31
#define MAX_CONFIG_OPTIONS 3 //same as the index below
char *configOptions[] = { "Rotate by", "Target Val.",  "Target Tol."};
int configOptionsIndex = 0;

//if in auto mode
#define WAIT_AFTER_TARGET_ACTION 8 //how many �ycles to wait after an action is taken to reach target

//configure
#define NUMBER_OF_DEVICES 4
#define LCD_ID 1
#define LCD_ENABLE_PIN 6
#define LCD_RS_PIN 7
#define ZMPT_ID 2
#define ZMPT_REPORT_INTERVAL 1000
#define ZMPT_HZ_THRESHOLD_VOLTAGE 110 //half of expected voltage
#define ZMPT_PIN A0
#define ZMPT_HZ_TARGET 51.0
#define ZMPT_HZ_TARGET_TOLERANCE 0.8
#define ZMPT_HZ_LOWERBOUND 44.0
#define ZMPT_HZ_UPPERBOUND 60.0
#define SVC_ID 3
#define SVC_SERVO_PIN 30
#define SERVO_START_POS 90
#define SERVO_RESOLUTION 5
#define SERVO_ROTATE_INC -2 //in multiples of servo resolution (see below)
#define SVC_LOWER_BOUND 10
#define SVC_UPPER_BOUND 135
#define BSP_ID 4
#define BOSPUMP_PIN 8
#define ENGINE_WARMUP_TIME 30*1000 //in millis ... the amount of time to wait before considering the engine warmed up (engineOn = true)

namespace Chetch{
    

//specific devices
LCD* lcd = NULL;
ZMPT101B* zmpt = NULL;
ServoController* svc = NULL;
SwitchDevice* bsp = NULL;
bool admConnected = false;

//control variables
//int mode = MODE_NONE; //this starts in an unselected mode
int mode = MODE_AUTO; //this starts as a governor
bool targetLost = false;
bool engineOn = false;
unsigned long bspOnAt = 0;
int rotateServoBy = SERVO_ROTATE_INC; //this is in steps of resolution
double hzTarget = ZMPT_HZ_TARGET;
double hzTargetTolerance = ZMPT_HZ_TARGET_TOLERANCE;


void setupGovernor(ArduinoDeviceManager* ADM) {

    ADM->initialise(ArduinoDeviceManager::AttachmentMode::OBSERVER_OBSERVED,
        NUMBER_OF_DEVICES,
        CADC::AnalogReference::AREF_INTERNAL);

    ADM->addMessageReceivedListener([](ADMMessage* message, ADMMessage* response) {
        if (message->target != ArduinoDeviceManager::ADM_TARGET_ID)return;

        if (message->type == ADMMessage::MessageType::TYPE_COMMAND) {
            int command = message->argumentAsInt(0);
            int argv = message->argumentAsInt(1);
            char output[20];
            switch (command) {
            case CMD_SET_MODE:
                if (argv == MODE_TEST) {
                    lcd->printLine("Test Mode...");
                    if (!zmpt->isSamplingPaused()) {
                        zmpt->pauseSampling(true);
                    }
                    zmpt->assignResults(220.0, hzTarget);
                }
                else if (argv == MODE_MANUAL) {
                    lcd->printLine("Manual Mode...");
                    if (!zmpt->isSamplingPaused()) {
                        zmpt->pauseSampling(true);
                    }
                    sprintf(output, "SP=%d", svc->getPosition());
                    lcd->printLine(output, 1);
                }
                else if (argv == MODE_CONFIG) {
                    lcd->printLine("Config Mode...");
                    if (!zmpt->isSamplingPaused()) {
                        zmpt->pauseSampling(true);
                    }
                    lcd->printLine("> To select", 1);
                    configOptionsIndex = MAX_CONFIG_OPTIONS - 1; //move to end so that first selection is at the beginning
                } else if (argv == MODE_AUTO) {
                    lcd->printLine("Auto Mode...");
                    if (engineOn && zmpt->isSamplingPaused()) {
                        zmpt->resumeSampling(true);
                    }
                    sprintf(output, "SP=%d", svc->getPosition());
                    lcd->printLine(output, 1);
                } else if (argv == MODE_MONITOR) {
                    lcd->printLine("Monitor Mode...");
                    if (zmpt->isSamplingPaused()) {
                        zmpt->resumeSampling(true);
                    }
                    lcd->printLine("", 1);
                }
                lcd->pauseUpdates(1500);
                mode = argv;
                break;

            case CMD_RESET_LCD:
                lcd->reset();
                break;

            case CMD_PAUSE_SAMPLING:
                if (mode != MODE_TEST)break;
                lcd->printLine(argv > 0 ? "Pause sampling!" : "Resume sampling!");
                lcd->pauseUpdates(2000);
                if (argv > 0) {
                    zmpt->pauseSampling(false);
                }
                else {
                    zmpt->resumeSampling(false);
                }
                break;

            case CMD_MODIFY_HZ:
                if (mode != MODE_TEST)break;
                //lcd->printLine("Modify hz!");
                if (argv == 1) {
                    zmpt->assignResults(zmpt->getVoltage(), zmpt->getHz() + TEST_HZ_INC);
                }
                else if (argv == -1) {
                    zmpt->assignResults(zmpt->getVoltage(), zmpt->getHz() - TEST_HZ_INC);
                }
                else {
                    zmpt->assignResults(zmpt->getVoltage(), TEST_HZ);
                }
                break;

            case CMD_MODIFY_VOLTAGE:
                if (mode != MODE_TEST)break;
                lcd->printLine("Modify voltage!");
                if (argv == 1) {
                    zmpt->assignResults(zmpt->getVoltage() + TEST_VOLTAGE_INC, zmpt->getHz());
                }
                else if (argv == -1) {
                    zmpt->assignResults(zmpt->getVoltage() - TEST_VOLTAGE_INC, zmpt->getHz());
                }
                else {
                    zmpt->assignResults(TEST_VOLTAGE, zmpt->getHz());
                }
                break;

            case CMD_ROTATE_SERVO:
                if (mode != MODE_MANUAL)break;
                if (argv == 1) {
                    svc->rotateBy(rotateServoBy*SERVO_RESOLUTION);
                }
                else if (argv == -1) {
                    svc->rotateBy(-rotateServoBy*SERVO_RESOLUTION);
                }
                else {
                    svc->moveTo(SERVO_START_POS);
                }
                sprintf(output, "SP=%d", svc->getPosition());
                lcd->printLine(output, 1);
                break;

              case CMD_CHOOSE_CONFIG_OPTION:
              case CMD_UPDATE_CONFIG_OPTION_VALUE:
                if (mode != MODE_CONFIG)break;
                bool modifyValue = false;
                if (command == CMD_CHOOSE_CONFIG_OPTION) {
                  configOptionsIndex = (configOptionsIndex + 1) % MAX_CONFIG_OPTIONS;
                } else {
                  modifyValue = true;
                }
                sprintf(output, "> %s", configOptions[configOptionsIndex]);
                lcd->printLine(output);
                switch(configOptionsIndex){
                  case 0: //
                    if(modifyValue)rotateServoBy += 1*argv;
                    sprintf(output, "%d", rotateServoBy*SERVO_RESOLUTION);
                    lcd->printLine(output, 1);
                    break;
  
                  case 1:
                    if(modifyValue)hzTarget += 0.1*(double)argv;
                    if(hzTarget < 0)hzTarget = 0.0;
                    dtostrf(hzTarget, 2, 1, output);
                    lcd->printLine(output, 1);
                    zmpt->setTargetParameters(ZMPT101B::Target::HZ,
                                              hzTarget,
                                              hzTargetTolerance,
                                              ZMPT_HZ_LOWERBOUND,
                                              ZMPT_HZ_UPPERBOUND);
                    break;

                  case 2:
                    if(modifyValue)hzTargetTolerance += 0.1*(double)argv;
                    if(hzTargetTolerance < 0)hzTargetTolerance = 0.0;
                    dtostrf(hzTargetTolerance, 2, 1, output);
                    lcd->printLine(output, 1);
                    zmpt->setTargetParameters(ZMPT101B::Target::HZ,
                                              hzTarget,
                                              hzTargetTolerance,
                                              ZMPT_HZ_LOWERBOUND,
                                              ZMPT_HZ_UPPERBOUND);
                    break;
                }
                break;
            }
        }
        else {
            switch (message->type) {
            case ADMMessage::MessageType::TYPE_FINALISE:
                admConnected = false;
                break;
            default:
                admConnected = true;
                break;
            }
        }
        });

    //Create the devices and configure
    lcd = (LCD*)ADM->addDevice(LCD_ID, ArduinoDevice::Category::LCD, "LCD");
    lcd->setPins(LCD::DataPinSequence::Pins_5_2, LCD_ENABLE_PIN, LCD_RS_PIN);
    lcd->setDimensions(LCD::DisplayDimensions::D16x2);
    //lcd->resetAfter = 60 * 1000;
    lcd->setAsReady(true); //enable at the same time

    zmpt = (ZMPT101B*)ADM->addDevice(ZMPT_ID, ArduinoDevice::Category::VAC_SENSOR, "ZMPT101B");
    zmpt->setVoltagePin(ZMPT_PIN);
    zmpt->setReportInterval(ZMPT_REPORT_INTERVAL);
    zmpt->setTargetParameters(ZMPT101B::Target::HZ,
        ZMPT_HZ_TARGET,
        ZMPT_HZ_TARGET_TOLERANCE,
        ZMPT_HZ_LOWERBOUND,
        ZMPT_HZ_UPPERBOUND);
    zmpt->setHzThresholdVoltage(ZMPT_HZ_THRESHOLD_VOLTAGE);
    zmpt->addEventListener([](ArduinoDevice* device, int eventID) {
        char output[20];
        switch (eventID) {
        case ZMPT101B::EVENT_NEW_RESULTS:
            if (ALLOW_SERIAL_PRINT) {
                Serial.print("Updated zmpt ");
                Serial.print(zmpt->getSummary());
                Serial.println();
            }
            if (mode == MODE_TEST) {
                sprintf(output, "T:%s", zmpt->getSummary());
            } else {
                sprintf(output, "%s", zmpt->getSummary());
            }
            lcd->printLine(output);
            sprintf(output, "SP=%d ES=%s", svc->getPosition(), engineOn ? "On" : (bsp->isOn() ? "BP On" : "Off"));
            lcd->printLine(output, 1);
            break;

        case ZMPT101B::EVENT_TARGET_REACHED:
            if (ALLOW_SERIAL_PRINT) {
                Serial.print("Target reached!");
                Serial.println();
            }
            targetLost = false;
            break;

        case ZMPT101B::EVENT_TARGET_LOST:
            if (mode == MODE_MANUAL)break;

            if (ALLOW_SERIAL_PRINT) {
                Serial.print("Adjustment required: ");
                Serial.println();
            }
            targetLost = true;
            break;

        case ZMPT101B::EVENT_OUT_OF_TARGET_RANGE:
            if (mode == MODE_MANUAL || !engineOn)break;
            targetLost = false;
            lcd->printLine("Out of range!", 1);
            lcd->pauseUpdates(2000);
            break;

        }
        return true;
        });
    zmpt->setAsReady(true); //enable at the same time

    svc = (ServoController*)ADM->addDevice(SVC_ID, ArduinoDevice::Category::SERVO, "SERVO");
    svc->setPin(SVC_SERVO_PIN);
    svc->setBounds(SVC_LOWER_BOUND, SVC_UPPER_BOUND);
    svc->createServo(Servo::ServoModel::MG996, SERVO_START_POS, 0, SERVO_RESOLUTION);
    /*svc->addEventListener([](ArduinoDevice* device, int eventID){
                            char output[20];
                            switch(eventID){
                              case ServoController::EVENT_STARTED_MOVING:
                                break;
                              case ServoController::EVENT_STOPPED_MOVING:
                                break;
                            }
                            return true;
                          });*/
    svc->setAsReady(true);
    
    bsp = (SwitchDevice*)ADM->addDevice(BSP_ID, ArduinoDevice::Category::SWITCH, "SWITCH");
    bsp->configure(SwitchDevice::SwitchMode::PASSIVE, BOSPUMP_PIN, 200);
    bsp->addEventListener([](ArduinoDevice* device, int eventID) {
        char output[20];
        switch (eventID) {
        case SwitchDevice::EVENT_SWITCH_TRIGGERED:
            if (bsp->isOn()) {
                lcd->printLine("Bosspump On!");
                sprintf(output, "SP=%d", svc->getPosition());
                lcd->printLine(output, 1);
                bspOnAt = millis();
                engineOn = false;
            }
            else {
                if(engineOn){
                  svc->moveTo(SERVO_START_POS); //return to start position
                  lcd->printLine("Engine Off");
                  sprintf(output, "SP=%d", svc->getPosition());
                  lcd->printLine(output, 1);
                  zmpt->pauseSampling(true);
                }
                engineOn = false;
                targetLost = false;
            }
            break;
        }
        });
    bsp->setAsReady(true);

    //Set ADM as ready, output a  message and wait a mo
    ADM->setAsReady();

    lcd->printLine("ADM Ready!");
    if(ALLOW_SERIAL_PRINT){
      Serial.println("Governor setup complete!");
    }
    delay(1000);
    lcd->reset();
}

void loopGovernor() {
    //monitoring engine on/off
    if (bsp->isOn() && !engineOn && ((millis() - bspOnAt) > ENGINE_WARMUP_TIME)) {
        engineOn = true;
        if(mode == MODE_AUTO && zmpt->isSamplingPaused()){
          zmpt->resumeSampling(true);
        }
        lcd->printLine("Engine On!");
    }

    
    //monitoring electricity
    static unsigned long ms2 = millis();
    static unsigned int waitFor = 0; //wait a number of cycles
    static double lastValue = 0.0;
    if (((mode == MODE_AUTO && engineOn) || mode == MODE_TEST) && targetLost && ((millis() - ms2) > 250)) {
        ms2 = millis();

        if (waitFor > 0) {
            if (waitFor < 5) {
                lcd->printLine("", 1);
            }
            waitFor--;
        }
        else {
            //if targeting value goes out of bounds then adjust according to desired direction
            char output[32];
            double currentValue = zmpt->getCurrentValue();
            ZMPT101B::Direction resultDir = lastValue == 0.0 ? zmpt->getCurrentDirection() : zmpt->getDirection(currentValue, lastValue);
            ZMPT101B::Direction desiredDir = zmpt->getDesiredDirection();
            lastValue = currentValue;

            if (resultDir != desiredDir) { //Take some action
                switch (desiredDir) {
                case ZMPT101B::Direction::Falling:
                    svc->rotateBy(-rotateServoBy*SERVO_RESOLUTION);
                    if (svc->reachedLowerBound()) {
                        lcd->printLine("<<<< SP=LB!", 1);
                    }
                    else {
                        sprintf(output, "<<<< SP <- %d", svc->getPosition());
                        lcd->printLine(output, 1);
                    }
                    lcd->pauseUpdates(1000);
                    waitFor = WAIT_AFTER_TARGET_ACTION;
                    break;

                case ZMPT101B::Direction::Rising:
                    svc->rotateBy(rotateServoBy*SERVO_RESOLUTION);
                    if (svc->reachedUpperBound()) {
                        lcd->printLine(">>>> SP=UB!", 1);
                    }
                    else {
                        sprintf(output, ">>>> SP -> %d", svc->getPosition());
                        lcd->printLine(output, 1);
                    }
                    lcd->pauseUpdates(1000);
                    waitFor = WAIT_AFTER_TARGET_ACTION;
                    break;

                default:
                    lcd->printLine("Weird1", 1);
                    break;
                }
            }
            else { //going in the right direction
                switch (resultDir) {
                case ZMPT101B::Direction::Falling:
                    lcd->printLine("<<<< Waiting...", 1);
                    lcd->pauseUpdates(1000);
                    waitFor = WAIT_AFTER_TARGET_ACTION;
                    break;

                case ZMPT101B::Direction::Rising:
                    lcd->printLine(">>>> Waiting...", 1);
                    lcd->pauseUpdates(1000);
                    waitFor = WAIT_AFTER_TARGET_ACTION;
                    break;

                default:
                    lcd->printLine("Weird2", 1);
                    break;
                }
            }
        } //end wait conditional
    } //end montior loop

    //ensuring results are assigend during test mode
    static unsigned long ms3 = millis();
    if (zmpt->isSamplingPaused() && mode == MODE_TEST && ((millis() - ms3) > 1000)) {
        ms3 = millis();
        zmpt->assignResults(zmpt->getVoltage(), zmpt->getHz());
    } 
}

} //end namespa