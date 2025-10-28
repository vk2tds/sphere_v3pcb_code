/* Copyright (c) 2023-2024 Radioactive Networks Pty Ltd */
/* All Rights Reserved                                  */
/* darryl@radio-active.net.au                           */

#include <ParseCommands.h>

#include "defines.h"
#include "everything.h"
#include "hmi.h"

ParseCommands pCmd;

extern struct Hardware h[];
extern uint8_t h_elementcount;
extern int safe_strcat(char *s1, char *s2, size_t s1_size);
extern struct DOORstorage doorstorage [MAX_DOORS];;
extern struct ADCstorage adcstorage[MAX_ADC];
extern struct TEMPstorage tempstorage [MAX_TEMP_STRINGS];
extern uint32_t secondsSinceStart;
extern struct ModbusInstance modbusinstance[MAX_MODBUSINSTANCES];
extern struct ModbusScan modbusscan[MAX_MODBUS];
extern struct AirComms aircomms;
extern struct MqttReporting mqttreporting [];
extern uint8_t mqttreporting_elementcount;



extern void door (uint8_t door_number, uint8_t dooraction);

// Commands
//
// door - last status
// door [DoorNumber] {1|0|open|close|lock|unlock} - lock and unlock doors
//
// temp
// temp mode {single|string}
// temp mode [StringNumber] {single|string}
// temp alarm stuff?
//
// ip...
//     DHCP, IP, Mask, Gateway, DNS1/2
//
// mqtt...
//     host, port, encrypt, usernamem password, which ports,
//     {primary|secondary|round robin}, subscriptions CLI over MQTT reporting
//     frequency
//
// port... what data goes where
// port {com1|com2|com3|usb|ethernet} {cli|mqtt|slave|hvac}
//     serial speeds too etc
//

//
// led... Is this flash mode?
//
// power {12v|48v|supply|fans|lights} [CircuitNumber] {0|1|On|Off|flash|toggle
// with time}
//
// serial - display serial number/MAC/firmware/compile date etc
//
// trace - see messages coming in on each port
//
// reboot period automatic

// MQTT
//
// Send serial number, software version, software date etc
// Send messages on change of state of objects
// Commands to do stuff, like restart hardware,

// Todo: Need to move these to setup() somehow. Not sure how :(
struct pcmd_command_t commandList[] = {
    // command, callback function
    "help",      CmdHelp,   
    "reset",  CmdReset,  
    "restart", CmdReset,    
    "info",    CmdInfo,  
    "FACTORY", CmdFactory,
    "stats",     CmdStats,  
    "door",   CmdDoor,                          // Done
    "temp",      CmdTemp,                       // Done
    "power",     CmdPower,                      // Done
    "ip",        CmdIp,     
    "mqtt",   CmdMqtt,   
    "port", CmdPort,     
    "fan",     CmdFan,   
    "led",     CmdLed,
    "modbus",   CmdModbus,
    "serial",    CmdSerial, 
    "trace",  Cmdtrace,  
    "hardware", CmdHardware, 
    "speed",   CmdSpeed, 
    "modbus", CmdModbus,
    "ac", CmdAc,
    NULL,      NULL  // END OF LIST (NEEDED)
};
ParseCommands pCmd(commandList, 64, 12);  // Constructor

bool check_if_true(int argc, char* argv[], int position) {
    if (position > argc) {
        char buf[64];
        snprintf(buf, 64, "Out of Bounds in check_if_true!!!");
        hmiPuts(buf, HMI_CLI);
        return false;
    }
    if ((argv[position][0] == '1') | (argv[position][0] == 't') |
        (argv[position][0] == 'T') | (argv[position][0] == 'y') |
        (argv[position][0] == 'Y')) {
        return true;
    } else {
        return false;
    }
}

void infoDoor(void) {
    char buf[64];
    snprintf(buf, 64, "door [Door Number] {0|lock|1|unlock|open}");
    hmiPuts(buf, HMI_CLI);

    for (uint8_t i = 0; i < MAX_DOORS; i++) {
        switch (doorstorage[i].DoorState) {
            case (false):
                snprintf(buf, 64, "door %d close", i + 1);
                hmiPuts(buf, HMI_CLI);
                break;
            case (true):
                snprintf(buf, 64, "door %d open", i + 1);
                hmiPuts(buf, HMI_CLI);
                break;
        }
    }
}

