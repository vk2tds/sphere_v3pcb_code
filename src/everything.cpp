#include "everything.h"
#include "defines.h"
#include "utility.h"
#include "hmi.h"
#include <ArduinoJson.h>

// Fan PWM for six ports
// Ran RPM for six ports
// Door Open/Close
// Power On/Off
// Read current (and voltage) - Raw and normalised
// RGB

// TEMP


// RPM - Interrupt Driven
// PWM - Timer
// Temperature - Library Polled
// Door - Event driven with timers

// In the future logging will be also done via MQTT




// TODO:
// State machine details for reading ADC
// Storing temperatures
// CLI
// More structure to the code
// Find the STM32F417 CPU - and define a CPU
// Define boards
// Modbus read

// Todo: Have trace only happen on specific CLI ports

// Need a queue of Modbus to poll




// Reporting Frequencies
// --------- -----------
// Modbus           As per definition
// Temperature      xxx
// Door Status      On Change; xxx
// Output Status    On Change; xxx
// Current          xxx
// Temperature      xxx
// LED              xxx
// Fan - PWM        On Change; xxx
// Fan - RPM        xxx
// A/C Output       On Change; xxx
// A/C Input        xxx


// When there is a virtual alarm, reporting frequencies change for a time

// Virtual Alarms
// ------- ------
// Over Current
// Under Current
// Fan RPM High or Low
// Temperature High or Low

// If T(2) > 45C, update reporting to every 5 seconds
// If Current(4) < 2A, update reporting to every 10 seconds for 60 seconds



uint32_t services = 0xFFFF; // What services are 'running'







// Serial just works???
HardwareSerial Serial2 (PA3, PA2); // (RX, TX)
HardwareSerial Serial2 (PA10, PA9); // RX, TX
HardwareSerial Serial4 (PC7, PC6); // RX, TX




// -------------------------------
// Switches and Current Monitoring

// CURRENT ADDRESS / VOLTAGE ADDRESS of 255 = NULL
struct Hardware h[] = {
    {voltage_12, 0, PA0, 3, INVALID_ADC_ADDRESS, "12V Switched Output  1"},
    {voltage_12, 1, PA1, 3, INVALID_ADC_ADDRESS, "12V Switched Output  2"},
    {voltage_12, 2, PA1, 3, INVALID_ADC_ADDRESS, "12V Switched Output  3"},
    {voltage_12, 3, PA1, 3, INVALID_ADC_ADDRESS, "12V Switched Output  4"},
    {voltage_12, 4, PA1, 3, INVALID_ADC_ADDRESS, "12V Switched Output  5"},
    {voltage_12, 5, PA1, 3, INVALID_ADC_ADDRESS, "12V Switched Output  6"},
    {voltage_12, 6, PA1, 3, INVALID_ADC_ADDRESS, "12V Switched Output  7"},
    {voltage_12, 7, PA1, 3, INVALID_ADC_ADDRESS, "12V Switched Output  8"},
    {voltage_12, 8, PA1, 3, INVALID_ADC_ADDRESS, "12V Switched Output  9"},
    {voltage_12, 9, PA1, 3, INVALID_ADC_ADDRESS, "12V Switched Output 10"},
    {voltage_48, 0, PA1, 3, INVALID_ADC_ADDRESS, "48V Switched Output 1"},
    {voltage_48, 1, PA1, 3, INVALID_ADC_ADDRESS, "48V Switched Output 2"},
    {voltage_48, 2, PA1, 3, INVALID_ADC_ADDRESS, "48V Switched Output 3"},
    {voltage_48, 3, PA1, 3, INVALID_ADC_ADDRESS, "48V Switched Output 4"},
    {voltage_48, 4, PA1, 3, INVALID_ADC_ADDRESS, "48V Switched Output 5"},
    {voltage_48, 5, PA1, 3, INVALID_ADC_ADDRESS, "48V Switched Output 6"},
    {voltage_fans, 0, PA1, 3, INVALID_ADC_ADDRESS, "Fan 12V Total Current Usage"}, // Fan current measurement
    {voltage_fans, 1, PA1, INVALID_ADC_ADDRESS, INVALID_ADC_ADDRESS, "Fan 1"}, // Fan 1
    {voltage_fans, 2, PA1, INVALID_ADC_ADDRESS, INVALID_ADC_ADDRESS, "Fan 2"}, // Fan 2
    {voltage_fans, 3, PA1, INVALID_ADC_ADDRESS, INVALID_ADC_ADDRESS, "Fan 3"}, // Fan 3
    {voltage_fans, 4, PA1, INVALID_ADC_ADDRESS, INVALID_ADC_ADDRESS, "Fan 4"}, // Fan 4
    {voltage_fans, 5, PA1, INVALID_ADC_ADDRESS, INVALID_ADC_ADDRESS, "Fan 5"}, // Fan 5
    {voltage_fans, 5, PA1, INVALID_ADC_ADDRESS, INVALID_ADC_ADDRESS, "Fan 6"}, // Fan 6
    {voltage_lights, 0, NO_PIN, 3, INVALID_ADC_ADDRESS, "Side Lights - Total Current"},
    {voltage_lights, 1, PA1, INVALID_ADC_ADDRESS, INVALID_ADC_ADDRESS, "Side Lights - Red"},
    {voltage_lights, 2, PA1, INVALID_ADC_ADDRESS, INVALID_ADC_ADDRESS, "Side Lights - Green"},
    {voltage_lights, 3, PA1, INVALID_ADC_ADDRESS, INVALID_ADC_ADDRESS, "Side Lights - Blue"},
    {voltage_supply, 0, PA1, 3, INVALID_ADC_ADDRESS, "48V Supply - Switched"},
    {voltage_supply, 1, PA1, 3, INVALID_ADC_ADDRESS, "48V Air Conditioner Supply - Switched"},
    {voltage_supply, 2, NO_PIN, 3, 3, "12V Supply - Total Current Usage"},
    {voltage_supply, 3, NO_PIN, 3, 3, "48V Supply - Total Current Usage"}
};

 uint8_t h_elementcount = sizeof (h) / sizeof (h[0]);


// ----------------
// Temperature Pins



struct TempPins temppins[MAX_TEMP_STRINGS]{
    {0, PD0},
    {1, PD1},
    {2, PD2},
    {3, PD3},
    {4, PD4},
    {5, PD5},
    {6, PD10},
    {7, PD11}
};

// --------------------
// Fan PWM and RPM Pins



// PWM is also set by timer
// NOTE: PWM is partially hard coded, just because of how things work
// RPM is based on interrupts, so this can be a bit complex too. 
struct FanPorts fanpins[MAX_FAN_PWM_RPM]{
    {0, PC8, PC0}, // TIM3-3
    {1, PC9, PE10}, // TIM3-4
    {2, PD12, PC2}, // TIM4-1
    {3, PD13, PE3}, // TIM4-2
    {4, PD14, PA4}, // TIM4-3
    {5, PD15, PA5} // TIM4-4
};



struct DoorPins doorpins[MAX_DOORS]{
  {0, false, NULL, NULL}
};


// NOTE: NonBlockingModbusMaster is CUSTOM
NonBlockingModbusMaster nullmodbus; // dont really want this but the easiest solution to storing a null reference to a class. 
NullStream nullstream;

