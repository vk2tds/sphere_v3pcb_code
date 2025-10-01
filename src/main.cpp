/* Copyright (c) 2023-2024 Radioactive Networks Pty Ltd */
/* All Rights Reserved                                  */
/* darryl@radio-active.net.au                           */

// OSDP Spec - https://ict.co/media/grxlszzt/an-321_configuring_ict_readers_for_osdp_communication.pdf
// https://registry.platformio.org/libraries/gbertaz/NonBlockingDallas
// https://github.com/Gbertaz/NonBlockingDallas/blob/master/NonBlockingDallas.cpp


#include "main.h"

#define TIME_INTERVAL 1500                      //Time interval among sensor readings [milliseconds]
#define PINS_TEMPS_0 PC_15                      // All off by one!!!!
#define PINS_TEMPS_1 PD_0
#define PINS_TEMPS_2 PD_1
#define PINS_TEMPS_3 PD_2
#define PINS_TEMPS_4 PD_3
#define PINS_TEMPS_5 PD_4


boolean emulate = false; // Emulate the chiller

CRC8 crc ;

HardwareSerial Serial2(PA3, PA2); // (RX, TX)
#define QUEUE_LEN 32
char queue[QUEUE_LEN][96];

extern BOARD config_board;



int safe_strcat(char *s1, char *s2, size_t s1_size);
unsigned long uid(void);
unsigned long eeprom_crc(int start, int length);
unsigned long string_crc(char str[]);
void print_eeprom (void);
void settings_save (void);
void settings_destroy (void);
void utilityLoop(void);
void myhexdump(uint8_t *buf, uint16_t len, char *desc);
void settings_save_restart(bool new_mode_cp);






OneWire oneWire_A(PINS_TEMPS_0);
DallasTemperature dallasTemp_A(&oneWire_A);
NonBlockingDallas temperatureSensors_A(&dallasTemp_A);

OneWire oneWire_B(PINS_TEMPS_1);
DallasTemperature dallasTemp_B(&oneWire_B);
NonBlockingDallas temperatureSensors_B(&dallasTemp_B);

OneWire oneWire_C(PINS_TEMPS_2);
DallasTemperature dallasTemp_C(&oneWire_C);
NonBlockingDallas temperatureSensors_C(&dallasTemp_C);

OneWire oneWire_D(PINS_TEMPS_3);
DallasTemperature dallasTemp_D(&oneWire_D);
NonBlockingDallas temperatureSensors_D(&dallasTemp_D);

OneWire oneWire_E(PINS_TEMPS_4);
DallasTemperature dallasTemp_E(&oneWire_E);
NonBlockingDallas temperatureSensors_E(&dallasTemp_E);

OneWire oneWire_F(PINS_TEMPS_5);
DallasTemperature dallasTemp_F(&oneWire_F);
NonBlockingDallas temperatureSensors_F(&dallasTemp_F);


uint32_t serialNumber; // Unique ID for the PCB



Atm_timer timer_ReportFlows;
Atm_timer timer_ReportRPMs;
Atm_timer timer_ReportTemps;
Atm_timer timer_ReportPWMs;
Atm_timer timer_ReportReporting;
Atm_timer timer_PollChiller;
Atm_timer timer_ReportPower;
Atm_timer timer_TestSetTime;
Atm_timer timer_SetHardware;
Atm_timer timer_SendQueue;
Atm_timer timer_SerialNumber;
Atm_timer timer_Chiller;

HardwareTimer *stmFanTimer_TIM2 = new HardwareTimer(TIM2);
HardwareTimer *stmFanTimer_TIM3 = new HardwareTimer(TIM3);


uint8_t reportFlows = 5;
uint8_t reportTemps = 5;
uint8_t reportRPMs = 5;
uint8_t reportPWMs = 15;
uint8_t reportReports = 15;
uint8_t reportPower = 10;
uint8_t pollChiller = 1;
uint8_t testSetTime = 3;

boolean chiller_power = true;
uint8_t chiller_fault = 0;
uint16_t chiller_temp_current = 24 * 10;
uint16_t chiller_temp_set = 18 * 10;
uint16_t chiller_compressor = 2500 * 10;
uint16_t chiller_flow = 750 * 10;


void mqtt_output (char *line)
{
  for (uint8_t i = 0; i < QUEUE_LEN; i++){
    if (queue[i][0] == 0x00){
      // Queue is empty at this position
      strncpy (queue[i], line, 96);
      return;
    }
  }

}


void mqtt (char *topic, char *value)
{
  static char sentence[64];
  snprintf (sentence, 64, "%08lX%s,%s",serialNumber, topic, value);
  mqtt_output (sentence);
}


void mqtt (char *topic, char *value, uint8_t unit)
{
  static char sentence[64];
  snprintf (sentence, 64, "%08lX%s/%d,%s",serialNumber, topic, unit, value);
  mqtt_output (sentence);
}