void CmdDoor(int argc, char* argv[]) {
    if (argc == 0) {
        infoDoor();
        return;
    }

    if (argc < 2) {
        char buf[64];
        snprintf(buf, 64, "Err: Syntax Error");
        hmiPuts(buf, HMI_CLI);
        return;
    }

    uint8_t dPort = parseInt(argv[0]);
    uint8_t dooraction = parseInt(argv[1]);  // parse 0 or 1

    if ((dPort > MAX_DOORS) || (dPort == 0)) {
        char buf[64];
        snprintf(buf, 64, "Err: Invalid door value");
        hmiPuts(buf, HMI_CLI);
        return;
    }

    if ((argv[1][0] == 'u') || (argv[1][0] == 'U')) {  // Unlock
        dooraction = 1;
    }

    if ((argv[1][0] == 'o') ||
        (argv[1][0] ==
         'O')) {  // Open - Not used at the moment with these locks
        dooraction = 2;
    }
    door(dPort - 1, dooraction);
}

void infoTemp(void) {
    char buf[64];
    snprintf(buf, 64, "temp [temp string] {temperature} {age}");
    hmiPuts(buf, HMI_CLI);

    for (uint8_t i = 0; i < MAX_TEMP_STRINGS; i++) {
        // TODO: Printing floats
        uint32_t diff;
        diff = secondsSinceStart - tempstorage[i].secondsSinceStart;
        snprintf(buf, 64, "temp %d %2.1f %lds", i, tempstorage[i].temperature, diff);
        hmiPuts(buf, HMI_CLI);
    }
}

void CmdTemp(int argc, char* argv[]) {
    if (argc == 0) {
        infoTemp();
        return;
    }

    char buf[64];
    snprintf(buf, 64, "Err: Additional Temp Settings Not Implemented");
    hmiPuts(buf, HMI_CLI);
    return;
}

void age (uint32_t s)
{
    char buf[64];
    snprintf(buf, 64, "  Age: %ld", secondsSinceStart - s);
    hmiPuts(buf, HMI_CLI);
}

void infoModbus (void)
{
    char buf[128];
    snprintf (buf, 128, "modbus scan [instance] [modbus addr] [type] [start address] [values] [frequency]");
    hmiPuts(buf, HMI_CLI);
    for (uint8_t i=0; i < MAX_MODBUS; i++){
        ModbusScan &ms = modbusscan[i];
        if (ms.scan_frequency > 0){
            snprintf (buf, 128, "modbus scan %02d %02d %02x %05d %04d %03d", 
                ms.modbus_instance, ms.modbus_address, ms.type, ms.start_address, ms.values, ms.scan_frequency);
            hmiPuts(buf, HMI_CLI);
            age(ms.secondsSinceStart);
            for (uint8_t j = 0; j < ms.values; j++){
                switch (ms.type){
                    case MODBUS_COILS:
                    case MODBUS_DISCRETE_INPUTS:
                        uint16_t temp = ms.data[j>>4];
                        if (temp & (1 << 0x0F)){
                            snprintf (buf, 128, "  %05d: True", ms.start_address+j);
                            hmiPuts(buf, HMI_CLI);
                        } else {
                            snprintf (buf, 128, "  %05d: False", ms.start_address+j);
                            hmiPuts(buf, HMI_CLI);
                        }
                        break;
                    case MODBUS_HOLDING_REGISTERS:
                    case MODBUS_INPUT_REGISTERS:
                        snprintf (buf, 128, "  %05d: %d", ms.start_address+j, ms.data[j]);
                        hmiPuts(buf, HMI_CLI);
                        break;
                    default:
                        snprintf (buf, 128, "  %05d: Error Unknown", ms.start_address+j);
                        hmiPuts(buf, HMI_CLI);
                }
            }
        }
    }
}


void CmdModbus(int argc, char* argv[]) {
    // modbus
    // modbus scan [instance] [modbus addr] [type] [start address] [values] [frequency]

    if (argc == 0) {
        infoModbus();
        return;
    }

    if (argc < 2) {
        char buf[64];
        snprintf(buf, 64, "Err: Syntax Error");
        hmiPuts(buf, HMI_CLI);
        return;
    }

    uint8_t dPort = parseInt(argv[0]);
    uint8_t dooraction = parseInt(argv[1]);  // parse 0 or 1

    if ((dPort > MAX_DOORS) || (dPort == 0)) {
        char buf[64];
        snprintf(buf, 64, "Err: Invalid door value");
        hmiPuts(buf, HMI_CLI);
        return;
    }

    if ((argv[1][0] == 'u') || (argv[1][0] == 'U')) {  // Unlock
        dooraction = 1;
    }

    if ((argv[1][0] == 'o') ||
        (argv[1][0] ==
         'O')) {  // Open - Not used at the moment with these locks
        dooraction = 2;
    }
    door(dPort - 1, dooraction);
}


