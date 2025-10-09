
#ifndef DEFINES_H
#define DEFINES_H

#define COPYRIGHT "INIT: COPYRIGHT 2025 SPHERE GROUP PTY LTD"

// fallback if not set by platformio
#ifndef LAST_BUILD_TIME
#define LAST_BUILD_TIME 2025
#endif



#define MAX_FAN_PWM_RPM 6
#define MAX_FAN_POWER 6
#define MAX_TEMP_STRINGS 8
#define MAX_TEMP_SENSORS_PER_STRING 4
#define MAX_DOORS 1
#define MAX_ADC 32

#define NO_PIN 12288 // TODO: Verify that this pin is NOT being used in real life

#include <Arduino.h>


enum Voltage {
    voltage_12,         // 12V outputs
    voltage_48,         // 48V outputs
    voltage_supply,     // High Current 
    voltage_fans,       // Fans
    voltage_lights      // Lights, including RGB and Internal. Not the flood light
};

#define INVALID_ADC_ADDRESS 0xFFFF

struct Hardware{
    Voltage mode;
    uint8_t index;
    uint32_t power_pin;
    uint16_t current_adc_address;
    uint16_t voltage_adc_address;
    //boolean value; // Current output value // We can do this live
    char name[48];
};

struct TempPins{ // Temperature
    uint8_t index;
    uint32_t temp_pin;
};

struct FanPorts{
    uint8_t index;
    uint32_t pwm_pin;    
    uint32_t rpm_pin;    
};

struct DoorPins{
  uint8_t index;
  bool h_bridge; // True = H-Bridge, idle off; False = power and direction
  uint32_t pin_a; // H-Bridge = Open. Else Power
  uint32_t pin_b; // H-Bridge = Close. Else Direction
};



struct FanPWMRPM{
    uint32_t rpmcount;
    uint32_t lastrpmcount;
    uint32_t timediff;
    uint32_t countdiff;
    uint8_t pwm;
    uint32_t secondsSinceStart;
};


struct ADCstorage{
  uint16_t adc;
  uint32_t secondsSinceStart;
};


struct TEMPstorage{
  float temperature;
  uint32_t secondsSinceStart;
};

struct DOORstorage{
  bool DoorState;
};




#endif 