void mqtt_10 (char *topic, uint16_t value)
{
  static char sentence[64];
  static char s_value[16];
  if (value > 9){
    snprintf (s_value, 16, "%d", value);   
  } else {
    snprintf (s_value, 16, "0%d", value);
  }
  uint8_t l = strlen (s_value);
  s_value[l+1] = 0;
  s_value[l] = s_value[l-1];
  s_value[l-1] = '.';
  snprintf (sentence, 64, "%08lX%s,%s",serialNumber, topic, s_value);
  mqtt_output (sentence);
}

void mqtt_10 (char *topic, int16_t value)
{
  static char sentence[64];
  static char s_value[16];
  if (abs(value) > 9){
    snprintf (s_value, 16, "%d", value);   
  } else {
    snprintf (s_value, 16, "0%d", value);
  }
  uint8_t l = strlen (s_value);
  s_value[l+1] = 0;
  s_value[l] = s_value[l-1];
  s_value[l-1] = '.';
  snprintf (sentence, 64, "%08lX%s,%s",serialNumber, topic, s_value);
  mqtt_output (sentence);
}


void mqtt_float (char *topic, float value)
{
  static char sentence[64];
  snprintf (sentence, 64, "%08lX%s,%f",serialNumber, topic, value);
  mqtt_output (sentence);
}


uint8_t parseInt(char *arg)
{
    char *ptr;
    long ret;
    if ((arg[0] >= '0') & (arg[0] <= '9'))
    {
        ret = strtoimax(arg, &ptr, 10);
        return (ret);
    }
    else
    {
        return -1;
    }
}

uint16_t parse16Int(char *arg)
{
    char *ptr;
    long ret;
    if ((arg[0] >= '0') & (arg[0] <= '9'))
    {
        ret = strtoimax(arg, &ptr, 10);
        return (ret);
    }
    else
    {
        return -1;
    }
}

uint32_t parse32Int(char *arg)
{
    char *ptr;
    long ret;
    if ((arg[0] >= '0') & (arg[0] <= '9'))
    {
        ret = strtoimax(arg, &ptr, 10);
        return (ret);
    }
    else
    {
        return -1;
    }
}


void mqtt (char *topic, int16_t value)
{
  static char sentence[64];
  snprintf (sentence, 64, "%08lX%s,%d",serialNumber, topic, value);
  mqtt_output (sentence);
}

void mqtt (char *topic, int32_t value, uint8_t unit)
{
  static char sentence[64];
  static char s_value[16];
  snprintf (sentence, 64, "%08lX%s/%d,%ld",serialNumber, topic, unit, value);
  mqtt_output (sentence);
}


boolean tempsStale[6];

// ToDo: This needs to be improcved. If there are two sensors per input, it will only show one.
void handleIntervalElapsed(int port, int deviceIndex, float temperature, String address)
{
  if (tempsStale[port]){
    tempsStale[port] = false;
    char topic[64];
    char addr[32];

    address.toCharArray(addr, address.length()+1);

    int16_t t = (temperature * 100);
    char temp[16];

    if (abs(t) > 100){
      snprintf (temp, 16, "%d");
    } else if (abs(t) > 10){
      snprintf (temp, 16, "0%d");
    } else {
      snprintf (temp, 16, "00%d");
    }

    uint8_t l = strlen (temp);
    temp[l+1] = 0;
    temp[l] = temp[l-1];
    temp[l-1] = temp[l-2];
    temp[l-2] = '.';




    snprintf (topic, 64, "/temp/%d/%d/%s", port + 1, deviceIndex, addr );
    mqtt (topic, temp);
  }
}



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

extern uint32_t config_rs485_rx;
extern uint32_t config_rs485_tx  ;
extern uint32_t config_rs485_ptt;




int16_t led_tx_count = 0;

void digitalClockDisplay()
{

  char buf[64];
  snprintf(buf, 64, "INIT: Compile Time: %02d:%02d:%02d %02d/%02d/%04d UTC", hour(), minute(), second(), day(), month(), year());
  Serial.println(buf);
}

void welcome(void)
{

  char buf[64];
  snprintf(buf, 64, "");
  Serial.println(buf);

  snprintf(buf, 64, "INIT: Welcome to CIBRAI STM32");
  Serial.println(buf);

  snprintf(buf, 64, COPYRIGHT);
  Serial.println(buf);

  snprintf(buf, 64, "INIT: http://www.radio-active.net.au");
  Serial.println(buf);

  snprintf(buf, 64, "INIT: Email us - sales@radio-active.net.au");
  Serial.println(buf);


#ifdef BOARD_PINS_PCBV1
  snprintf(buf, 64, "INIT: Pins: PCB Version 1.00");
  Serial.println(buf);
#endif



  uint32_t ser = uid();

  snprintf(buf, 64, "INIT: Serial Number: %08x", ser);
  Serial.println(buf);


  setTime(LAST_BUILD_TIME);
  digitalClockDisplay();
}

int rs485_send (uint8_t *buf, int len)
{
  static bool state = LOW;

  // if (settings.flash_LED_on_serial == 1)
  //   digitalWrite(config_board_led, state);
  led_tx_count++; // increase the comms counter
  state = !state;
  digitalWrite(config_rs485_ptt, HIGH);
  delayMicroseconds (80);
  Serial1.write(buf, len);
  Serial1.flush();
  delayMicroseconds (80); // debug
  digitalWrite(config_rs485_ptt, LOW);
  return len;
}