void infoMqtt (void)
{
    char buf[128];

    for (uint8_t i = 0; i < mqttreporting_elementcount; i++){
        char buf2[64];
        switch (mqttreporting[i].sensor){
            case MQTT_REP_SOFTHARDWARE:
                snprintf (buf2, 64, "Board software and hardware");
                break;
            case MQTT_REP_TEMPERATURE:
                snprintf (buf2, 64, "Temperature                ");
                break;
            case MQTT_REP_ADC_VOLTS:
                snprintf (buf2, 64, "Voltage Sensors            ");
                break;
            case MQTT_REP_ADC_CURRENT:
                snprintf (buf2, 64, "Current Sensors            ");
                break;
            case MQTT_REP_FAN_RPM:
                snprintf (buf2, 64, "Fan RPM Measurement        ");
                break;
            case MQTT_REP_FAN_PWM:
                snprintf (buf2, 64, "Fan PWM Control            ");
                break;
            case MQTT_REP_OUTPUTS:
                snprintf (buf2, 64, "Switch Outputs             ");
                break;
            case MQTT_REP_LOCK:
                snprintf (buf2, 64, "Door Locks                 ");
                break;
            case MQTT_REP_AIRCOND:
                snprintf (buf2, 64, "Air Conditioner            ");
                break;
            case MQTT_REP_LED:
                snprintf (buf2, 64, "LEDs                       ");
                break;
            case MQTT_REP_MODBUS:
                snprintf (buf2, 64, "Modbus                     ");
                break;
            case MQTT_REP_SETTINGS:
                snprintf (buf2, 64, "Settings                   ");
                break;
        }
        uint32_t next_age =  mqttreporting[i].next_report - secondsSinceStart;

        snprintf (buf, 128, "mqtt reporting - %s - %s - %d %ld", buf2, mqttreporting[i].mqtt_base, mqttreporting[i].frequency, next_age);
        hmiPuts(buf, HMI_CLI);
    }



}



void CmdMqtt(int argc, char* argv[]) {

    if (argc == 0){
        infoMqtt();
        return;
    }


}


void infoAc (void)
{
    char buf[128];

    char buf_voltage[16];
    char buf_current[16];

    snprintf (buf_voltage, 16, "%05d", aircomms.rx_busbar_voltage);
    buf_voltage[6] = 0x00;
    buf_voltage[5] = buf_voltage[4];
    buf_voltage[4] = '.';

    snprintf (buf_current, 16, "%05d", aircomms.rx_compressor_currernt);
    buf_current[6] = 0x00;
    buf_current[5] = buf_current[4];
    buf_current[4] = '.';


    
    snprintf (buf, 128, "ac rx %sV %sA %04dRPM", buf_voltage, buf_current, aircomms.rx_compressor_speed);
    hmiPuts(buf, HMI_CLI);

    for (uint8_t i = 0; i < 1; i++){
        uint8_t errors;
        switch (i){
            case 0: // Live Errors
                errors = aircomms.rx_status_now;
                snprintf (buf, 128, "ac errors live");
                hmiPuts(buf, HMI_CLI);
                break;
            case 1: // Historical Errors
                errors = aircomms.rx_status_historical;
                snprintf (buf, 128, "ac errors since poweron");
                hmiPuts(buf, HMI_CLI);
                break;
        }
        if (errors == 0){
            snprintf (buf, 128, "  - none");
            hmiPuts(buf, HMI_CLI);
        } else {
            if (errors && 0x01){
                snprintf (buf, 128, "  - bit 0 - Software Over-current");
                hmiPuts(buf, HMI_CLI);
            }
            if (errors && 0x02){
                snprintf (buf, 128, "  - bit 1 - Over-voltage protection");
                hmiPuts(buf, HMI_CLI);
            }
            if (errors && 0x04){
                snprintf (buf, 128, "  - bit 2 - Under-voltage protection");
                hmiPuts(buf, HMI_CLI);
            }
            if (errors && 0x08){
                snprintf (buf, 128, "  - bit 3 - Phase loss protection");
                hmiPuts(buf, HMI_CLI);
            }
            if (errors && 0x10){
                snprintf (buf, 128, "  - bit 4 - Stall protection");
                hmiPuts(buf, HMI_CLI);
            }
            if (errors && 0x20){
                snprintf (buf, 128, "  - bit 5 - Hardware Over-current protection");
                hmiPuts(buf, HMI_CLI);
            }
            if (errors && 0x40){
                snprintf (buf, 128, "  - bit 6 - Abnormal Phase Current");
                hmiPuts(buf, HMI_CLI);
            }
        }
    }


}



