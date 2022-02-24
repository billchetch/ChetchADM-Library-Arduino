#ifndef CHETCH_ADM_CONFIG_H
#define CHETCH_ADM_CONFIG_H

#if defined(ARDUINO_AVR_UNO)
	//Uno specific code
	#define MAX_DEVICES 8	
    #define BOARD_NAME "UNO"	
#elif defined(ARDUINO_AVR_MEGA2560)
	//Mega 2560 specific code
	#define MAX_DEVICES 32
    #define BOARD_NAME "MEGA"
#elif defined(ARDUINO_SAM_DUE)
	#define MAX_DEVICES 16
    #define BOARD_NAME "SAM"
#elif defined(ESP8266)
	#define MAX_DEVICES 8
    #define BOARD_NAME "ESP8266"
#else
    #define MAX_DEVICES 8
    #define BOARD_NAME "OTHER"
//#error Unsupported hardware
#endif

#define TEMPERATURE_DEVICES 1
#define RANGE_FINDER_DEVICES 2
#define IR_DEVICES 4
#define ELECTRICITY_MEASURING_DEVICES 8
#define DIAGNOSTIC_DEVICES 16
#define MOTOR_DEVICES 32
#define DISPLAY_DEVICES 64 


//!!!Specify here devices beyond the default set that should be included ... combinations allowed by OR-ing
//#define INCLUDE_DEVICES TEMPERATURE_DEVICES
//#define INCLUDE_DEVICES RANGE_FINDER_DEVICES
//#define INCLUDE_DEVICES ELECTRICITY_MEASURING_DEVICES
//#define INCLUDE_DEVICES TEMPERATURE_DEVICES + ELECTRICITY_MEASURING_DEVICES + DIAGNOSTIC_DEVICES + MOTOR_DEVICES + DISPLAY_DEVICES
#define INCLUDE_DEVICES IR_DEVICES + DIAGNOSTIC_DEVICES

#ifndef CHETCH_ADM_ADM_H
	#if (INCLUDE_DEVICES & IR_DEVICES) == IR_DEVICES 
		#include <IRremote.hpp>
	#endif
#endif

#endif