struct PortInformation portinformation[5] = {
  {portfunction_cli, "", 0, 0, 0, NULL, NULL, "", 0, nullstream, nullstream, 115200, SERIAL_8N1, false},           // port_USB
  {portfunction_serialMQTT, "", 0, 0, 0, NULL, NULL, "", 0, nullstream, nullstream, 115200, SERIAL_8N1, false},    // portfunction_serialMQTT
  {portfunction_cli, "", 0, 0, 0, NULL, NULL, "", 0, nullstream, nullstream, 115200, SERIAL_8N1, false},           // port_ethernet_telnet
  {portfunction_serialAC, "", 0, 0, 0, NULL, NULL, "", 0, nullstream, nullstream, 600, SERIAL_8N1, false},      // port_serial_ac
  {portfunction_serialMODBUS, "", 0, 0, 0, NULL, NULL, "", 0, nullstream, nullstream, 115200, SERIAL_8E1, false}   // port_serial_rs485

};

uint8_t portinformation_elementcount = sizeof (portinformation) / sizeof (portinformation[0]);


struct ModbusScan modbusscan[MAX_MODBUS];

struct AirComms aircomms; 




// These are the default reporting frequencies. They can be overridden for various reasons by setting 
// the propery .next_report to the required settingsSinceStart. One reason would be external stimulus causing 
// for instance an output to change state

struct MqttReporting mqttreporting [] = {
  {MQTT_REP_SOFTHARDWARE, "/board", 900, 5},
  {MQTT_REP_TEMPERATURE, "/temperature", 15, 15},
  {MQTT_REP_ADC, "/analog", 60, 15},
  {MQTT_REP_FAN, "/fan/", 30, 60},
  {MQTT_REP_OUTPUTS, "/outputs", 120, 60},
  {MQTT_REP_LOCK, "/lock", 0, 0},
  {MQTT_REP_AIRCOND, "/aircond", 20, 15},
  {MQTT_REP_LED, "/led", 0,0},
  {MQTT_REP_SETTINGS, "/settings", 0,0},
  {MQTT_REP_MODBUS, "/modbus", 0,0}                 // Modbus reports as per definition
};

uint8_t mqttreporting_elementcount = sizeof (mqttreporting) / sizeof (mqttreporting[0]);



struct MqttToSend mqtttosend[MQTT_UNIQUE_MAX];




// CONSTRUCTORS

HardwareTimer *stmFanTimer_TIM3 = new HardwareTimer(TIM3); // PWM
HardwareTimer *stmFanTimer_TIM4 = new HardwareTimer(TIM4); // PWM


OneWire oneWire_A(temppins[0].temp_pin);
DallasTemperature dallasTemp_A(&oneWire_A);
NonBlockingDallas temperatureSensors_A(&dallasTemp_A);

OneWire oneWire_B(temppins[1].temp_pin);
DallasTemperature dallasTemp_B(&oneWire_B);
NonBlockingDallas temperatureSensors_B(&dallasTemp_B);

OneWire oneWire_C(temppins[2].temp_pin);
DallasTemperature dallasTemp_C(&oneWire_C);
NonBlockingDallas temperatureSensors_C(&dallasTemp_C);

OneWire oneWire_D(temppins[3].temp_pin);
DallasTemperature dallasTemp_D(&oneWire_D);
NonBlockingDallas temperatureSensors_D(&dallasTemp_D);

OneWire oneWire_E(temppins[4].temp_pin);
DallasTemperature dallasTemp_E(&oneWire_E);
NonBlockingDallas temperatureSensors_E(&dallasTemp_E);

OneWire oneWire_F(temppins[5].temp_pin);
DallasTemperature dallasTemp_F(&oneWire_F);
NonBlockingDallas temperatureSensors_F(&dallasTemp_F);

OneWire oneWire_G(temppins[6].temp_pin);
DallasTemperature dallasTemp_G(&oneWire_G);
NonBlockingDallas temperatureSensors_G(&dallasTemp_G);

OneWire oneWire_H(temppins[7].temp_pin);
DallasTemperature dallasTemp_H(&oneWire_H);
NonBlockingDallas temperatureSensors_H(&dallasTemp_H);




TwoWire Wire1 (PB7, PB6);
TwoWire Wire2 (PB11, PB10); 

ADS1115* ADS[MAX_ADC];








// -------------
// Value Storage
// -------------

// fan_pwm_rpm[]
// tempstorage[]
// adcstorage[] - Current and Voltage
// doorstorage[]
// outputs in h above
// A/C?????






struct FanPWMRPM fan_pwm_rpm[MAX_FAN_PWM_RPM];






// ----
// Time
// ----

uint32_t secondsSinceStart = 0;


// ---
// ADC
// ---

// Index is by hardware address


struct ADCstorage adcstorage [MAX_ADC];


// -----------
// Temperature
// -----------

// NOTE: Assuming single sensor per string


struct TEMPstorage tempstorage [MAX_TEMP_STRINGS];


// ----
// Door
// ----


struct DOORstorage doorstorage [MAX_DOORS];


// ----------
// Fans - RPM
// ----------
//
// These are interrupt handlers

void fansRpmInt_A (void)
{
  fan_pwm_rpm[0].rpmcount ++;
}

void fansRpmInt_B (void)
{
  fan_pwm_rpm[1].rpmcount ++;
}

void fansRpmInt_C (void)
{
  fan_pwm_rpm[2].rpmcount ++;
}

void fansRpmInt_D (void)
{
  fan_pwm_rpm[3].rpmcount ++;
}

void fansRpmInt_E (void)
{
  fan_pwm_rpm[4].rpmcount ++;
}

void fansRpmInt_F (void)
{
  fan_pwm_rpm[5].rpmcount ++;
}

void setup_RPM(void)
{
  pinMode(fanpins[0].rpm_pin, INPUT_PULLUP);
  pinMode(fanpins[1].rpm_pin, INPUT_PULLUP);
  pinMode(fanpins[2].rpm_pin, INPUT_PULLUP);
  pinMode(fanpins[3].rpm_pin, INPUT_PULLUP);
  pinMode(fanpins[4].rpm_pin, INPUT_PULLUP);
  pinMode(fanpins[5].rpm_pin, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(fanpins[0].rpm_pin), fansRpmInt_A, CHANGE);
  attachInterrupt(digitalPinToInterrupt(fanpins[1].rpm_pin), fansRpmInt_B, CHANGE);
  attachInterrupt(digitalPinToInterrupt(fanpins[2].rpm_pin), fansRpmInt_C, CHANGE);
  attachInterrupt(digitalPinToInterrupt(fanpins[3].rpm_pin), fansRpmInt_D, CHANGE);
  attachInterrupt(digitalPinToInterrupt(fanpins[4].rpm_pin), fansRpmInt_E, CHANGE);
  attachInterrupt(digitalPinToInterrupt(fanpins[5].rpm_pin), fansRpmInt_F, CHANGE);
}


void sample_RPM(void)
{

  static uint32_t lasttime;
  uint32_t now;
  now = millis();
  if (now < lasttime){ // Happens every 49 days or so
    lasttime = now;
    return;
  }
  uint32_t timediff = now - lasttime;
  lasttime = now;

  for (uint8_t i=0; i < 6; i++){
    if (fan_pwm_rpm[i].rpmcount < fan_pwm_rpm[i].lastrpmcount){
        fan_pwm_rpm[i].lastrpmcount = fan_pwm_rpm[i].rpmcount;
      break;
    }
    fan_pwm_rpm[i].timediff = timediff;
    fan_pwm_rpm[i].rpmcount = fan_pwm_rpm[i].rpmcount - fan_pwm_rpm[i].lastrpmcount;
  }
}


// -------------------------------
// MQTT
// -------------------------------
// These functions send MQTT, generally when things change, but also on startup.
// They can send MQTT to any connected MQTT devices, but also possibly out the
// USB port if in trace mode ETC. 