void CmdAc(int argc, char* argv[]) {
    //Ac 

    if (argc == 0) {
        infoAc();
        return;
    }

    if (argc < 2) {
        char buf[64];
        snprintf(buf, 64, "Err: Syntax Error");
        hmiPuts(buf, HMI_CLI);
        return;
    }

    uint8_t dPort = parseInt(argv[0]);
    uint8_t dooraction = parseInt(argv[1]);  // parse 0 or 1

    if ((dPort > MAX_DOORS) || (dPort == 0)) {
        char buf[64];
        snprintf(buf, 64, "Err: Invalid door value");
        hmiPuts(buf, HMI_CLI);
        return;
    }

    if ((argv[1][0] == 'u') || (argv[1][0] == 'U')) {  // Unlock
        dooraction = 1;
    }

    if ((argv[1][0] == 'o') ||
        (argv[1][0] ==
         'O')) {  // Open - Not used at the moment with these locks
        dooraction = 2;
    }
    door(dPort - 1, dooraction);
}





void infoPower(void) {
    char buf[128];
    snprintf(buf, 128,
             "power {12v|48v|supply|fans|lights} [CircuitNumber] "
             "{0|1|On|Off|flash|toggle with time} [On Time]");
    hmiPuts(buf, HMI_CLI);
    snprintf(
        buf, 128,
        "  - WARNING: Output SUPPLY/0 is INVERTED. ON turns the output OFF");
    hmiPuts(buf, HMI_CLI);

    char buf2[16];
    char buf3[16];

    char amp_buf[16];
    char volt_buf[16];

    for (uint8_t i = 0; i < h_elementcount; i++) {
        snprintf(buf2, 64, "ERROR  ");

        if (h[i].mode == voltage_12) {
            snprintf(buf2, 64, "12v    ");
        }
        if (h[i].mode == voltage_48) {
            snprintf(buf2, 64, "48v    ");
        }
        if (h[i].mode == voltage_supply) {
            snprintf(buf2, 64, "supply ");
        }
        if (h[i].mode == voltage_fans) {
            snprintf(buf2, 64, "fans   ");
        }
        if (h[i].mode == voltage_lights) {
            snprintf(buf2, 64, "lights ");
        }

        snprintf(buf3, 16, "ALWAYS ON ");
        if (h[i].power_pin != NO_PIN) {
            if (digitalRead(h[i].power_pin)) {
                snprintf(buf3, 16, "on       ");
            } else {
                snprintf(buf3, 16, "off      ");
            }

            snprintf(amp_buf, 16, "       ");
            snprintf(volt_buf, 16, "       ");

            if (h[i].current_adc_address != INVALID_ADC_ADDRESS) {
                snprintf(amp_buf, 16, "%2.3fA",
                         adcstorage[h[i].current_adc_address]);
            }

            if (h[i].voltage_adc_address != INVALID_ADC_ADDRESS) {
                snprintf(amp_buf, 16, "%2.2f V",
                         adcstorage[h[i].current_adc_address]);
            }

            snprintf(buf, 128, "%s %02d %s %s %s", buf2, h[i].index, buf3,
                     amp_buf, volt_buf);
            hmiPuts(buf, HMI_CLI);
        }
    }
}

//  char *command_line = "power {12v|48v|supply|fans|lights} [CircuitNumber]
//  {0|1|On|Off|flash|toggle with time} [On Time]";

void CmdPower(int argc, char* argv[]) {
    if (argc == 0) {
        infoPower();
        return;
    }

    if (argc < 3) {
        char buf[64];
        snprintf(buf, 64, "Err: Syntax Error");
        hmiPuts(buf, HMI_CLI);
        return;
    }

    uint8_t mode;

    if (strcasecmp(argv[1], "12v") == 0) {
        mode = voltage_12;
    }
    if (strcasecmp(argv[1], "48v") == 0) {
        mode = voltage_48;
    }
    if (strcasecmp(argv[1], "supply") == 0) {
        mode = voltage_supply;
    }
    if (strcasecmp(argv[1], "fans") == 0) {
        mode = voltage_fans;
    }
    if (strcasecmp(argv[1], "lights") == 0) {
        mode = voltage_lights;
    }

    uint8_t circuit = parseInt(argv[2]);

    uint8_t state = parseInt(argv[3]);  // Picks up 1 & 0

    if (strcasecmp(argv[3], "on") == 0) {
        state = 1;
    }

    if (strcasecmp(argv[3], "pulse") == 0) {
        state = 2;
    }

    if (strcasecmp(argv[3], "flash") == 0) {
        state = 3;
    }

    if (state > 1) {
        if (argc < 3) {
            char buf[64];
            snprintf(buf, 64, "Err: Syntax Error");
            hmiPuts(buf, HMI_CLI);
            return;
        }
    }

    for (uint8_t i = 0; i < h_elementcount; i++) {
        if ((h[i].mode == mode) && (h[i].index == circuit)) {
            if (state == 0) digitalWrite(h[i].power_pin, false);
            if (state == 1) digitalWrite(h[i].power_pin, true);
            if (state > 1) {
                char buf[64];
                snprintf(buf, 64, "Err: Output not found");
                hmiPuts(buf, HMI_CLI);
                return;
            }
            return;
        }
    }
}