uint8_t serialBuf[64];
uint8_t serialBufPos = 0;

void initVariant(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 96;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    //Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
  {
    //Error_Handler();
  }
}


void SystemClock_Config(void)
{
 initVariant();
}



boolean aux_power[4];

uint8_t pwms[6];

boolean fan_power[6];
uint32_t lastFlows[6];
uint32_t flows[6];
void flowInt_A (void)
{
  flows[0] ++;
}

void flowInt_B (void)
{
  flows[1] ++;
}

void flowInt_C (void)
{
  flows[2] ++;
}

void flowInt_D (void)
{
  flows[3] ++;
}

void flowInt_E (void)
{
  flows[4] ++;
}

void flowInt_F (void)
{
  flows[5] ++;
}



uint32_t fansRpm[6];
uint32_t lastfansRpm[6];
void fansRpmInt_A (void)
{
  fansRpm[0] ++;
}

void fansRpmInt_B (void)
{
  fansRpm[1] ++;
}

void fansRpmInt_C (void)
{
  fansRpm[2] ++;
}

void fansRpmInt_D (void)
{
  fansRpm[3] ++;
}

void fansRpmInt_E (void)
{
  fansRpm[4] ++;
}

void fansRpmInt_F (void)
{
  fansRpm[5] ++;
}


void setFanPWM(uint8_t fan, uint8_t percent)
{
  switch (fan){
    case 0:
      stmFanTimer_TIM2->setCaptureCompare(1, percent, PERCENT_COMPARE_FORMAT); // 50%
      break;
    case 1:
      stmFanTimer_TIM2->setCaptureCompare(2, percent, PERCENT_COMPARE_FORMAT); // 50%
      break;
    case 2:
      stmFanTimer_TIM3->setCaptureCompare(1, percent, PERCENT_COMPARE_FORMAT); // 50%
      break;
    case 3:
      stmFanTimer_TIM3->setCaptureCompare(2, percent, PERCENT_COMPARE_FORMAT); // 50%
      break;
    case 4:
      stmFanTimer_TIM3->setCaptureCompare(3, percent, PERCENT_COMPARE_FORMAT); // 50%
      break;
    case 5:
      stmFanTimer_TIM3->setCaptureCompare(4, percent, PERCENT_COMPARE_FORMAT); // 50%
      break;
  }
}

void setFanPWMs(void)
{
  for (uint8_t i = 0; i < 6; i++){
    setFanPWM(i, pwms[i]);
  }  
}



void send_chiller_power(boolean state)
{
  uint8_t packet[10];
  packet[0] = 0xC0;
  packet[1] = 0x07;
  packet[2] = 0x01;
  if (state){
    packet[3] = 0x01;
  } else {
    packet[3] = 0x02;
  }
  packet[4] = 0x00;
  packet[5] = 0x00;
  packet[6] = calcCRC8(&packet[1], packet[1]-2);
  packet[7] = 0x01;

  rs485_send (packet, 8);
}

void send_chiller_temp(unsigned long t)
{
  uint16_t t2;
  t2 = (uint8_t) (t * 10);

  uint8_t packet[10];
  packet[0] = 0xC0;
  packet[1] = 0x07;
  packet[2] = 0x01;
  packet[3] = 0x04;
  packet[4] = (t2 & 0xFF00) >> 8;
  packet[5] = (t2 & 0xFF);
  packet[6] = calcCRC8(&packet[1], packet[1]-2);
  packet[7] = 0x01;

  chiller_temp_current = t;

  rs485_send (packet, 8);
}


uint8_t q_error;
boolean q_power;
int16_t q_ctemp;
int16_t q_stemp;
uint16_t q_speed;
uint16_t q_flow;