// MQTT
// sendMQTT (sentence, data)
// Need MQTT Queue


void queueMQTT(char sentence[], char data[])
{
  uint8_t found = 255;
  uint32_t mintime = 0xFFFF;
  for (uint8_t i=0; i<MQTT_UNIQUE_MAX; i++){
    if (mqtttosend[i].secondsSinceStart != 0){
      if (mqtttosend[i].secondsSinceStart < mintime){
        mintime = mqtttosend[i].secondsSinceStart;
        found = i;
      }
    }
  }

  if (found != 255){
    // we have found something to send
    
    // *****************************
    // ******* SEND THIS ONE *******
    // *****************************

    // Blank things out once sent
    mqtttosend[found].secondsSinceStart = 0;
    mqtttosend[found].sentence[0] = 0;
  }
} 

void sendMqtt (char sentence[], char data[])
{
  // -----------------------------
  // SEND THE MQTT TO THE QUEUE... 
  // -----------------------------

  // Send it as a trace, so that any CLI who needs to see it gets it
  // At the moment TRACE sends everywhere on CLI
  char buf[512];
  snprintf (buf, 512, "%s|%s", sentence, data);
  hmiPuts (buf, HMI_TRACE);

  queueMQTT (sentence, data);
}

void sendMqtt (uint8_t unique_identifier, char sentence[], JsonDocument doc)
{
  // -----------------------------
  // SEND THE MQTT TO THE QUEUE... 
  // -----------------------------

  // Only add to the queue if there is no unique_identifier in the queue already. 

  // Question: Can we have an array of pointers to the json etc. That way we dont need to queue
  // using a queue object but do it using something more locked down. Queue cannot get too big. Latest information
  // is stored in the structure...


  // Send it as a trace, so that any CLI who needs to see it gets it
  // At the moment TRACE sends everywhere on CLI
  char buf[512];
  //snprintf (buf, 512, "%s|%s", sentence, data);
  hmiPuts (buf, HMI_TRACE);



  // Add the JSON to the list of JSON to send. If there is already a JSON for the [unique_identifier]
  // then throw it out and replace it with an updated JDOC. What this means is that only the latest 
  // JSON of any identifier will be sent, but the oldest waiting JDOC will be sent first. 

  mqtttosend[unique_identifier].jdoc = doc;
  if (mqtttosend[unique_identifier].secondsSinceStart != 0){
    mqtttosend[unique_identifier].secondsSinceStart = secondsSinceStart;
  }
  safe_strcat ((char *)mqtttosend[unique_identifier].sentence, sentence, 96);

  //queueMQTT (sentence, data);
}

uint8_t getMqttReportingIndex (uint8_t sensor)
{
    uint8_t sensor_index = 0xFF;
    for (uint8_t i = 0; i < mqttreporting_elementcount; i++){
      if (mqttreporting[i].sensor == sensor){
        sensor_index = i;
      }
    }
    return sensor_index;
}

// https://arduinojson.org/v7/tutorial/serialization/

void sendMqttModbus(uint8_t modbusscanindex)
{
    char buf_sentence[128];
    ModbusScan &ms = modbusscan[modbusscanindex];

    uint8_t sensor_index = getMqttReportingIndex(MQTT_REP_MODBUS);

    if (sensor_index == 0xFF){
      // Error... Oops
      return;
    }

    // /modbus/[modbus_instance]/[modbus_address]/[register_type]/[start_address]/[value_count]
    // /modbus/0/1/3/2301/16
    snprintf (buf_sentence, 128, "%s/%02d/%02d/%02x/%05d/%04d", mqttreporting[sensor_index],
      ms.modbus_instance, ms.modbus_address, ms.type, ms.start_address, ms.values);

    JsonDocument doc;

    doc["time"] = secondsSinceStart;
    doc["sample_time"] = ms.secondsSinceStart;

    uint16_t val;
    for (uint8_t j = 0; j < ms.values; j++){
      switch (ms.type){
        case MODBUS_COILS:
        case MODBUS_DISCRETE_INPUTS:
            uint16_t temp = ms.data[j>>4];
            uint8_t c;
            if (temp & (1 << 0x0F)){
              val = 1;
            } else {
              val = 0;
            }
            break;
        case MODBUS_HOLDING_REGISTERS:
        case MODBUS_INPUT_REGISTERS:
            val = ms.start_address+j, ms.data[j];
            break;
        default:
            break;
        doc["data"][j] = val;
      }
    }
    sendMqtt(MQTT_UNIQUE_MODBUS ,buf_sentence, doc);
}



void sendMqttADCorOutputs (bool outputs)
{
  // If outputs == true, this is for output status
  // if outputs == false, this is for ADC values

  char buf_sentence[128];

  char output_string[16];

  uint8_t sensor_index = getMqttReportingIndex(MQTT_REP_ADC);

  if (sensor_index == 0xFF){
    // Error... Oops
    return;
  }

  if (outputs){
    snprintf (output_string, 16, "outputs");
  } else {
    snprintf (output_string, 16, "analog");
  };


  for (uint8_t j=0; j<32; j++){
    JsonDocument doc;
    doc["time"] = secondsSinceStart;
    if (outputs){
      doc["sample_time"] = secondsSinceStart;
    }

    switch (j){
      case voltage_12:
        snprintf (buf_sentence, 128, "%s%s/12v", mqttreporting[sensor_index], output_string);
        break;
      case voltage_48:
        snprintf (buf_sentence, 128, "%s%s/48v", mqttreporting[sensor_index], output_string);
        break;
      case voltage_fans:
        snprintf (buf_sentence, 128, "%s%s/fans", mqttreporting[sensor_index], output_string);
        break;
      case voltage_lights:
        snprintf (buf_sentence, 128, "%s%s/lights", mqttreporting[sensor_index], output_string);
        break;
      case voltage_supply:          
        snprintf (buf_sentence, 128, "%s%s/supply", mqttreporting[sensor_index], output_string);
        break;
      default:
    }

    if (strlen(buf_sentence) > 0){
      if (outputs){
        JsonDocument doc;
        doc["time"] = secondsSinceStart;
        doc["sample_time"] = secondsSinceStart;

        for (uint8_t i=0; i<h_elementcount; i++){
          struct Hardware &h2 = h[i];
          bool state;
          if (digitalRead (h2.power_pin)){
            state == true;
          }
          doc["data"][i] = state;
        }
        sendMqtt (MQTT_UNIQUE_OUTPUTS_ALL, buf_sentence, doc);
      } else {
        JsonDocument doc;
        doc["time"] = secondsSinceStart;

        for (uint8_t i=0; i<h_elementcount; i++){
          struct Hardware &h2 = h[i];
          if (h2.mode == j){
            if (h2.current_adc_address != INVALID_ADC_ADDRESS){
              doc["data"]["amps"][j] = adcstorage[h2.current_adc_address].adc;
              doc["data"]["amps"][j]["sample_time"] = adcstorage[h2.current_adc_address].secondsSinceStart;
            }
            if (h2.voltage_adc_address != INVALID_ADC_ADDRESS){
              doc["data"]["volts"][j] = adcstorage[h2.voltage_adc_address].adc;
              doc["data"]["volts"][j]["sample_time"] = adcstorage[h2.voltage_adc_address].secondsSinceStart;
            }
          }
        }
        sendMqtt (MQTT_UNIQUE_ADC, buf_sentence, doc);
      }
    }
  }
}