void CmdOutput(int argc, char* argv[]) {
    if (argc == 0) {
        infoOutput();
        return;
    }

    if (argc < 2) {
        char buf[64];
        snprintf(buf, 64, "Err: Syntax Error");
        hmiPuts(buf, HMI_CLI);
    }

    uint8_t oPort = parseInt(argv[0]);
    ;
    uint8_t wPort = parseInt(argv[1]);
    ;

    if ((argv[1][0] == 'f') || (argv[1][0] == 'o')) {
        wPort = 0;
    }

    if ((oPort > OSDP_MAX_OSDP_COUNT) || (oPort == 0)) {
        char buf[64];
        snprintf(buf, 64, "Err: OSDP port is bad");
        hmiPuts(buf, HMI_CLI);
        return;
    }

    if ((wPort > OSDP_MAX_WIEGAND_COUNT)) {
        char buf[64];
        snprintf(buf, 64, "Err: OSDP port is bad");
        hmiPuts(buf, HMI_CLI);
        return;
    }

    if (wPort == 0) {
        settings.wiegand_output[oPort - 1] = 0xff;
    } else {
        settings.wiegand_output[oPort - 1] = wPort - 1;
    }
    char buf[64];
    if (settings.wiegand_output[oPort - 1] == 0xff) {
        snprintf(buf, 64, "Restart required");
        hmiPuts(buf, HMI_CLI);
    } else {
        snprintf(buf, 64, "Restart suggested");
        hmiPuts(buf, HMI_CLI);
        pinMode(reader[settings.wiegand_output[oPort - 1]].in_D0, OUTPUT);
        pinMode(reader[settings.wiegand_output[oPort - 1]].in_D1, OUTPUT);
        digitalWrite(reader[settings.wiegand_output[oPort - 1]].in_D0, HIGH);
        digitalWrite(reader[settings.wiegand_output[oPort - 1]].in_D1, HIGH);
    }

    settings_save();
}

void CmdSpeed(int argc, char* argv[]) {
    printCommand();
    if (argc == 0) {
        infoSpeed();
        return;
    }
    uint32_t speed = parse32Int(argv[0]);
    char buf[128];

    switch (speed) {
        case 9600:
        case 19200:
        case 38400:
        case 57600:
        case 115200:
            settings.speed = speed / 9600;
            settings_save();
            snprintf(buf, 120, "OSDP speed set. Please restart unit to enable");
            hmiPuts(buf, HMI_CLI);
            return;
            break;
        default:
            snprintf(buf, 128,
                     "Speed must be 9600, 19200, 38400, 57600 or 115200");
            hmiPuts(buf, HMI_CLI);
            return;
    }
}



void infoSpeed(void) {
    char buf[64];
    snprintf(buf, 64, "speed %d", settings.speed * 9600);
    hmiPuts(buf, HMI_CLI);
}

void CmdLed(int argc, char* argv[]) {
    printCommand();

    if (!checkPassword()) return;  // Console locked

    if (argc == 0) {
        infoLed();
        return;
    }
    if (argc >= 1) {
        uint8_t led_flash = parseInt(argv[0]);
        if (led_flash < 4) {
            settings.flash_LED_on_serial = led_flash;
        } else {
            settings.flash_LED_on_serial = 0;
        }

        settings_save();
        infoLed();
    }
}

void CmdFactory(int argc, char* argv[]) {
    printCommand();

    if (!checkPassword()) return;  // Console locked

    char buf[64];
    snprintf(buf, 64, "Resetting to factory settings");
    hmiPuts(buf, HMI_CLI);

    settings_destroy();
    CmdReset(0, NULL);
}

void CmdSerial(int argc, char* argv[]) {
    printCommand();
    infoSerial();
}

void CmdStats(int argc, char* argv[]) {
    printCommand();

    //if (!checkPassword()) return;  // Console locked

    infoStatus();
}



