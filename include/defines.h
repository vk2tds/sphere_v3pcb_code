
#ifndef DEFINES_H
#define DEFINES_H

#define COPYRIGHT "INIT: COPYRIGHT 2025 SPHERE GROUP PTY LTD"

#include "NonBlockingModbusMaster.h"


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
#define MAX_MODBUS 16                   // Maximum modbus addresses to scan
#define MAX_MODBUSINSTANCES 1

#define NO_PIN 12288 // TODO: Verify that this pin is NOT being used in real life


#define HMI_ALWAYS 0
#define HMI_CLI 2
#define HMI_CONFIG 3
#define HMI_STATUS 4
#define HMI_TRACE 5



#include <Arduino.h>





// https://s.campbellsci.com/documents/us/manuals/climavue40.pdf
// 1 - read coils
// 2 - read discrete inputs
// 3 - Holding Registers
// 4 - Read input registers
// 5 - write single coil
// 6 - write single register
// F - Write multiple coils
// 10 - write multiple registers
// 16 - mask write registers
// 17 - read/write multiple registers

#define MODBUS_COILS 1
#define MODBUS_DISCRETE_INPUTS 2
#define MODBUS_HOLDING_REGISTERS 3
#define MODBUS_INPUT_REGISTERS 4

struct ModbusScan{
  uint8_t type;             // Coils == 1, etc
  uint16_t start_address;   // Modbus Address
  uint16_t values;          // How many values to scan
  uint16_t scan_frequency;  // How often to scan
};



enum Port {
  port_USB,               // Main USB serial port
  port_ESP32,             // ESP32 Serial Port
  port_ethernet_telnet,   // Telnet on Ethernet 
  port_serial_ac,         // Air Conditioner
  port_serial_rs485,      // expansion interface
};


enum PortFunction{
  portfunction_cli,               // Command Line Interface
  portfunction_serialMQTT,        // MQTT over Serial
  portfunction_serialMQTTslave,   // Slave Serial over MQTT
  portfunction_serialAC,          // Air Conditioner
  portfunction_serialMODBUS       // Modbus
};

#define BUFFER_SIZE_TX 256
#define BUFFER_SIZE_RX 256


struct PortInformation{
  PortFunction portfunction;

  uint8_t RxBuffer[BUFFER_SIZE_TX];
  uint16_t RxBufferSize;
  uint32_t RxTimeout; // Millis()
  uint32_t RxLast; // In Millis
  uint8_t RxCharLast = '\0';
  uint8_t RxCharBeforeThat = '\0';

  uint8_t TxBuffer[BUFFER_SIZE_RX];
  uint16_t TxBufferSize;
  Stream &s;
  NonBlockingModbusMaster &modbus;
  uint16_t serial_bps;
  uint8_t serial_config; //https://docs.arduino.cc/language-reference/en/functions/communication/serial/begin/

  bool haveUsedHMIputs = false; 

};


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




// https://s.campbellsci.com/documents/us/manuals/climavue40.pdf
// 1 - read coils
// 2 - read discrete inputs
// 3 - Holding Registers
// 4 - Read input registers
// 5 - write single coil
// 6 - write single register
// F - Write multiple coils
// 10 - write multiple registers
// 16 - mask write registers
// 17 - read/write multiple registers

#define MODBUS_COILS 1
#define MODBUS_DISCRETE_INPUTS 2
#define MODBUS_HOLDING_REGISTERS 3
#define MODBUS_INPUT_REGISTERS 4

#define MODBUX_MAX_WORDS 64

struct ModbusScan{
  uint8_t modbus_instance;  // Instance of the modbus library. 0 at the moment. 
  uint16_t modbus_address;  // Modbus Address
  uint8_t type;             // Coils == 1, etc
  uint16_t start_address;   // Modbus Address
  uint16_t values;          // How many values to scan
  uint16_t scan_frequency;  // How often to scan
  uint32_t next_scan_time;  // Secondssincestate based. 
  uint16_t data[MODBUX_MAX_WORDS];
  uint32_t secondsSinceStart;
};

struct ModbusInstance{
  NonBlockingModbusMaster &modbusinstance;
  uint32_t ptt_pin; // Push to Talk pin
  bool transmit; // if PTT is on
  Stream &serial;
  uint8_t modbusscan_index;
};

struct AirComms{
  bool tx_power;
  uint16_t tx_speed;
  uint8_t tx_buffer[16];
  uint8_t rx_buffer[16];
  uint16_t rx_compressor_speed; // 0.1 units
  uint16_t rx_compressor_currernt; // 0.1 units
  uint16_t rx_busbar_voltage; // 0.1 units
  // The following two values are bitmaps
  // * Bit0 = Software OverCurrent
  // * Bit1 = OverVoltage Protection
  // * Bit2 = UnderVoltage Protection
  // * Bit3 = Phase Loss Protection
  // * Bit4 = Stall Protection
  // * Bit5 = Hardware overcurrent protection
  // * Bit6 = Abnormal Phase Current
  // The now value resets after 120 seconds. 
  uint8_t rx_status_now; 
  uint8_t rx_status_historical; 
  uint32_t rx_secondssincestart;
};


#define MQTT_REP_SOFTHARDWARE 0
#define MQTT_REP_ADC 1
//#define MQTT_REP_ADC_CURRENT 2
#define MQTT_REP_FAN 3
//#define MQTT_REP_FAN_PWM 4
#define MQTT_REP_OUTPUTS 5
#define MQTT_REP_LOCK 6
#define MQTT_REP_TEMPERATURE 7
#define MQTT_REP_AIRCOND 8
#define MQTT_REP_LED 9
#define MQTT_REP_SETTINGS 10 // Including calibration
#define MQTT_REP_MODBUS 11


// Only itens within this list will be reported via MQTT. Setting a frequency of 0 means 
// it will only send on change. The idea behind next_report is that you can set this to 
// secondssincestart+n and it will report then regardless.
struct MqttReporting{
  uint8_t sensor;
  uint8_t mqtt_base[64];
  uint16_t frequency;
  uint32_t next_report;

};


#endif 