// Need POLL frequency
void decodePacket (uint8_t packet[], uint8_t len)
{
  char value[32];
  if (len >= 8){
    if (packet[0] == 0xc0){ // We have the start of a valid packet
      if (len >= (packet[1] + 1)){ // We have a valid lenth of packet
        if (packet[packet[1]] == 0x01){ // We have a valid end of packet
          if (packet[2] == 0x01){ // we have a valid packet to us
            // TODO: Check CRC8 here
            uint8_t mycrc = calcCRC8(&packet[1], packet[1]-2);
            if (calcCRC8(&packet[1], packet[1]-2) == packet[packet[1]-1]){
              if (packet[3] == 0x01){ // Chiller On
                if ((packet[4] == 0xFF) & (packet[5] == 0xFF)){ // Response
                  // Chiller On Message Received - Valid
                  mqtt("/chiller/power/get", "1");
                } else { // Command
                  chiller_power = true;
                  send_chiller_power(true);
                }
              }
              if (packet[3] == 0x02){ // Chiller Off
                if ((packet[4] == 0xFF) & (packet[5] == 0xFF)){
                  // Chiller Off Message Received - Valid
                  mqtt("/chiller/power/get", "0");
                } else {
                  chiller_power = false;
                  send_chiller_power(false);
                }
              }
              if (packet[3] == 0x03){ // Get Status
                // Query

                  q_power = packet[4] & 0x01; // Power on or off

                  uint8_t q_error = packet[4] >> 4; // Error code. 0 = No Error. 


                  q_ctemp = ((packet[5] & 0x7F) << 8) | packet[6]; // Current Temp - 0.1C
                  if (packet[7] & 0x80) q_ctemp = - q_ctemp;

                  q_stemp = ((packet[7] & 0x7F) << 8) | packet[8]; // Set Temp - 0.1C
                  if (packet[7] & 0x80) q_stemp = -q_stemp;

                  q_speed = (packet[9]<<8) | packet[10]; // Compressor speed in RPM
                  q_flow = (packet[11]<<8) | packet[12]; // units 0.1 Litres

                  // TODO: Print power
                  // TODO: Print error

                  if ((packet[5] == 0x00) & (packet[6] == 0x00) & (packet[7] == 0x00) & (packet[8] == 0x00)
                    & (packet[9] == 0x00) & (packet[10] == 0x00) & (packet[11] == 0x00) & (packet[12] == 0x00)){
                    // We are a query
                    Serial.println ("*** we have decoded RS485");

                    uint8_t mypacket2[] = {0xc0, 0x0E, 0x01, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x88, 0x01};

                    if (chiller_power){ // Power
                      mypacket2[4] = 0x01;
                    } else {
                      mypacket2[4] = 0x00;
                    }

                    mypacket2[4] |= ((chiller_fault & 0x0F) << 4); // Error Code

                    mypacket2[5] = chiller_temp_current >> 8; // Current Temp
                    mypacket2[6] = chiller_temp_current & 0xFF;

                    mypacket2[7] = chiller_temp_set >> 8; // Current Temp
                    mypacket2[8] = chiller_temp_set & 0xFF;

                    mypacket2[9] = chiller_compressor >> 8; // Current Temp
                    mypacket2[10] = chiller_compressor & 0xFF;

                    mypacket2[11] = chiller_flow >> 8; // Current Temp
                    mypacket2[12] = chiller_flow & 0xFF;

                    mypacket2[13] = calcCRC8(&mypacket2[1], mypacket2[1]-2);

                    rs485_send (mypacket2, 15);
                  } else { // we are getting a reply
                    if (q_power){
                      mqtt("/chiller/power/get", "1");  
                    } else {
                      mqtt("/chiller/power/get", "0");  
                    }

                    mqtt_10 ("/chiller/temp/get", q_stemp);
                    mqtt_10 ("/chiller/temp/current", q_ctemp);
                    mqtt_10 ("/chiller/speed/current", q_speed);
                    mqtt_10 ("/chiller/flow/current", q_flow);
                  }
              }
              if (packet[3] == 0x04){ // Set Temp
                if ((packet[4] == 0xFF) & (packet[5] == 0xFF)){
                  // Set Temp Message Valid
                  mqtt_10 ("/chiller/temp/get", q_stemp);
                } else {
                  q_stemp = (packet[4] << 8) | (packet[5]);
                  send_chiller_temp (q_stemp);
                  Serial.println ("Chiller Temp Set");
                }
              }
            } else {
              Serial.print ("Bad Packet");
            }
          }
        }
      }
    }
  }
}



void comms_RecvChillerOn(void)
{
  uint8_t mypacket[] = {0xc0, 0x07, 0x01, 0x01, 0xFF, 0xFF, 0x70, 0x01};
  // Serial.println ("Chill On");
  decodePacket (mypacket, 8);
}

void comms_RecvChillerOff(void)
{
  uint8_t mypacket[] = {0xc0, 0x07, 0x01, 0x02, 0xFF, 0xFF, 0xCD, 0x01};
  // Serial.println ("Chill Off");
  decodePacket (mypacket, 8);
}

void comms_RecvChillerSetTemp(void)
{
  uint8_t mypacket[] = {0xc0, 0x07, 0x01, 0x04, 0xFF, 0xFF, 0xB0, 0x01};
  // Serial.println ("Set Temp");
  decodePacket (mypacket, 8);
}

void comms_RecvChillerStatus(void)
{
  uint8_t mypacket[] = {0xc0, 0x0E, 0x01, 0x03, 0x51, 0x09, 0x60, 0x00, 0xC8, 0x13, 0x88, 0x00, 0x37, 0xC5, 0x01};
  // Serial.println ("Chill Status Req");
  decodePacket (mypacket, sizeof(mypacket));
  // uint8_t mypacket2[] = {0xc0, 0x0E, 0x01, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x88, 0x01};
  // Serial.println ("Chill Status Response");
  // decodePacket (mypacket2, sizeof(mypacket2));

}



void send_mqtt_queue  (int idx, int v, int up )
{
  for (uint8_t i = 0; i < QUEUE_LEN; i++){
    if (queue[i][0] != 0x00){
      Serial.println (queue[i]);
      Serial2.println (queue[i]);
      queue[i][0] = 0;
      break;
    }
  }
}