void infoLed(void) {
    char buf[64];

    switch (settings.flash_LED_on_serial) {
        case 0:
            snprintf(buf, 64, "led 0 # flash LED as watchdog");
            hmiPuts(buf, HMI_CLI);
            break;
        case 1:
            snprintf(buf, 64, "led 1 # flash LED on serial activity");
            hmiPuts(buf, HMI_CLI);
            break;
        case 2:
            snprintf(buf, 64, "led 2 # flash LED on wiegand activity");
            hmiPuts(buf, HMI_CLI);
            break;
        case 3:
            snprintf(buf, 64, "led 3 # flash LED on libosdp activity");
            hmiPuts(buf, HMI_CLI);
            break;
        default:
            snprintf(buf, 64, "led %d unknown", settings.flash_LED_on_serial);
            hmiPuts(buf, HMI_CLI);
            break;
    }
}

void infoSerial(void) {
    char buf[64];

    snprintf(buf, 64, "serial %08x", settings.base_serial_no);
    hmiPuts(buf, HMI_CLI);
}

void infoOutput(void) {
    char buf[64];
    snprintf(buf, 64, "output osdp_port wiegand_port");
    hmiPuts(buf, HMI_CLI);

    for (uint8_t i = 0; i < OSDP_MAX_OSDP_COUNT; i++) {
        if (settings.wiegand_output[i] == 0xff) {
            snprintf(buf, 64, "output %d off", i + 1);
        } else {
            snprintf(buf, 64, "output %d %d", i + 1,
                     settings.wiegand_output[i] + 1);
        }
        hmiPuts(buf, HMI_CLI);
    }
}

void CmdInfo(int argc, char* argv[]) {
    printCommand();

    if (!checkPassword()) return;  // Console locked

    char buf[64];
    snprintf(buf, 64, "Information...");
    hmiPuts(buf, HMI_CLI);

    infoRoute();
    infoOsdp();
    infoFake();
    infoOffset();
    infoEncrypt();
    infoOutput();
    infoSerial();
    infoStatus();
    infoEnable();
    infoCp();  // This ***MUST*** be last. Changing setting.mode_cp ***WILL***
               // force a restart!
}

void CmdReset(int argc, char* argv[]) {
    printCommand();
    if (!checkPassword()) return;  // Console locked

    char buf[64];
    snprintf(buf, 64, "Resetting...");
    hmiPuts(buf, HMI_CLI);
    Serial.flush();
    NVIC_SystemReset();
}

// Commands
//
//
// ip...
//     DHCP, IP, Mask, Gateway, DNS1/2
//
// mqtt...
//     host, port, encrypt, usernamem password, which ports,
//     {primary|secondary|round robin}, subscriptions CLI over MQTT reporting
//     frequency
//
// port... what data goes where
// port {com1|com2|com3|usb|ethernet} {cli|mqtt|slave|hvac}
//     serial speeds too etc
//
//
// led... Is this flash mode?
//
//
// serial - display serial number/MAC/firmware/compile date etc
//
// trace - see messages coming in on each port
//
// reboot period automatic

// MQTT
//
// Send serial number, software version, software date etc
// Send messages on change of state of objects
// Commands to do stuff, like restart hardware,
//
// Amps and Voltage????



#define HELP_VALUE_DOOR 1
#define HELP_VALUE_TEMP 2

void printhelp(char* command, char* text, uint16_t values, uint8_t warning) {
    char buf[256];
    char buf2[256];

    hmiPuts(command, HMI_CLI);
    hmiPuts("", HMI_CLI);
    hmiPuts(text, HMI_CLI);

    if ((values > 0) | (action > 0)) {
        hmiPuts("", HMI_CLI);
    }

    // | Function                          | Minimum       | Maximum       |
    // Example                           |
    // |-----------------------------------|---------------|---------------|-----------------------------------|
    // | 1 PD OSDP Port                      | 1             | 8             | |
    // | 2 CP OSDP Port                      | 1             | 16            | |
    // | 4 OSDP Address                      | 0             | 126           | |
    // | 8 OSDP Physical Address (Read Only) | 4 Hex Digits  | 4 Hex Digits  |
    // 3FDC                              | | 16 Wiegand Port | 1             | 8
    // |                                   | | 32 Encryption Key | 32 Hex Digits
    // | 32 Hex Digits | 303132333435363738393A3B3C3D3E3F  | | 64 RFID Card
    // Number                  | 8 Hex Digits  | 10 Hex Digits | 22015993D5C1 |
    // | 128 On Time and Off Time              | 0             | 255           |
    // 5 = 0.5 Seconds; 12 = 1.2 Seconds |

    if (values & HELP_VALUE_DOOR) {
        snprintf(buf2, 256, " - [Door Number] - 1-MAX_DOORS");
        hmiPuts(buf2, HMI_CLI);
    }
    if (values & HELP_VALUE_TEMP) {
        snprintf(buf2, 256, " - [Temperature Port] - 1-MAX_TEMP");
        hmiPuts(buf2, HMI_CLI);
    }
    if (values & VALUE_OSDP_ADDRESS) {
        snprintf(buf2, 256, " - [OSDP Address] - 0-256");
        hmiPuts(buf2, HMI_CLI);
    }
    if (values & VALUE_OSDP_ADDRESS_PHY) {
        snprintf(buf2, 256,
                 " - [OSDP Physical Address] - Read Only - 4 Hex Digits - "
                 "0000-FFFF");
        hmiPuts(buf2, HMI_CLI);
    }
    if (values & VALUE_WIEGAND_PORT) {
        snprintf(buf2, 256, " - [Wiegand Port] - 1-8");
        hmiPuts(buf2, HMI_CLI);
    }
    if (values & VALUE_TRUE) {
        snprintf(
            buf2, 256,
            " - [state] - The state may be 1, 0, true or false, yes or no");
        hmiPuts(buf2, HMI_CLI);
    }
    if (values & VALUE_LED) {
        snprintf(buf2, 256,
                 " - [LED] - 0=Off. 1=Red. 2=Green. 3=Amber. 4=Blue. "
                 "5=Magenta. 6=Cyan. 7=White");
        hmiPuts(buf2, HMI_CLI);
    }

    if (warning & 0x01) {
        snprintf(buf2, 256,
                 " - WARNING: This command will cause the device to restart");
        hmiPuts(buf2, HMI_CLI);
    }
    if (warning & 0x02) {
        snprintf(buf2, 256, " - WARNING: Restart is recommended");
        hmiPuts(buf2, HMI_CLI);
    }
}