void sendMqttTemp (void)
{


    char buf_sentence[128];

    uint8_t sensor_index = getMqttReportingIndex(MQTT_REP_TEMPERATURE);

    if (sensor_index == 0xFF){
      // Error... Oops
      return;
    }

    JsonDocument doc;
    doc["time"] = secondsSinceStart;

    snprintf (buf_sentence, 128, "%s", mqttreporting[sensor_index]);


    //struct TEMPstorage tempstorage [MAX_TEMP_STRINGS];

    for (uint8_t i = 0; i<MAX_TEMP_STRINGS; i++){
      doc["data"][i] = tempstorage[i].temperature;
      doc["data"][i]['sample_time'] = tempstorage[i].secondsSinceStart;
    }
    sendMqtt (MQTT_UNIQUE_TEMPERATRE, buf_sentence, doc);
}


void sendMqttFan (void)
{


    char buf_sentence[128];

    uint8_t sensor_index = getMqttReportingIndex(MQTT_REP_FAN);

    if (sensor_index == 0xFF){
      // Error... Oops
      return;
    }

    JsonDocument doc;
    doc["time"] = secondsSinceStart;

    snprintf (buf_sentence, 128, "%s", mqttreporting[sensor_index]);


    //struct TEMPstorage tempstorage [MAX_TEMP_STRINGS];

    for (uint8_t i = 0; i < MAX_FAN_PWM_RPM; i++){
      doc["data"][i]["pwm"] = fan_pwm_rpm[i].pwm;
      doc["data"][i]["rpm"]["timediff"] = fan_pwm_rpm[i].timediff;
      doc["data"][i]["rpm"]["rpmcount"] = fan_pwm_rpm[i].rpmcount;
    }
    sendMqtt (MQTT_UNIQUE_FAN_ALL, buf_sentence, doc);
}

void sendMqttAc (void)
{

    char buf_sentence[128];

    uint8_t sensor_index = getMqttReportingIndex(MQTT_REP_AIRCOND);

    if (sensor_index == 0xFF){
      // Error... Oops
      return;
    }

    JsonDocument doc;
    doc["time"] = secondsSinceStart;
    doc["sample_time"] = aircomms.rx_secondssincestart;

    snprintf (buf_sentence, 128, "%s", mqttreporting[sensor_index]);

    doc["data"]["status"]["compressor"]["amps"] = aircomms.rx_compressor_currernt;
    doc["data"]["status"]["compressor"]["speed"] = aircomms.rx_compressor_speed;
    doc["data"]["status"]["bus"]["voltage"] = aircomms.rx_busbar_voltage;
    doc["data"]["status"]["live"] = aircomms.rx_status_now;
    doc["data"]["status"]["historical"] = aircomms.rx_status_historical;
    doc["data"]["power"] = aircomms.tx_power;
    doc["data"]["speed"] = aircomms.tx_speed;

    sendMqtt (MQTT_UNIQUE_AC, buf_sentence, doc);
}


void sendMqttSoftHardWare (void)
{

    char buf_sentence[128];

    uint8_t sensor_index = getMqttReportingIndex(MQTT_REP_SOFTHARDWARE);

    if (sensor_index == 0xFF){
      // Error... Oops
      return;
    }

    JsonDocument doc;
    doc["time"] = secondsSinceStart;

    snprintf (buf_sentence, 128, "%s", mqttreporting[sensor_index]);

    doc["software"]["copyright"] = "COPYRIGHT";
    doc["software"]["product"] = "PROJECT_NAME";
    doc["software"]["version"] = "PROJECT_VERSION";
    doc["software"]["date"] = "COPYRIGHT";
    doc["software"]["compile"] = "LAST_BUILD_TIME";


    doc["hardware"]["processor"] = "STM32F417";
    doc["hardware"]["MAC"] = "";
    doc["hardware"]["serial"] = "";
    doc["hardware"]["product"] = "HUB Motherboard V3";
    doc["hardware"]["version"] = "0.1";



    sendMqtt (MQTT_UNIQUE_SOFTHARDWARE, buf_sentence, doc);
}

void mqtt_door (uint8_t door_number, bool state){
  // TODO: Send MQTT
}





// ----------
// Fans - PWM
// ----------


// Interface
void setFanPWM(uint8_t fan, uint8_t percent)
{
  if (services & SERVICE_PWM){
    switch (fan){
      case 0:
        stmFanTimer_TIM3->setCaptureCompare(3, percent, PERCENT_COMPARE_FORMAT); 
        break;
      case 1:
        stmFanTimer_TIM3->setCaptureCompare(4, percent, PERCENT_COMPARE_FORMAT); 
        break;
      case 2:
        stmFanTimer_TIM4->setCaptureCompare(1, percent, PERCENT_COMPARE_FORMAT); 
        break;
      case 3:
        stmFanTimer_TIM4->setCaptureCompare(2, percent, PERCENT_COMPARE_FORMAT); 
        break;
      case 4:
        stmFanTimer_TIM4->setCaptureCompare(3, percent, PERCENT_COMPARE_FORMAT); 
        break;
      case 5:
        stmFanTimer_TIM4->setCaptureCompare(4, percent, PERCENT_COMPARE_FORMAT); 
        break;
    }
  }
}

// Interface
void setFanPWMs(void)
{
  for (uint8_t i = 0; i < MAX_FAN_PWM_RPM; i++){
    setFanPWM(i, fan_pwm_rpm[i].pwm);
  }  
}


void setup_PWM (void)
{

  // Note: Timers are weird on the STM32. Some timer pins are on 'Alternate' outputs. These need to be
  // explicity highlightes as being ALTERNATE pins, else things will not work as anticipated. 

  // ToDo: Make sure that pins are correct

  stmFanTimer_TIM3->pause();
  stmFanTimer_TIM4->pause();

  stmFanTimer_TIM3->setOverflow(25000, HERTZ_FORMAT);
  stmFanTimer_TIM4->setOverflow(25000, HERTZ_FORMAT);

  stmFanTimer_TIM3->setMode(3, TIMER_OUTPUT_COMPARE_PWM1, fanpins[0].pwm_pin); // TODO: Check ALT if needed
  stmFanTimer_TIM3->setMode(4, TIMER_OUTPUT_COMPARE_PWM1, fanpins[1].pwm_pin);

  stmFanTimer_TIM4->setMode(1, TIMER_OUTPUT_COMPARE_PWM1, fanpins[2].pwm_pin);

  // stmFanTimer_TIM3->setMode(2, TIMER_OUTPUT_COMPARE_PWM1, PA7_ALT1);
  stmFanTimer_TIM4->setMode(2, TIMER_OUTPUT_COMPARE_PWM1, fanpins[3].pwm_pin | ALT1); // Alternate timer needs a different pin name. WTF

  stmFanTimer_TIM4->setMode(3, TIMER_OUTPUT_COMPARE_PWM1, fanpins[4].pwm_pin | ALT1);
  stmFanTimer_TIM4->setMode(4, TIMER_OUTPUT_COMPARE_PWM1, fanpins[5].pwm_pin | ALT1);

  stmFanTimer_TIM3->resume();
  stmFanTimer_TIM4->resume();

  for (uint8_t i = 0; i < MAX_FAN_PWM_RPM; i++){
    fan_pwm_rpm[i].pwm = 0;
  }

  setFanPWMs();

}


// -----------
// Temperature


boolean tempsStale[6];

// ToDo: This needs to be improved. If there are two sensors per input, it will only show one.
void handleIntervalElapsed(int port, int deviceIndex, float temperature, String address)
{
  if (tempsStale[port]){
    tempsStale[port] = false;
    char topic[64];
    char addr[32];

    address.toCharArray(addr, address.length()+1);

    // do soemthing with temperature

  }
}


// Individulal callbacks work best

