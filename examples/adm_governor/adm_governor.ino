//general includes
#include <ChetchADMConfig.h>
#include <ChetchADM.h>

//device specific includes
#include "devices/displays/ChetchLCD.h"
#include "devices/electricity/ChetchZMPT101B.h"
#include "devices/motors/ChetchServoController.h"
#include "devices/ChetchSwitchDevice.h"

//SFC values (these values are for UNO)
#define LOCAL_UART_BUFFER 64
#define REMOTE_UART_BUFFER 64
#define RECEIVE_BUFFER 128
#define SEND_BUFFER 128
#define CTS_TIMEOUT 4000

//Setup defs
//testing and debugging
#define ALLOW_SERIAL_PRINT false //change to false for production

//modes
#define CMD_SET_MODE 1
#define MODE_TEST 1
#define MODE_MANUAL 2
#define MODE_AUTO 3

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
#define SERVO_ROTATE_INC 5 //in degrees

//if in auto mode
#define WAIT_AFTER_TARGET_ACTION 8 //how many çycles to wait after an action is taken to reach target

//configure
#define NUMBER_OF_DEVICES 4
#define LCD_ID 1
#define LCD_ENABLE_PIN 11
#define LCD_RS_PIN 12
#define ZMPT_ID 2
#define ZMPT_REPORT_INTERVAL 1000
#define ZMPT_PIN A0
#define ZMPT_HZ_TARGET 50
#define ZMPT_HZ_TARGET_TOLERANCE 0.5
#define ZMPT_HZ_LOWERBOUND 46.0
#define ZMPT_HZ_UPPERBOUND 54.0
#define SVC_ID 3
#define SVC_SERVO_PIN 8
#define SERVO_START_POS 90
#define SERVO_RESOLUTION 5
#define BSP_ID 4
#define BOSPUMP_PIN 9

//

using namespace Chetch;

//ADM comms and object 
StreamFlowController stream(LOCAL_UART_BUFFER, REMOTE_UART_BUFFER, RECEIVE_BUFFER, SEND_BUFFER);
ArduinoDeviceManager* ADM;

//specific devices
LCD* lcd = NULL;
ZMPT101B* zmpt = NULL;
ServoController* svc = NULL;
SwitchDevice* bsp = NULL;
bool clientConnected = false;

//control variables
int mode = MODE_AUTO; //this starts as a governor
bool targetLost = false;
bool engineOn = false;
unsigned long bspOnAt = 0;