void CmdHelp(int argc, char* argv[]) {
    printCommand();

    if (argc >= 1) {
        if (strcasecmp("door", argv[0]) == 0) {
            char* command_line = "door [Door] [DoorState]... ";
            char* command_help = "Unlock, Lock or Open a door";
            uint8_t warning = 0;
            uint16_t values = HELP_VALUE_DOOR;
            printhelp(command_line, command_help, values, warning);
            return;
        }

        if (strcasecmp("temp", argv[0]) == 0) {
            char* command_line =
                "temp mode [Temp String]\r\n"
                "temp alarm [Temp String] xxxxxxx";
            char* command_help = "Unlock, Lock or Open a door";
            uint8_t warning = 0;
            uint16_t values = HELP_VALUE_TEMP;
            printhelp(command_line, command_help, values, warning);
            return;
        }

        if (strcasecmp("power", argv[0]) == 0) {
            char* command_line =
                "power {12v|48v|supply|fans|lights} [CircuitNumber] "
                "{0|1|On|Off|flash|toggle with time} [On Time]";
            char* command_help = "Turn on a circuit on and off";
            uint8_t warning = 0;
            uint16_t values = CIRCUIT_TYPE | CIRCUIT NUMBER | ON_TIME;
            printhelp(command_line, command_help, values, warning);
            return;
        }

        if (strcasecmp("fan", argv[0]) == 0) {
            char* command_line =
                "fan [Fan Number] pwm [PWM Value]\r\n"
                "fam {pwr|power} [Fan State]";
            char* command_help = "Turn on a circuit on and off";
            uint8_t warning = 0;
      uint16_t values = FAN_NUMBER | FAN_STATE | PWR_VALUE];
      printhelp(command_line, command_help, values, warning);
      return;
        }

        // fan
        // fan [FanNumber]
        // fan [FanNumber] pwm [0-100]
        // fan [FanNumber] {power|pwr} [1|0|on|off] - clone on power

        if (strcasecmp("reset", argv[0]) == 0) {
            char* command_line = "reset";
            char* command_help =
                "This command simply causes the device to restart as if it was "
                "power cycled.";
            uint8_t warning = 1;
            uint16_t values = 0;
            printhelp(command_line, command_help, values, warning);
            return;
        }

        if (strcasecmp("info", argv[0]) == 0) {
            char* command_line = "info";
            char* command_help = "Return the value of all settings.";
            uint8_t warning = 0;
            uint16_t values = 0;
            printhelp(command_line, command_help, values, warning);
            return;
        }

        if (strcasecmp("stats", argv[0]) == 0) {
            char* command_line = "stats";
            char* command_help = "Display verious system statistics";
            uint8_t warning = 0;
            uint16_t values = 0;
            uint8_t action = MODE_PD | MODE_CP;
            printhelp(command_line, command_help, values, warning, action);
            return;
        }

        if (strcasecmp("factory", argv[0]) == 0) {
            char* command_line = "FACTORY";
            char* command_help =
                "This command is in UPPER CASE. Sending this command will "
                "cause the device to be set back to factory \r\nsettings and "
                "then restarted.";
            uint16_t values = 0;
            uint8_t warning = 1;
            uint8_t action = MODE_PD;
            printhelp(command_line, command_help, values, warning, action);
            return;
        }

        if (strcasecmp("speed", argv[0]) == 0) {
            char* command_line = "speed [OSDP Speed]";
            char* command_help =
                "Sets or displays the speed of the OSDP interface. Valid "
                "speeds are 9600, 19200, 38400, 57600 or 115200.";
            uint16_t values = VALUE_OSDP_PORT_PD | VALUE_OSDP_ADDRESS;
            uint8_t warning = 1;
            uint8_t action = MODE_PD;
            printhelp(command_line, command_help, values, warning, action);
            return;
        }

        if (strcasecmp("route", argv[0]) == 0) {
            char* command_line = "route [Wiegand Port] [PD OSDP Port]";
            char* command_help =
                "Unlike most Wiegand to OSDP interfaces, this device lets you "
                "assign multiple Wiegand readers to a \r\n"
                "single OSDP port. There is no limit to the number of Wiegand "
                "devices that can be assigned to an OSDP port.";
            uint16_t values = VALUE_OSDP_PORT_PD | VALUE_WIEGAND_PORT;
            uint8_t warning = 0;
            uint8_t action = MODE_PD;
            printhelp(command_line, command_help, values, warning, action);
            return;
        }

        if (strcasecmp("led", argv[0]) == 0) {
            char* command_line = "led [mode]";
            char* command_help =
                "The on board LED has a number of modes. Mode 0 has the led "
                "changing state as the main loop executes. \r\n"
                "In mode 2 the LED toggles in response to OSDP transmissions. "
                "Mode 3 momentarily flashes the LED in line\r\n"
                "with Wiegand output.";
            uint16_t values = 0;
            uint8_t warning = 0;
            uint8_t action = MODE_PD | MODE_CP;
            printhelp(command_line, command_help, values, warning, action);
            return;
        }

        if (strcasecmp("cp", argv[0]) == 0) {
            char* command_line =
                "cp mode [state]\r\n"
                "cp osdp    [CP OSDP Port] ENABLE     [state]\r\n"
                "cp osdp    [CP OSDP PORT] ADDRESS    [OSDP ADDRESS]\r\n"
                "cp osdp    [CP OSDP PORT] WIEGAND    [WIEGAND PORT]\r\n"
                "cp osdp    [CP OSDP PORT] SECURE     [state]\r\n"
                "cp osdp    [CP OSDP PORT] INSTALL    [state]\r\n"
                "cp osdp    [CP OSDP PORT] ENCRYPTION [encryption]\r\n"
                "cp wiegand [WIEGAND PORT] LED        [HIGH|LOW] TEMP  [ON "
                "TIME] [OFF TIME] [COUNT] [ON COLOR] [OFF COLOR]\r\n"
                "cp wiegand [WIEGAND PORT] BUZ        [HIGH|LOW] TEMP  [ON "
                "TIME] [OFF TIME] [COUNT]\r\n"
                "cp wiegabd [WIEGAND PORT] LED        [HIGH|LOW] PERM  [ON "
                "TIME] [OFF TIME] [ON COLOR] [OFF COLOR]";
            char* command_help =
                "CP Commands. 'cp mode' turns CP mode ON or OFF. A restart is "
                "needed after this command. OSDP commands act on \r\n"
                "an OSDP connection. Wiegand commands work on a Wiegand port"
                "Setting the Wiegand \r\nPort to 0 will disable that mapping. ";
            uint16_t values = VALUE_OSDP_PORT_CP | VALUE_WIEGAND_PORT |
                              VALUE_TRUE | VALUE_ENCRYPTION |
                              VALUE_ON_OFF_TIME | VALUE_LED;
            uint8_t warning = 2;
            uint8_t action = MODE_CP;
            printhelp(command_line, command_help, values, warning, action);
            return;
        }
    }

    char* cmds[] = {"### System Commands",
                    "reset        - restart the device",
                    "FACTORY      - reset to factory settings",
                    " ",
                    "### Control Commands",
                    "door         - Unlock, Lock or Open a door",
                    "temp         - Temperature functions",
                    "power        - Turn circuits on and off",
                    "fan          - Control the cooling fans",
                    "info         - display all settings",
                    "stats        - display system statistics",
                    "help         - display help on individual commands",
                    "led          - change the meaning of the on board LED",
                    "speed        - adjust the comms speed",
                    "modbus       - Modbus serial functions",
                    " ",
                    "\0"};

    for (uint8_t i = 0; i < 32; i++) {
        hmiPuts(cmds[i], HMI_CLI);
    }
}