void handleIntervalElapsed_A(int deviceIndex, int32_t temperatureRAW)
{
  handleIntervalElapsed(0, deviceIndex, temperatureSensors_A.rawToCelsius(temperatureRAW), temperatureSensors_A.getAddressString(deviceIndex));  
}

void handleIntervalElapsed_B(int deviceIndex, int32_t temperatureRAW)
{
  handleIntervalElapsed(1, deviceIndex, temperatureSensors_A.rawToCelsius(temperatureRAW), temperatureSensors_B.getAddressString(deviceIndex));  
}

void handleIntervalElapsed_C(int deviceIndex, int32_t temperatureRAW)
{
  handleIntervalElapsed(2, deviceIndex, temperatureSensors_A.rawToCelsius(temperatureRAW), temperatureSensors_C.getAddressString(deviceIndex));  
}

void handleIntervalElapsed_D(int deviceIndex, int32_t temperatureRAW)
{
  handleIntervalElapsed(3, deviceIndex, temperatureSensors_A.rawToCelsius(temperatureRAW), temperatureSensors_D.getAddressString(deviceIndex));  
}

void handleIntervalElapsed_E(int deviceIndex, int32_t temperatureRAW)
{
  handleIntervalElapsed(4, deviceIndex, temperatureSensors_A.rawToCelsius(temperatureRAW), temperatureSensors_E.getAddressString(deviceIndex));  
}

void handleIntervalElapsed_F(int deviceIndex, int32_t temperatureRAW)
{
  handleIntervalElapsed(5, deviceIndex, temperatureSensors_A.rawToCelsius(temperatureRAW), temperatureSensors_F.getAddressString(deviceIndex));  
}

void handleDeviceDisconnected(int deviceIndex)
{
}

void setup_temperature (void)
{
    for (uint8_t i = 0; i < 6; i++){
    tempsStale[i] = false;
  }


  temperatureSensors_A.begin(NonBlockingDallas::resolution_12, 1500);
  temperatureSensors_A.onIntervalElapsed(handleIntervalElapsed_A);
  IWatchdog.reload();

  temperatureSensors_B.begin(NonBlockingDallas::resolution_12, 1600);
  temperatureSensors_B.onIntervalElapsed(handleIntervalElapsed_B);
  IWatchdog.reload();

  temperatureSensors_C.begin(NonBlockingDallas::resolution_12, 1700);
  temperatureSensors_C.onIntervalElapsed(handleIntervalElapsed_C);
  IWatchdog.reload();

  temperatureSensors_D.begin(NonBlockingDallas::resolution_12, 1800);
  temperatureSensors_D.onIntervalElapsed(handleIntervalElapsed_D);
  IWatchdog.reload();

  temperatureSensors_E.begin(NonBlockingDallas::resolution_12, 1900);
  temperatureSensors_E.onIntervalElapsed(handleIntervalElapsed_E);
  IWatchdog.reload();

  temperatureSensors_F.begin(NonBlockingDallas::resolution_12, 1400);
  temperatureSensors_F.onIntervalElapsed(handleIntervalElapsed_F);
  IWatchdog.reload();

}


void temperature_loop (void)
{
  temperatureSensors_A.update();
  temperatureSensors_B.update();
  temperatureSensors_C.update();
  temperatureSensors_D.update();
  temperatureSensors_E.update();
  temperatureSensors_F.update();
  temperatureSensors_G.update();
  temperatureSensors_H.update();
}




// ----
// DOOR

Atm_led door_pin_a;
Atm_led door_pin_b;
Atm_step door_step;


void door (uint8_t door_number, uint8_t dooraction)
{
  // This code uses the AUTOMATON state machien
  // https://github.com/tinkerspy/Automaton/tree/master
  //
  // 
  // Operations are done using state machines internally.  
  // State = 0 = Lock. 1 = Unlock 2 = Future Open

  bool state;

  if (dooraction == 0){
    state = false;
  } else {
    state= true;
  }

  if (doorstorage[door_number].DoorState != state){
    mqtt_door (door_number, state);
  }


  doorstorage[door_number].DoorState = state;


  if (door_number < MAX_DOORS){
    if (doorpins[door_number].h_bridge == false){
      // This use case is for when we have one relay controlling power to the locks, and the other
      // controlling direction (Lock or Unlock)
      //
      // Set direction -> Turn power on -> turn off -> unset direction
      // time pin_a pin_b
      // 0    False False - Neutral State just in case
      // 500  False DIR   - Set direction
      // 600  True  DIR   - Turn power on
      // 1600 False DIR   - Turn power off
      // 1700 False False - Neutral State
      if (services & SERVICE_DOOR){
        door_pin_a.begin (doorpins[door_number].pin_a)
          .lead (600)
          .on()
          .blink (1000)
          .off();
      }
      if (state){
        if (services & SERVICE_DOOR){
          // Only needed if we are chanting the state of pin B
          door_pin_b.begin (doorpins[door_number].pin_b)
          .lead(500)
          .blink (1200)
          .off();
        }
      }
    }
    if (doorpins[door_number].h_bridge == true){
      // This is a H-Bridge use case. In steady state, both relays
      // are off. To unlock, one realy is turned on for a short time
      // and then it is turned off.

      // turn off both outputs for 500 mSec
      // Turn on the correct output for 1000 mSec
      // Turn off both outputs.

      if (state == true){
        if (services & SERVICE_DOOR){
          door_pin_a.begin (doorpins[door_number].pin_a)
            .off()
            .pause(500)
            .blink(1000)
            .off();
          door_pin_b.begin (doorpins[door_number].pin_b)
            .off();
        }
      }
      if (state == true){
        if (services & SERVICE_DOOR){
          door_pin_a.begin (doorpins[door_number].pin_a)
            .off();
          door_pin_b.begin (doorpins[door_number].pin_b)
            .off()
            .pause(500)
            .blink(1000)
            .off();
        }  
      }
    }
  }
}




// ---------------------------
// Current and Voltage Reading

Atm_timer adc_loop_timer;
Atm_timer adc_step_timer;
uint8_t adc_index = 0;

// To read current or voltage
// - Set the I2C Multiplexor
// - Request conversion via I2C
// - Read the value
//
//https://github.com/tinkerspy/Automaton/tree/master

 


void adc_callback_state_2_set_adc_port_and_read (int idx, int v, int up)
{



  // ToDo: add ADC
  adcstorage[adc_index].adc = ADS[1]->getValue();
  adcstorage[adc_index].secondsSinceStart = secondsSinceStart;

  adc_index ++;
  adc_index = adc_index & 0x1F;


}


void adc_callback_state_1_set_multiplexor (int idx, int v, int up)
{
  // Set Multiplexor
  uint8_t multiplexor = adc_index >> 2;

  // ToDo: Read ADC
  ADS[adc_index]->requestADC(adc_index & 0x03); // Throw away the value. This just sets the port

  adc_step_timer.begin(20).onTimer(adc_callback_state_2_set_adc_port_and_read).start();
}

void adc_callback (int idx, int v, int up)
{
  // Called about 32 times per second.

  if (adc_index >= MAX_ADC){ 
    adc_index = 0;
  };

  adc_step_timer.begin(0).onTimer(adc_callback_state_1_set_multiplexor).start();


}

void init_adc (uint8_t bus, uint8_t device)
{
  uint8_t index = (bus*4)+device;
  if (bus == 0){
    ADS[index] = new ADS1115 (0x048 + device, &Wire1);
  } else {
    ADS[index] = new ADS1115 (0x048 + device, &Wire2);
  }

  ADS[index]->begin();
  ADS[index]->setGain(0);         //  0 == 6.144 volt, default
  ADS[index]->setDataRate(7);     //  0 = slow   4 = medium   7 = fast
  ADS[index]->setMode(0);         //  0 == continuous mode
  ADS[index]->requestADC(0); //  0 == default channel,  trigger first read

}