void setup() {
  Serial.begin(115200);
  
  stream.setCTSTimeout(CTS_TIMEOUT);
  stream.begin(&Serial, true);

  //Create ADM and initialise
  ADM = ArduinoDeviceManager::create(&stream); 
  ADM->initialise(ArduinoDeviceManager::AttachmentMode::OBSERVER_OBSERVED, 
                  NUMBER_OF_DEVICES, 
                  CADC::AnalogReference::AREF_INTERNAL);

  ADM->addMessageReceivedListener([](ADMMessage* message, ADMMessage* response){
                                    if(message->target != ArduinoDeviceManager::ADM_TARGET_ID)return;
                                    
                                    if(message->type == ADMMessage::MessageType::TYPE_COMMAND){
                                    
                                      int command = message->argumentAsInt(0);
                                      int argv = message->argumentAsInt(1);
                                      switch(command){
                                        case CMD_SET_MODE:
                                          if(argv == MODE_TEST){
                                            lcd->printLine("Test Mode...");
                                          } else if(argv == MODE_MANUAL){
                                            lcd->printLine("Manual Mode...");
                                          } else {
                                            lcd->printLine("Auto Mode...");
                                          }
                                          lcd->pauseUpdates(2000);
                                          mode = argv;
                                          break;

                                        case CMD_PAUSE_SAMPLING:
                                          if(mode != MODE_TEST)break;
                                          lcd->printLine(argv > 0 ? "Pause sampling!" : "Resume sampling!");
                                          lcd->pauseUpdates(2000);
                                          zmpt->pauseSampling(argv > 0, false);
                                          break;

                                        case CMD_MODIFY_HZ:
                                          if(mode != MODE_TEST)break;
                                          //lcd->printLine("Modify hz!");
                                          if(argv == 1){
                                              zmpt->assignResults(zmpt->getVoltage(), zmpt->getHz() + TEST_HZ_INC);
                                          } else if(argv == -1){
                                              zmpt->assignResults(zmpt->getVoltage(), zmpt->getHz() - TEST_HZ_INC);
                                          } else {
                                              zmpt->assignResults(zmpt->getVoltage(), TEST_HZ);
                                          }
                                          break;

                                        case CMD_MODIFY_VOLTAGE:
                                          if(mode != MODE_TEST)break;
                                          lcd->printLine("Modify voltage!");
                                          if(argv == 1){
                                              zmpt->assignResults(zmpt->getVoltage() + TEST_VOLTAGE_INC, zmpt->getHz());
                                          } else if(argv == -1){
                                              zmpt->assignResults(zmpt->getVoltage() - TEST_VOLTAGE_INC, zmpt->getHz());
                                          } else {
                                              zmpt->assignResults(TEST_VOLTAGE, zmpt->getHz());
                                          }
                                          break;

                                        case CMD_ROTATE_SERVO:
                                          if(mode != MODE_MANUAL)break;
                                          if(argv == 1){
                                              svc->rotateBy(SERVO_ROTATE_INC);
                                          } else if(argv == -1){
                                              svc->rotateBy(-SERVO_ROTATE_INC);
                                          } else {
                                              svc->moveTo(SERVO_START_POS);
                                          }
                                          break;
                                      }
                                    } else {
                                      switch(message->type){
                                        case ADMMessage::MessageType::TYPE_FINALISE:
                                          clientConnected = false;
                                          break;
                                        default:
                                          clientConnected = true;
                                          break;
                                      }
                                    }
                                  });

  //Create the devices and configure
  lcd = (LCD*)ADM->addDevice(LCD_ID, ArduinoDevice::Category::LCD, "LCD");
  lcd->setPins(LCD::DataPinSequence::Pins_2_5, LCD_ENABLE_PIN, LCD_RS_PIN);
  lcd->setDimensions(LCD::DisplayDimensions::D16x2);
  lcd->setAsReady(true); //enable at the same time

  zmpt = (ZMPT101B*)ADM->addDevice(ZMPT_ID, ArduinoDevice::Category::VAC_SENSOR, "ZMPT101B");
  zmpt->setVoltagePin(ZMPT_PIN);
  zmpt->setReportInterval(ZMPT_REPORT_INTERVAL);
  zmpt->setTargetParameters(ZMPT101B::Target::HZ,
                            ZMPT_HZ_TARGET, 
                            ZMPT_HZ_TARGET_TOLERANCE,
                            ZMPT_HZ_LOWERBOUND, 
                            ZMPT_HZ_UPPERBOUND);
  zmpt->addEventListener([](ArduinoDevice* device, int eventID){
                            char output[20];
                            switch(eventID){
                              case ZMPT101B::EVENT_NEW_RESULTS:
                                if(ALLOW_SERIAL_PRINT){
                                  Serial.print("Updated zmpt ");
                                  Serial.print(zmpt->getSummary());
                                  Serial.println();
                                }
                                if(clientConnected){
                                  sprintf(output, "*%s", zmpt->getSummary());
                                } else {
                                  sprintf(output, "%s", zmpt->getSummary());
                                }
                                lcd->printLine(output);
                                break;

                              case ZMPT101B::EVENT_TARGET_REACHED:
                                if(ALLOW_SERIAL_PRINT){
                                  Serial.print("Target reached!");
                                  Serial.println();
                                }
                                targetLost = false;
                                char output[32];
                                if(svc->reachedUpperBound()){
                                  sprintf(output, "> SP=%d (UB) <", svc->getPosition());
                                } else if(svc->reachedLowerBound()){
                                  sprintf(output, "> SP=%d (LB) <", svc->getPosition());
                                } else {
                                  sprintf(output, "> SP=%d <", svc->getPosition());
                                }
                                lcd->printLine(output, 1);
                                break;

                              case ZMPT101B::EVENT_TARGET_LOST:
                                if(mode == MODE_MANUAL)break;
                                
                                if(ALLOW_SERIAL_PRINT){
                                  Serial.print("Adjustment required: ");
                                  Serial.println();
                                }
                                targetLost = true;
                                break;

                              case ZMPT101B::EVENT_OUT_OF_TARGET_RANGE:
                                if(mode == MODE_MANUAL || !engineOn)break;
                                targetLost = false;
                                lcd->printLine("Out of range!", 1);
                                lcd->pauseUpdates(2000);
                                break;
                              
                            }
                            return true;
                          });
  zmpt->setAsReady(true); //enable at the same time
  zmpt->pauseSampling(true, true);

  svc = (ServoController*)ADM->addDevice(SVC_ID, ArduinoDevice::Category::SERVO, "SERVO");
  svc->setPin(SVC_SERVO_PIN);
  svc->setBounds(45, 135);
  svc->createServo(Servo::ServoModel::MG996, SERVO_START_POS, 0, SERVO_RESOLUTION);
  /*svc->addEventListener([](ArduinoDevice* device, int eventID){
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
  bsp->addEventListener([](ArduinoDevice* device, int eventID){
                        switch(eventID){
                            case SwitchDevice::EVENT_SWITCH_TRIGGERED:
                              lcd->clear();
                              if(bsp->isOn()){
                                lcd->printLine("Bosspump On!");
                                bspOnAt = millis();
                                engineOn = false;
                              } else {
                                lcd->printLine("Engine Off");
                                zmpt->pauseSampling(true, true);
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
  
  delay(1000);
}

void loop() {
  ADM->loop();

  //monitoring engine on/off
  if(bsp->isOn() && !engineOn && ((millis() - bspOnAt) > 2500)){
    engineOn = true;
    lcd->printLine("Engine On!");
    lcd->pauseUpdates(1500);
    zmpt->pauseSampling(false, true);
  }

  //monitoring electricity
  static unsigned long ms2 = millis();
  static unsigned int waitFor = 0; //wait a number of cycles
  static double lastValue = 0.0;
  if(mode != MODE_MANUAL && targetLost && ((millis() - ms2) > 250)){
    ms2 = millis();
    
    if(waitFor > 0){
      if(waitFor < 5){
        lcd->printLine("", 1);
      }
      waitFor--;
    } else {
      //if targeting value goes out of bounds then adjust according to desired direction
      char output[32];
      double currentValue = zmpt->getCurrentValue();
      ZMPT101B::Direction resultDir = lastValue == 0.0 ? zmpt->getCurrentDirection() : zmpt->getDirection(currentValue, lastValue);
      ZMPT101B::Direction desiredDir = zmpt->getDesiredDirection();
      lastValue = currentValue;

      if(resultDir != desiredDir){ //Take some action
        switch(desiredDir){
          case ZMPT101B::Direction::Falling:
            svc->rotateBy(-SERVO_ROTATE_INC);
            if(svc->reachedLowerBound()){
              lcd->printLine("<<<< SP=LB!", 1);
            } else {
              sprintf(output, "<<<< SP <- %d", svc->getPosition());
              lcd->printLine(output, 1); 
            }
            waitFor = WAIT_AFTER_TARGET_ACTION;
            break;

          case ZMPT101B::Direction::Rising:
            svc->rotateBy(SERVO_ROTATE_INC);
            if(svc->reachedUpperBound()){
              lcd->printLine(">>>> SP=UB!", 1);
            } else {
              sprintf(output, ">>>> SP -> %d", svc->getPosition());
              lcd->printLine(output, 1); 
            }
            waitFor = WAIT_AFTER_TARGET_ACTION;
            break;

          default:
            lcd->printLine("Weird1", 1);
            break;
        }
      } else { //going in the right direction
        switch(resultDir){
          case ZMPT101B::Direction::Falling:
            lcd->printLine("<<<< Waiting...", 1);
            waitFor = WAIT_AFTER_TARGET_ACTION;
            break;

          case ZMPT101B::Direction::Rising:
            lcd->printLine(">>>> Waiting...", 1);
            waitFor = WAIT_AFTER_TARGET_ACTION;
            break;

          default:
            lcd->printLine("Weird2", 1);
            break;
        }
      }
    } //end wait conditional
  } //end montior loop

  static unsigned long ms3 = millis();
  //ensuring results are assigend during test mode
  if(zmpt->isSamplingPaused() && mode == MODE_TEST && ((millis() - ms3) > 1000)){
      ms3 = millis();
      zmpt->assignResults(zmpt->getVoltage(), zmpt->getHz());
  }

}