void printFlows (int idx, int v, int up )
{
  static uint32_t lasttime = 0;
  uint32_t now = millis();
  if (now < lasttime){ // Happens every 49 days or so
    lasttime = now;
    return;
  }
  uint32_t timediff = now - lasttime;
  lasttime = now;

  for (uint8_t i=0; i < 6; i++){
    if (flows[i] < lastFlows[i]){
      lastFlows[i] = flows[i]; // We will loose some flow every so often. 
      break;
    }
    uint32_t flowdiff = flows[i] - lastFlows[i];
    lastFlows[i] = flows[i];
    mqtt("/flow/flowdiff", flowdiff, i+1);
    uint32_t flowrate = flowdiff / (timediff/1000) * 60;
    mqtt("/flow/flowrate", flowrate, i+1);
  }
}



void printFansRPM (int idx, int v, int up )
{
  static uint32_t lasttime = 0;
  uint32_t now = millis();
  if (now < lasttime){ // Happens every 49 days or so
    lasttime = now;
    return;
  }
  uint32_t timediff = now - lasttime;
  lasttime = now;

  for (uint8_t i=0; i < 6; i++){
    if (fansRpm[i] < lastfansRpm[i]){
      lastfansRpm[i] = fansRpm[i];
      break;
    }
    uint32_t rpmdiff;
    rpmdiff = fansRpm[i] - lastfansRpm[i];
    uint32_t td = timediff / 100;

    uint32_t rpm = (rpmdiff / td) * 600 ;
    mqtt ("/fans/rpm", rpm, i+1);
    lastfansRpm[i] = fansRpm[i];
  }

}

void printFansPWM (int idx, int v, int up)
{
  static uint32_t lasttime = 0;
  uint32_t now = millis();
  if (now < lasttime){ // Happens every 49 days or so
    lasttime = now;
    return;
  }
  uint32_t timediff = now - lasttime;
  lasttime = now;

  for (uint8_t i=0; i < 6; i++){
    mqtt ("/fans/pwm/get", pwms[i], i+1);
  }
}



void run_chiller (int idx, int v, int up)
{
  static boolean last_chiller_power = chiller_power;

  if (!chiller_power){
    if (chiller_power != last_chiller_power){
      chiller_compressor = 0;
      chiller_flow = 0;
      //last_chiller_compressor = 0;
      //last_chiller_flow = 0;
    }
    int16_t diff = 350 - chiller_temp_current;
    chiller_temp_current += (diff/10);

    return;
  }

  if (chiller_power){
    int16_t diff = chiller_temp_current - chiller_temp_set;
    chiller_temp_current -= (diff/10);
    chiller_compressor += random (0,20);
    if (chiller_compressor < 100) chiller_compressor = 100;
    if (chiller_compressor > 7500) chiller_compressor = 7500;
    chiller_flow += random(0,20);
    if (chiller_flow < 500) chiller_flow = 500;
    if (chiller_flow > 5000) chiller_flow = 5000;
    return;
  }
}



int strpos(char *hay, char *needle, int offset)
{
   char haystack[strlen(hay)];
   strncpy(haystack, hay+offset, strlen(hay)-offset);
   char *p = strstr(haystack, needle);
   if (p)
      return p - haystack+offset;
   return -1;
}


void parse (char *line){
  IWatchdog.reload();
  // /chiller/temp/set
  // /chiller/power/set
  // /fans/pwm/set/#
  // /fans/power/set/#
  // /aux/power/set/#

  static char id[16];
  uint8_t offset;
  snprintf (id, 16, "%08lX", serialNumber);

  char value[32];

  if (strpos (line, ",", 0) > 0){
    // We have a comma
    strncpy (value, line + strpos (line, ",", 0)+1, 32);
  }
  
  if (strlen(value) == 0){
    // We need a value
    return;
  }

  if (strpos(line, id, 0) == 0){
    //Serial.println ("Found Serial");
    // We have found our ID at the start of the string
    offset = strlen (id);
    if (strpos(line, "/chiller/temp/set", offset) == offset){
      // TODO: Send value to chiller
      double decodedVal = atof (value);
      chiller_temp_set = decodedVal * 10;
      if (!emulate){
        send_chiller_temp(decodedVal);
      }
    }
    if (strpos(line, "/chiller/speed/current", offset) == offset){
      // TODO: Send value to chiller
      double decodedVal = atof (value);
      chiller_compressor = decodedVal * 10;
    }
    if (strpos(line, "/chiller/flow/current", offset) == offset){
      // TODO: Send value to chiller
      double decodedVal = atof (value);
      chiller_flow = decodedVal * 10;
    }
    if (strpos(line, "/chiller/temp/current", offset) == offset){
      // TODO: Send value to chiller
      double decodedVal = atof (value);
      chiller_temp_current = decodedVal * 10;
    }
    if (strpos(line, "/chiller/power/set", offset) == offset){
      //offset += strlen ("/chiller/temp/set/");
      if (value[0] == '1'){
        chiller_power = true;
        if (!emulate){
          send_chiller_power(true);
        }
      } else if (value[0] == '0') {
        chiller_power = false;
        if (!emulate){
          send_chiller_power(false);
        }
      }
    }
    if (strpos(line, "/fans/pwm/set/", offset) == offset){
      offset += strlen ("/fans/pwm/set/");
      uint8_t unit = atoi (line + offset);

      if ((unit < 1) | (unit > 6)){
        return;
      }
      int decodedVal = atoi (value);
      if ((decodedVal < 0) | (decodedVal > 100)){
        return;
      }
      pwms[unit-1] = decodedVal;
      //Serial.println ("Setting PWM");
      mqtt ("/fans/pwm/get", pwms[unit-1], unit);
      setFanPWM(unit-1, pwms[unit-1]);
    }
    if (strpos(line, "/fans/power/set", offset) == offset){
      offset += strlen ( "/fans/power/set");
      int unit = atoi (line + offset);
      if ((unit < 1) | (unit > 6)){
        return;
      }
      if (value[0] == '1'){
        fan_power[unit-1] = true;
        mqtt ("/fans/power/get", "1", unit);
      } else if (value[0] == '0') {
        fan_power[unit-1] = false;
        mqtt ("/fans/power/get", "0", unit);
      }
    }
    if (strpos(line, "/aux/power/set", offset) == offset){
      offset += strlen ("/aux/power/set");
      int unit = atoi (line + offset);
      if ((unit < 1) | (unit > 4)){
        return;
      }
      if (value[0] == '1'){
        aux_power[unit-1] = true;
        mqtt ("/aux/power/get", "1", unit);
      } else if (value[0] == '0') {
        aux_power[unit-1] = false;
        mqtt ("/aux/power/get", "0", unit);
      }
    }
  }
}