void setup_adc (void)
{

  // Init ADCs
  for (uint8_t i=0; i < 4; i++){
    init_adc(0, i);
    init_adc(1, i);
  }





  adc_loop_timer.begin (30) // 30 mSec is about 1000 mSec / 32
    .onTimer (adc_callback)
    .start();
}




uint8_t findNextModbusIndex(uint8_t modbus_instance)
{
  // Find the index for the next scan. So, go through the list. Find the first enytry where
  // the next_scan_time is <= seconds since restart. If there are two that have the sane
  // time, the priority is the one with the greatest time between scans. This is beause 
  // this is least likely to be run, and more likely to be pushed to the bottom of the list. 
  uint8_t found_index = 255; // NULL
  for (uint8_t i = 0; i < MAX_MODBUS; i++){
    if (modbusscan[i].modbus_instance == modbus_instance){
      if (modbusscan[i].next_scan_time <= secondsSinceStart){
        // This rule needs to run...
        if (found_index != 255){
          // And there is a previous found entry
          if (modbusscan[i].scan_frequency > modbusscan[found_index].scan_frequency){
            // And the scan frequency of this entry is bigger than the last found one
            found_index = i;    
          }
        } else {
          found_index = i;
        }
      }    
    }
  }
  return found_index;
}



// Set PTT
// Read {Holding} registers
//
//
// Loop
//  .processdata()
//  if .IsTransmit == False then
//    if Buffer is Empy 
//      Schedule PTT == False



Atm_led modbus_machine;


struct ModbusInstance modbusinstance[MAX_MODBUSINSTANCES];




void processModbusData( NonBlockingModbusMaster &mb)
{

  uint8_t err = mb.getError(); // 0 for OK
  if (err){
    // Do Something
    return;
  }


  for (uint8_t i=0; i < MAX_MODBUSINSTANCES; i++){
    if (&modbusinstance[i].modbusinstance == &mb){
      struct ModbusScan &ms = modbusscan[modbusinstance[i].modbusscan_index];
      // Store the following data
      // ms.modbus_address;
      // ms.start_address;
      // ms.values;
      // ms.type;
      // secondsSinceStart;

      for (uint8_t size=0; size<mb.getResponseBufferLength(); size++){
        ms.data[size] = mb.getResponseBuffer(size);
      }
      ms.secondsSinceStart = secondsSinceStart; 
  
      sendMqttModbus (modbusinstance[i].modbusscan_index);

    }
  }


}


// NOTE: 3.5 character times after checksum finishes

void pollModbus (uint8_t modbus_instance)
{
  uint8_t next = findNextModbusIndex(modbus_instance);
  if (next != 255){
    // Therefore we have an entry to deal with...

    //if (nbModbusMaster.readHoldingRegisters(slaveId, address, qty)) {

    switch (modbusscan[next].type){
      case MODBUS_COILS:
        modbusinstance[modbus_instance].transmit = true;
        modbusinstance[modbus_instance].modbusscan_index = next;
        modbusinstance[modbus_instance].modbusinstance.readCoils(modbusscan[next].modbus_address, modbusscan[next].start_address, modbusscan[next].values, processModbusData);
        break;
      case MODBUS_DISCRETE_INPUTS:
        modbusinstance[modbus_instance].transmit = true;
        modbusinstance[modbus_instance].modbusscan_index = next;
        modbusinstance[modbus_instance].modbusinstance.readDiscreteInputs(modbusscan[next].modbus_address, modbusscan[next].start_address, modbusscan[next].values, processModbusData);
        break;
      case MODBUS_HOLDING_REGISTERS:
        modbusinstance[modbus_instance].transmit = true;
        modbusinstance[modbus_instance].modbusscan_index = next;
        modbusinstance[modbus_instance].modbusinstance.readHoldingRegisters(modbusscan[next].modbus_address, modbusscan[next].start_address, modbusscan[next].values, processModbusData);
        break;
      case MODBUS_INPUT_REGISTERS:
        modbusinstance[modbus_instance].transmit = true;
        modbusinstance[modbus_instance].modbusscan_index = next;
        modbusinstance[modbus_instance].modbusinstance.readInputRegisters(modbusscan[next].modbus_address, modbusscan[next].start_address, modbusscan[next].values, processModbusData);
        break;
    }

    modbusscan[next].next_scan_time = secondsSinceStart + modbusscan[next].scan_frequency;
  }
}




void setup_modbus(void)
{

    modbusinstance[0].ptt_pin = NONE_PIN; //ToDo


    // https://s.campbellsci.com/documents/us/manuals/climavue40.pdf
    modbusscan[0] = {0, 1, MODBUS_INPUT_REGISTERS, 3001, 43, 60, 1}; // Scan weather station - Cumulative
    modbusscan[1] = {0, 1, MODBUS_INPUT_REGISTERS, 3202, 43, 6, 1}; // Scan weatehr station - Live
    modbusscan[2] = {0, 1, MODBUS_INPUT_REGISTERS, 3401, 24, 900, 1}; // Scan weather - Serial Numbers

    for (uint8_t i = 0; i < portinformation_elementcount; i++){
      if (portinformation[i].portfunction == portfunction_serialMODBUS){
        modbusinstance[0].modbusinstance = portinformation[i].modbus;

      }
    }
}






struct NullStream : public Stream{
  NullStream( void ) { return; }
  int available( void ) { return 0; }
  void flush( void ) { return; }
  int peek( void ) { return -1; }
  int read( void ){ return -1; }
  size_t write( uint8_t u_Data ){ return u_Data, 0x01; }
};




// Note... Stream data structure will not set baud rate
void setup_ports(void){
  for (uint8_t i = 0; i < portinformation_elementcount; i++){
    // i = enum of Port. 
    //     port_USB, portfunction_serialMQTT, port_ethernet_telnet, port_serial_ac, port_serial_rs485
    switch (i){
      // Standard Serial Ports
      case port_USB:               
        SerialUSB.begin();
        portinformation[i].s = SerialUSB;
        break;
      case port_ESP32:             
        Serial1.begin(portinformation[i].serial_bps, portinformation[i].serial_config);
        portinformation[i].s = Serial1;
        break;
      case port_serial_ac:
        Serial2.begin(portinformation[i].serial_bps, portinformation[i].serial_config);
        portinformation[i].s = Serial2;
        break;
      case port_serial_rs485:
        Serial4.begin(portinformation[i].serial_bps, portinformation[i].serial_config);      
        portinformation[i].s = Serial4;
        if (services & SERVICE_MODBUS){
          NonBlockingModbusMaster nbmm;
          portinformation[i].modbus = nbmm;
          // NOTE: The NonBlockingModbusMaster is CUSTOM and is included by value rather than reference. It has
          // a PTT pin defined for RS485
          portinformation[i].modbus.initialize (portinformation[i].s, 5000, 5000, modbusinstance[i].ptt_pin ,1000000); // Delays are in uSec. TxDelay, TxHang, Timeout
        }
        break;
      // Ethernet Serial Port Abstraction
      case port_ethernet_telnet:
        if (services & SERVICE_TELNET){
          // To Be done
          //n portinformation[i].s = nothin;
        }
        break;
    }
    switch (portinformation[i].portfunction){
      case portfunction_cli:
        break;
      case portfunction_serialMQTT:
        break;
      case portfunction_serialAC:
        break;
      case portfunction_serialMODBUS:
        break;
      default: 
    }
  }
}



void setup_loadservices (void)
{
  // eventually load this from flash
  services = 0xFFFF;

}

void PowerOnSelfTest (void)
{
  // Not even sure we want to do anything here. 
  
  // maybe flash the trailer lights, just to say hello. 

}


// -----------------------------------------------------
// SETUP
// -----------------------------------------------------



void setup (void)
{

  setup_loadservices();

  for (uint8_t i=0; i < MQTT_UNIQUE_MAX; i++){
    mqtttosend[i].secondsSinceStart = 0;
  }
  setup_ports();


  if (services & SERVICE_CLI){      
  }
  if (services & SERVICE_ADC){      
    setup_adc();
  }
  if (services & SERVICE_TEMP){      
    setup_temperature();
  }
  if (services & SERVICE_PWM){    
    setup_PWM();  
  }
  if (services & SERVICE_RPM){      
    setup_RPM();
  }
  if (services & SERVICE_AC){      
  }
  if (services & SERVICE_MQTT){      
  }
  if (services & SERVICE_TELNET){      
  }
  if (services & SERVICE_OUTPUT){      
  }
  if (services & SERVICE_DOOR){      
  }
  if (services & SERVICE_MODBUS){      
    setup_modbus(); // It is OK if this runs and we dont do modbus later. 
  }



  PowerOnSelfTest();
  
}




void aircomms_settx ()
{
  aircomms.tx_buffer[0] = 0xAA;
  aircomms.tx_buffer[1] = 0x00;
  if (aircomms.tx_power){
    aircomms.tx_buffer[2] = 0x01; // Power On
  } else {
    aircomms.tx_buffer[2] = 0x00; // Power Off
  }
  aircomms.tx_buffer[3] = aircomms.tx_speed & 0xFF; // Low byte of speed
  aircomms.tx_buffer[4] = aircomms.tx_speed >> 8;   // High byte of speed
  aircomms.tx_buffer[5] = 0x00;
  aircomms.tx_buffer[6] = 0x00;
  aircomms.tx_buffer[7] = 0x00;
  aircomms.tx_buffer[8] = 0x00;
  aircomms.tx_buffer[9] = 0x00;
  aircomms.tx_buffer[10] = 0x00;
  aircomms.tx_buffer[11] = 0x00;
  aircomms.tx_buffer[12] = 0x00;
  aircomms.tx_buffer[13] = 0x00;
  uint16_t chksum = 0x100 - ((aircomms.tx_buffer[1] + aircomms.tx_buffer[2] + aircomms.tx_buffer[3] + aircomms.tx_buffer[4] +
                                    aircomms.tx_buffer[5] + aircomms.tx_buffer[6] +  aircomms.tx_buffer[7] + aircomms.tx_buffer[8] +
                                    aircomms.tx_buffer[9] + aircomms.tx_buffer[10] + aircomms.tx_buffer[11] + aircomms.tx_buffer[12] +
                                    aircomms.tx_buffer[13]) | 0xFF);
  aircomms.tx_buffer[14] = chksum;
  aircomms.tx_buffer[15] = 0x55;

}

bool aircomms_decode(void)
{

  uint16_t chksum = 0x100 - ((aircomms.rx_buffer[1] + aircomms.rx_buffer[2] + aircomms.rx_buffer[3] + aircomms.rx_buffer[4] +
                                    aircomms.rx_buffer[5] + aircomms.rx_buffer[6] +  aircomms.rx_buffer[7] + aircomms.rx_buffer[8] +
                                    aircomms.rx_buffer[9] + aircomms.rx_buffer[10] + aircomms.rx_buffer[11] + aircomms.rx_buffer[12] +
                                    aircomms.rx_buffer[13]) | 0xFF);

  if (
    (aircomms.rx_buffer[0] != 0xAA) || 
    (aircomms.rx_buffer[1] != 0x01) || 
    (aircomms.rx_buffer[15] != 0x55) ||
    (aircomms.rx_buffer[8] != 0x00) ||
    (aircomms.rx_buffer[12] != 0x00) || 
    (aircomms.rx_buffer[14] != chksum)
  ){
    return false;
  }

  aircomms.rx_compressor_speed = aircomms.rx_buffer[2] + (aircomms.rx_buffer[3]<<8);
  aircomms.rx_compressor_currernt = aircomms.rx_buffer[4] + (aircomms.rx_buffer[5]<<8);
  aircomms.rx_busbar_voltage = aircomms.rx_buffer[6] + (aircomms.rx_buffer[7]<<8);
  aircomms.rx_status_now = aircomms.rx_buffer[13];
  aircomms.rx_status_historical = aircomms.rx_buffer[9];
  aircomms.rx_secondssincestart = secondsSinceStart; // Save time of latest reading 
  return true;
}
















void processSerial (uint8_t port)
{

  switch (portinformation[port].portfunction){
    case portfunction_cli:
      portinformation[port].RxBufferSize = 0;
      return;
      break;
    case portfunction_serialMQTT:
      portinformation[port].RxBufferSize = 0;
      return;
      break;
    case portfunction_serialAC:
      portinformation[port].RxBufferSize = 0;
      return;
      break;
    case portfunction_serialMODBUS:
      portinformation[port].RxBufferSize = 0;
      return;
      break;
    default: 
  }
}

bool endofpacket (uint8_t port, uint8_t c)
{
    switch (portinformation[port].portfunction){
      case portfunction_cli:
      case portfunction_serialMQTT:
        if (c < 0x20){
          portinformation[port].RxBuffer[portinformation[port].RxBufferSize] = 0;
          return true;
        } else {
          return false;
        }
      break;
      case portfunction_serialAC:
        break;
      case portfunction_serialMODBUS:
        break;
      default: 
    }
}





int hmiPuts(uint8_t port, char *str, uint8_t mode);
void hmiPrintCommandPrompt(uint8_t port);
extern ParseCommands pCmd;


void processCLI(uint8_t port) {
  PortInformation &pi = portinformation[port];


  int16_t err = true;

    int16_t err_1;
    int16_t err_2;
    for (uint8_t pos = 0; pos < BUFFER_SIZE_RX; pos++) {
        if (pi.TxBuffer[pos] == 0x00) break;
        err = pCmd.read(pi.RxBuffer[pos]);
    }

    err = false;
    err_1 = pCmd.read('\r');
    if (!err_1) {
        err = pCmd.getError();
        // Serial.println (err);
    }
    err_2 = pCmd.read('\n');
    if (!err_2) {
        err = pCmd.getError();
        // Serial.println (err);
    }

    if (((pi.RxCharLast == '\r') && (pi.RxCharBeforeThat == '\n')) ||
        ((pi.RxCharLast == '\n') && (pi.RxCharBeforeThat == '\r'))) {
        // Strangely, we can ignire if there is a CR/LF or LF/CR, becasue we see
        // CR __OR__ LF as the end of line, and have aready dealt with it...
        pi.RxCharLast = ' ';
        pi.RxCharBeforeThat = ' ';
        return;
    }

    switch (err) {
        case -5:
        case -6:
            char buf[64];
            snprintf(buf, 64, "cmd >%s", pi.RxBuffer);
            hmiPuts(port, buf, HMI_CLI);

            hmiPuts(port, "Eh?", HMI_CLI);
            break;

        case 0:
            // Serial.print ("cmd >");
            // Serial.println (CommandLine);
            break;
        default:
            pi.s.println("");
            break;
    }

    pi.RxBuffer[0] = 0;

    pi.haveUsedHMIputs = true;

    hmiPrintCommandPrompt(port);
}