void setHardware (int idx, int v, int up)
{
  return;
  for (uint8_t i = 0; i < 6; i++){
    digitalWrite (pins_fans_pwr[i], fan_power[i]);
  }
  for (uint8_t i = 0; i < 4; i++){
    digitalWrite (pins_aux_pwr[i], aux_power[i]);
  }
  setFanPWMs();
}


void testSet (int idx, int v, int up)
{

  char line[64];


  static char sentence[64];
  snprintf (sentence, 64, "/%08lX%s,%s",serialNumber, "/chiller/temp/set", "21.1");
  parse(sentence);

  snprintf (sentence, 64, "/%08lX%s,%s",serialNumber, "/chiller/power/set", "1");
  parse(sentence);

  snprintf (sentence, 64, "/%08lX%s,%s",serialNumber, "/fans/pwm/set/3", "50");
  parse(sentence);

  snprintf (sentence, 64, "/%08lX%s,%s",serialNumber, "/fans/power/set/4", "1");
  parse(sentence);

  snprintf (sentence, 64, "/%08lX%s,%s",serialNumber, "/aux/power/set/2", "1");
  parse(sentence);


}

void printPower (int idx, int v, int up)
{
  for (uint8_t i=0; i < 6; i++){
    if (fan_power[i]){
      mqtt ("/fans/power/get", "1", i+1);
    } else {
      mqtt ("/fans/power/get", "0", i+1);
    }
  }
  for (uint8_t i=0; i < 4; i++){
    if (aux_power[i]){
      mqtt ("/aux/power/get", "1", i+1);
    } else {
      mqtt ("/aux/power/get", "0", i+1);
    }
  }
}

void printReporting (int idx, int v, int up)
{
    mqtt ("/reporting/flows", reportFlows);
    mqtt ("/reporting/temps", reportTemps);
    mqtt ("/reporting/fan_rpms", reportRPMs);
    mqtt ("/reporting/fan_pwms", reportPWMs);
    mqtt ("/reporting/reports", reportReports);
    mqtt ("/reporting/power", reportPower);
    mqtt ("/reporting/chiller", pollChiller);
}

void getTemps(int idx, int v, int up)
{
  for (uint8_t i = 0; i < 6; i++){
    tempsStale[i] = true;
  }
}

void PollChiller (int idx, int v, int up)
{
  uint8_t mypacket2[] = {0xc0, 0x0E, 0x01, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x88, 0x01};
  rs485_send (mypacket2, 15);
}


void send_serial_number (int idx, int v, int up)
{
  static char id[32];
  snprintf (id, 32, "serial_number,%08lX", serialNumber);
  Serial.println (id);
  Serial2.println (id);
}



void common_setup()
{
  char buf[64];

  // ***************************
  // ENABLE ME
  IWatchdog.begin(4000000);

  Serial.begin(115200); // HMI

  pinMode (PA4, OUTPUT);

  // Flashing LED's are needed so that button presses do not send the CPU into BOOT1 mode
  for (uint8_t i = 0; i < 10; i++)
  {
    digitalWrite (PA4, HIGH);
    delay (200);
    digitalWrite (PA4, LOW);
    delay (100);
  }

  welcome();

  IWatchdog.reload();
  
  pinMode(config_rs485_ptt, OUTPUT);
  digitalWrite(config_rs485_ptt, LOW);

  IWatchdog.reload();

  // Now that settings are loaded and are stable.
  Serial1.begin(9600 ); // RS485
  Serial2.begin (115200); 
}