bool processCLIchar(uint8_t port)
{
    PortInformation &pi = portinformation[port];

    // read asynchronously  until full command input
    while (Serial.available())
    {
        char c = Serial.read();
        pi.RxCharBeforeThat = pi.RxCharLast;
        pi.RxCharLast = c;
        switch (c)
        {
        case '\n':
        case '\r':                     // likely have full command in buffer now,  commands are terminated by CR and/or LS
            pi.RxBuffer[pi.RxBufferSize] = '\0'; // null terminate our command char array
            if (pi.RxBufferSize > 0)
            {
                pi.RxBufferSize = 0; // charsRead is static,  so have to reset
                return true;
            }
            return true;
            break;
        case 0x7f:
        case '\b': //  handle backspace in input: put a space in last char
            if (pi.RxBufferSize > 0)
            { // and adjust commandLine and charsRead
                pi.RxBuffer[--pi.RxBufferSize] = '\0';
                pi.s.print("\b \b"); // no idea  how this works, found it on the Internet
            }
            break;
        default:
            // c = tolower(c);
            if (pi.RxBufferSize < BUFFER_SIZE_RX)
            {
                pi.RxBuffer[pi.RxBufferSize++] = c;
            }
            pi.s.print(c);
            pi.RxBuffer[pi.RxBufferSize] = '\0'; // just in case
            break;
        }
    }
    return false;
}


bool processMQTTchar(uint8_t port)
{
  PortInformation &pi = portinformation[port];

  while (pi.s.available()){
    pi.RxLast = millis();
    pi.RxBuffer[pi.RxBufferSize] = pi.s.read();
    pi.RxBufferSize++;
    if (endofpacket(port, pi.RxBuffer[pi.RxBufferSize-1])){
      processSerial(port); //process
      pi.RxBufferSize = 0;           
    }
    if (pi.RxBufferSize >= (BUFFER_SIZE_TX-2)){
      processSerial(port); //process
      pi.RxBufferSize = 0;
    }
  }
}

bool processACchar(uint8_t port)
{
  PortInformation &pi = portinformation[port];

  while (pi.s.available()){
    pi.RxLast = millis();
    aircomms.rx_buffer[aircomms.rx_buffer_pos] = pi.s.read();
    if (aircomms.rx_buffer_pos == 0){
      if (aircomms.rx_buffer[aircomms.rx_buffer_pos] != 0xAA){
        // Bad beginning of packet
        return;
      }
    }
    aircomms.rx_buffer_pos++;
    if (aircomms.rx_buffer_pos == 15){
      if (aircomms_decode()){
        // The packet is valid...
        sendMqttAc();
      }
      aircomms.rx_buffer_pos;
    }
  }
}



// -----------------------------------------------------
// LOOP
// -----------------------------------------------------


void loop_oncePerSecond(void)
{
    // This is for functions that need to run about once a second. 
    
    sample_RPM();

}



void loop (void)
{
  // This is a counter that permits us to only run some code periodically. For instance,
  // if we did the test ((every & 0x0F) == 0), this would be true every 16 loops. 
  static uint8_t every = 0;

  // Loop Philosophy...
  // 1. Calculate the number of seconds since system start. This is not quite as easy as it sounds, without using 
  // an interrupt. 
  




  // Time
  // Every one second, inc secondsSinceStart
  static uint32_t lastMillis = 0;

  if (millis() < lastMillis){
    // We have looped around - every 49 days
    lastMillis = millis();
  }
  if ((millis() - 1000) >= lastMillis){
    secondsSinceStart++;
    loop_oncePerSecond();
  }


    if (services & SERVICE_CLI){      
    }
    if (services & SERVICE_ADC){      
      // This is done via the AUTOMATON state machine. Dont need to worry about it
    }
    // if (services & SERVICE_TEMP){      
    // }
    if (services & SERVICE_PWM){      
    }
    if (services & SERVICE_RPM){     
      // dont do this every loop. Just every so often
      sample_RPM(); 
    }
    if (services & SERVICE_AC){
      // dont do this every loop
      aircomms_settx();
      then send the tx...
    }
    if (services & SERVICE_MQTT){      
    }
    if (services & SERVICE_TELNET){      
    }
    if (services & SERVICE_OUTPUT){      
    }
    // if (services & SERVICE_DOOR){      
    // }
    // if (services & SERVICE_MODBUS){      
    // }




    if (services & SERVICE_TEMP){      
      // bitrate is 16.3 kbps, so we have to sample at least at
      // 32.6 khz. Do this once per loop.
      temperature_loop();
    }






  // We probably do not need to run this every loop
  if ((every & 0x0F) == 0){
    // Run the state machine.
    automaton.run();
  }
  
  if ((every & 0x1F) == 0){
    // Run this every 32 loops
  }


  // Serial code will probably run the slowest. But the data comes in likely
  // the slowest too, or at least can cope with a heap of delay. For instance,
  // the serial CLI running at 115200 comes in at about 11520 bytes per
  // second. But there is an incoming buffer of 256 bytes, so we only need to
  // check about 45 times per second. 
  //
  // Of course, we want to check things a lot more often than that. Modbus is 
  // running at 19200 bps, or 1920 bytes per second. Assuming 10 bytes per packet, 
  // that is 192 packets per second. But Modbus is done elsewhere
  //
  // In essence, if we run this code every 16 loops, that is OK

  if ((every & 0x0F) == 0x08){
    // do not overlap with other every 16 operators.

    // Three times to process end of packet
    //   * When the buffer fills up
    //   * When there is an end of packet character
    //   * When there is a timeout. 

    for (uint8_t i = 0; i < portinformation_elementcount; i++){
      // i = enum of Port. 
      //     port_USB, portfunction_serialMQTT, port_ethernet_telnet, port_serial_ac, port_serial_rs485
      PortInformation &pi = portinformation[i];
      switch (i){
        // Standard Serial Ports
        case port_USB:               
        case port_ESP32:             
        case port_serial_ac:
          if (pi.s.available()){
            pi.RxLast = millis();      
            switch (portinformation[i].portfunction){
              case portfunction_cli:
                if (processCLIchar(i)){
                  processCLI(i);
                }
                break;
              case portfunction_serialMQTT:
                processMQTTchar(i);
                break;
              case portfunction_serialAC:
                processACchar(i);
                break;
              default: 
            }
          }
  

        // RS485 will be ModBus - at least at the moment. 
        case port_serial_rs485:
          // Ignore things hgere 
          break;
        // Ethernet Serial Port Abstraction
        case port_ethernet_telnet:

          break;
      }

      switch (portinformation[i].portfunction){
        case portfunction_cli:
          break;
        case portfunction_serialMQTT:
          break;
        case portfunction_serialAC:
          break;
        case portfunction_serialMODBUS:
          if (services & SERVICE_MODBUS){      
            if (portinformation[i].modbus.justFinished()) { // Also manage timeout
            xxx();
            }
          }
          break;
        default: 
      }

      // Timeouts

      if (pi.RxBufferSize > 0){
        if ((pi.RxLast > (millis() - pi.RxTimeout)) | (pi.RxLast >= (0xFFFF - pi.RxTimeout ))){
          // If we have a real timeout... OR...
          // If we are within a timeout from the length of millis()
          // Then process the data we have received regardless of anything else
          // This will mean that if data comes in at some point within the last (timeout) mSec of the 
          // 49 day cycle then these will be thrown away. Unlikely to happen. 
          processSerial(i); //process
          pi.RxBufferSize = 0;

        }
      }

    }
  }


  every++;


}