unsigned long uid(void)
{
  // Hardware Serial Number
  // From https://github.com/khoih-prog/FlashStorage_STM32/blob/main/examples/EEPROM_CRC/EEPROM_CRC.ino
  const unsigned long crc_table[16] =
      {
          0x00000000, 0x1db71064, 0x3b6e20c8, 0x26d930ac,
          0x76dc4190, 0x6b6b51f4, 0x4db26158, 0x5005713c,
          0xedb88320, 0xf00f9344, 0xd6d6a3e8, 0xcb61b38c,
          0x9b64c2b0, 0x86d3d2d4, 0xa00ae278, 0xbdbdf21c};

  unsigned long crc = ~0L;

  for (int index = 0; index < UniqueIDsize; ++index)
  {
    crc = crc_table[(crc ^ UniqueID[index]) & 0x0f] ^ (crc >> 4);
    crc = crc_table[(crc ^ (UniqueID[index] >> 4)) & 0x0f] ^ (crc >> 4);
    crc = ~crc;
  }

  return crc;
}


void setup()
{
#ifdef BOARD_PINS_PCBV1
  config_board = BOARD_PCB_V1;

#endif 


  pins_setup();

  common_setup();

  for (uint8_t i = 0; i < 6; i++){
    tempsStale[i] = false;
  }

  for (uint8_t i = 0; i < 4; i++){
    aux_power[i] = true;
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

  serialNumber = uid();


  pinMode(pins_flows[0], INPUT);
  pinMode(pins_flows[1], INPUT);
  pinMode(pins_flows[2], INPUT);
  pinMode(pins_flows[3], INPUT);
  pinMode(pins_flows[4], INPUT);
  pinMode(pins_flows[5], INPUT);

  attachInterrupt(digitalPinToInterrupt(pins_flows[0]), flowInt_A, CHANGE);
  attachInterrupt(digitalPinToInterrupt(pins_flows[1]), flowInt_B, CHANGE);
  attachInterrupt(digitalPinToInterrupt(pins_flows[2]), flowInt_C, CHANGE);
  attachInterrupt(digitalPinToInterrupt(pins_flows[3]), flowInt_D, CHANGE);
  attachInterrupt(digitalPinToInterrupt(pins_flows[4]), flowInt_E, CHANGE);
  attachInterrupt(digitalPinToInterrupt(pins_flows[5]), flowInt_F, CHANGE);

  pinMode(pins_fans_rpm[0], INPUT_PULLUP);
  pinMode(pins_fans_rpm[1], INPUT_PULLUP);
  pinMode(pins_fans_rpm[2], INPUT_PULLUP);
  pinMode(pins_fans_rpm[3], INPUT_PULLUP);
  pinMode(pins_fans_rpm[4], INPUT_PULLUP);
  pinMode(pins_fans_rpm[5], INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(pins_fans_rpm[0]), fansRpmInt_A, CHANGE);
  attachInterrupt(digitalPinToInterrupt(pins_fans_rpm[1]), fansRpmInt_B, CHANGE);
  attachInterrupt(digitalPinToInterrupt(pins_fans_rpm[2]), fansRpmInt_C, CHANGE);
  attachInterrupt(digitalPinToInterrupt(pins_fans_rpm[3]), fansRpmInt_D, CHANGE);
  attachInterrupt(digitalPinToInterrupt(pins_fans_rpm[4]), fansRpmInt_E, CHANGE);
  attachInterrupt(digitalPinToInterrupt(pins_fans_rpm[5]), fansRpmInt_F, CHANGE);


  stmFanTimer_TIM2->pause();
  stmFanTimer_TIM3->pause();

  stmFanTimer_TIM2->setOverflow(25000, HERTZ_FORMAT);
  stmFanTimer_TIM3->setOverflow(25000, HERTZ_FORMAT);

  stmFanTimer_TIM2->setMode(1, TIMER_OUTPUT_COMPARE_PWM1, pins_fans_pwm[0]);
  stmFanTimer_TIM2->setMode(2, TIMER_OUTPUT_COMPARE_PWM1, pins_fans_pwm[1]);

  stmFanTimer_TIM3->setMode(1, TIMER_OUTPUT_COMPARE_PWM1, pins_fans_pwm[2]);
  stmFanTimer_TIM3->detachInterrupt();
  stmFanTimer_TIM3->detachInterrupt(1);
  stmFanTimer_TIM3->detachInterrupt(2);
  stmFanTimer_TIM3->detachInterrupt(3);
  stmFanTimer_TIM3->detachInterrupt(4);

  // stmFanTimer_TIM3->setMode(2, TIMER_OUTPUT_COMPARE_PWM1, PA7_ALT1);
  stmFanTimer_TIM3->setMode(2, TIMER_OUTPUT_COMPARE_PWM1, pins_fans_pwm[3] | ALT1); // Alternate timer needs a different pin name. WTF

  stmFanTimer_TIM3->setMode(3, TIMER_OUTPUT_COMPARE_PWM1, pins_fans_pwm[4] | ALT1);
  stmFanTimer_TIM3->setMode(4, TIMER_OUTPUT_COMPARE_PWM1, pins_fans_pwm[5] | ALT1);

  stmFanTimer_TIM2->resume();
  stmFanTimer_TIM3->resume();

  for (uint8_t i = 0; i < 6; i++){
    pwms[i] = 75;
  }

  setFanPWMs();

  if (digitalRead(pins_flows[0]) == false ){
    emulate = true;
    Serial.println ("Chiller Emulator");
  }


  if (!emulate){
    timer_ReportFlows.begin (reportFlows * 1000)
      .onTimer (printFlows)
      .repeat(-1)
      .start();
    timer_ReportRPMs.begin (reportRPMs * 1000)
      .onTimer (printFansRPM)
      .repeat(-1)
      .start();
    timer_ReportPWMs.begin (reportPWMs * 1000)
      .onTimer (printFansPWM)
      .repeat(-1)
      .start();
    timer_ReportReporting.begin (reportReports * 1000)
      .onTimer (printReporting)
      .repeat(-1)
      .start();
    timer_ReportTemps.begin (reportTemps * 1000)
      .onTimer (getTemps)
      .repeat(-1)
      .start();
    timer_PollChiller.begin (pollChiller * 1000)
      .onTimer (PollChiller)
      .repeat(-1)
      .start();
    timer_ReportPower.begin (reportPower * 1000)
      .onTimer (printPower)
      .repeat (-1)
      .start();
  }
  // timer_TestSetTime.begin (testSetTime * 1000)
  //   .onTimer (testSet)
  //   .repeat (-1)
  //   .start();

  timer_SetHardware.begin (25)
    .onTimer (setHardware)
    .repeat (-1)
    .start();

  timer_SendQueue.begin (125)
    .onTimer (send_mqtt_queue)
    .repeat (-1)
    .start();

  timer_SerialNumber.begin (60000)
    .onTimer (send_serial_number)
    .repeat (-1)
    .start();

  timer_Chiller.begin (10000)
    .onTimer (run_chiller)
    .repeat (-1)
    .start();



}


#define BUFFER_LEN 96
char Serial_buffer [BUFFER_LEN];
uint16_t Serial_buffer_pos = 0;
char Serial2_buffer [BUFFER_LEN];
uint16_t Serial2_buffer_pos = 0;
char SerialUSB_buffer [BUFFER_LEN];
uint16_t SerialUSB_buffer_pos = 0;


void rs485_recieve(void)
{
  static uint8_t Serial1_buffer [BUFFER_LEN];
  static uint16_t Serial1_buffer_pos = 0;
  static unsigned long last_serial = 0;

  if (last_serial > millis()){
    last_serial = millis();
    // Not perfect but it will work. Cause retry every 49 days
    return;
  }
  if ((millis() - last_serial) > 50){
    if (Serial1_buffer_pos > 0){
      decodePacket (Serial1_buffer, Serial1_buffer_pos);
      Serial1_buffer_pos = 0;
    }
  }
  while (Serial1.available() > 0){
    char c = Serial1.read();
    Serial1_buffer[Serial1_buffer_pos] = c;
    Serial1_buffer_pos++;

    if (Serial1_buffer_pos > (BUFFER_LEN - 3)){
      Serial1_buffer_pos = 0;
    }
    last_serial = millis();
  }
}




void loop()
{
  IWatchdog.reload();
  while (SerialUSB.available() > 0){
    char c = SerialUSB.read();
    if (c < 32){
      SerialUSB_buffer[SerialUSB_buffer_pos] = 0;
      parse(SerialUSB_buffer);
      Serial.println (SerialUSB_buffer);
      SerialUSB_buffer_pos = 0; 
      break; 
    }
    if (SerialUSB_buffer_pos >= (BUFFER_LEN -2)){
      SerialUSB_buffer_pos = 0;
      break;
    }
    SerialUSB_buffer[SerialUSB_buffer_pos] = c;
    SerialUSB_buffer_pos++;
  }
  IWatchdog.reload();

  while (Serial2.available() > 0){
    char c = Serial2.read();
    if (c < 32){
      Serial2_buffer[Serial2_buffer_pos] = 0;
      parse(Serial2_buffer);
      // Serial.print (">");
      // Serial.println (Serial2_buffer);
      Serial2_buffer_pos = 0; 
      break; 
    }
    if (Serial2_buffer_pos >= (BUFFER_LEN -2)){
      Serial2_buffer_pos = 0;
      break;
    }
    Serial2_buffer[Serial2_buffer_pos] = c;
    Serial2_buffer_pos++;
  }
  
  IWatchdog.reload();
  rs485_recieve();

  IWatchdog.reload();
  temperatureSensors_A.update();
  temperatureSensors_B.update();
  temperatureSensors_C.update();
  temperatureSensors_D.update();
  temperatureSensors_E.update();
  temperatureSensors_F.update();
  IWatchdog.reload();

  automaton.run();

}