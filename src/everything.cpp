#include "everything.h"

// Fan PWM for six ports
// Ran RPM for six ports
// Door Open/Close
// Power On/Off
// Read current - Raw and normalised
// RGB

// TEMP


// RPM - Interrupt Driven
// PWM - Timer
// Temperature - Library Polled
// Door - Event driven with timers















#define MAX_FAN_PWM_RPM 6
#define MAX_FAN_POWER 6
#define MAX_TEMP_STRINGS 8
#define MAX_TEMP_SENSORS_PER_STRING 4
#define MAX_DOORS 1

enum Voltage {
    voltage_12,         // 12V outputs
    voltage_48,         // 48V outputs
    voltage_supply,     // High Current 
    voltage_fans,       // Fans
    voltage_lights      // Lights, including RGB and Internal. Not the flood light
};

struct Hardware{
    Voltage mode;
    uint8_t index;
    uint32_t power_pin;
    uint16_t current_adc_address;
    uint16_t voltage_adc_address;
    char name[48];
};

// -------------------------------
// Switches and Current Monitoring

struct Hardware h[] = {
    {voltage_12, 0, PA0, 3, NULL, "12V Switched Output  1"},
    {voltage_12, 1, PA1, 3, NULL, "12V Switched Output  2"},
    {voltage_12, 2, PA1, 3, NULL, "12V Switched Output  3"},
    {voltage_12, 3, PA1, 3, NULL, "12V Switched Output  4"},
    {voltage_12, 4, PA1, 3, NULL, "12V Switched Output  5"},
    {voltage_12, 5, PA1, 3, NULL, "12V Switched Output  6"},
    {voltage_12, 6, PA1, 3, NULL, "12V Switched Output  7"},
    {voltage_12, 7, PA1, 3, NULL, "12V Switched Output  8"},
    {voltage_12, 8, PA1, 3, NULL, "12V Switched Output  9"},
    {voltage_12, 9, PA1, 3, NULL, "12V Switched Output 10"},
    {voltage_48, 0, PA1, 3, NULL, "48V Switched Output 1"},
    {voltage_48, 1, PA1, 3, NULL, "48V Switched Output 2"},
    {voltage_48, 2, PA1, 3, NULL, "48V Switched Output 3"},
    {voltage_48, 3, PA1, 3, NULL, "48V Switched Output 4"},
    {voltage_48, 4, PA1, 3, NULL, "48V Switched Output 5"},
    {voltage_48, 5, PA1, 3, NULL, "48V Switched Output 6"},
    {voltage_fans, 0, PA1, 3, NULL, "Fan 12V Total Current Usage"}, // Fan current measurement
    {voltage_fans, 1, PA1, NULL, NULL, "Fan 1"}, // Fan 1
    {voltage_fans, 2, PA1, NULL, NULL, "Fan 2"}, // Fan 2
    {voltage_fans, 3, PA1, NULL, NULL, "Fan 3"}, // Fan 3
    {voltage_fans, 4, PA1, NULL, NULL, "Fan 4"}, // Fan 4
    {voltage_fans, 5, PA1, NULL, NULL, "Fan 5"}, // Fan 5
    {voltage_fans, 5, PA1, NULL, NULL, "Fan 6"}, // Fan 6
    {voltage_lights, 0, NULL, 3, NULL, "Side Lights - Total Current"},
    {voltage_lights, 1, NULL, NULL, NULL, "Side Lights - Red"},
    {voltage_lights, 2, NULL, NULL, NULL, "Side Lights - Green"},
    {voltage_lights, 3, NULL, NULL, NULL, "Side Lights - Blue"},
    {voltage_supply, 0, NULL, 3, NULL, "12V Supply - Total Current Usage"},
    {voltage_supply, 1, NULL, 3, NULL, "48V Supply - Total Current Usage"},
    {voltage_supply, 2, NULL, 3, NULL, "48V Air Conditioner Supply - Switched"},
    {voltage_supply, 3, NULL, 3, NULL, "48V Supply - Switched"},
};

// ----------------
// Temperature Pins

struct TempPins{
    uint8_t index;
    uint32_t temp_pin;
};


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


struct FanPorts{
    uint8_t index;
    uint32_t pwm_pin;    
    uint32_t rpm_pin;    
};

// PWM is also set by timer
struct FanPorts fanpins[MAX_FAN_PWM_RPM]{
    {0, PB10, PC0}, // TIM2-3
    {1, NULL, PE10}, // TIM2-4 TODO: NULL is PB11
    {2, PD12, PC2}, // TIM4-1
    {3, PD13, PE3}, // TIM4-2
    {4, PD14, PA4}, // TIM4-3
    {5, PD15, PA5} // TIM4-4
};




















// CONSTRUCTORS

HardwareTimer *stmFanTimer_TIM2 = new HardwareTimer(TIM2); // PWM
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









// Value Storage


struct FanPWMRPM{
    uint32_t rpmcount;
    uint32_t lastrpmcount;
    uint32_t timediff;
    uint32_t countdiff;
    uint8_t pwm;
};

struct FanPWMRPM fan_pwm_rpm[MAX_FAN_PWM_RPM];


// Add how long since valid temperature read
struct Temperatures {
    float temp;
}






// ----------
// Fans - RPM
// ----------

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

  static uint32_t lasttime = 0;
  uint32_t now = millis();
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



// ----------
// Fans - PWM
// ----------

void setup_PWM (void)
{


  stmFanTimer_TIM2->pause();
  stmFanTimer_TIM4->pause();

  stmFanTimer_TIM2->setOverflow(25000, HERTZ_FORMAT);
  stmFanTimer_TIM4->setOverflow(25000, HERTZ_FORMAT);

  stmFanTimer_TIM2->setMode(3, TIMER_OUTPUT_COMPARE_PWM1, fanpins[0].pwm_pin); // TODO: Check ALT if needed
  stmFanTimer_TIM2->setMode(4, TIMER_OUTPUT_COMPARE_PWM1, fanpins[1].pwm_pin);

  stmFanTimer_TIM4->setMode(1, TIMER_OUTPUT_COMPARE_PWM1, fanpins[2].pwm_pin);

  // stmFanTimer_TIM3->setMode(2, TIMER_OUTPUT_COMPARE_PWM1, PA7_ALT1);
  stmFanTimer_TIM4->setMode(2, TIMER_OUTPUT_COMPARE_PWM1, fanpins[3].pwm_pin | ALT1); // Alternate timer needs a different pin name. WTF

  stmFanTimer_TIM4->setMode(3, TIMER_OUTPUT_COMPARE_PWM1, fanpins[4].pwm_pin | ALT1);
  stmFanTimer_TIM4->setMode(4, TIMER_OUTPUT_COMPARE_PWM1, fanpins[5].pwm_pin | ALT1);

  stmFanTimer_TIM2->resume();
  stmFanTimer_TIM4->resume();

  for (uint8_t i = 0; i < MAX_FAN_PWM_RPM; i++){
    fan_pwm_rpm[i].pwm = 0;
  }

  setFanPWMs();

}


// Interface
void setFanPWM(uint8_t fan, uint8_t percent)
{
  switch (fan){
    case 0:
      stmFanTimer_TIM2->setCaptureCompare(4, percent, PERCENT_COMPARE_FORMAT); 
      break;
    case 1:
      stmFanTimer_TIM2->setCaptureCompare(4, percent, PERCENT_COMPARE_FORMAT); 
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

// Interface
void setFanPWMs(void)
{
  for (uint8_t i = 0; i < MAX_FAN_PWM_RPM; i++){
    setFanPWM(i, fan_pwm_rpm[i].pwm);
  }  
}


// -----------
// Temperature


boolean tempsStale[6];

// ToDo: This needs to be improcved. If there are two sensors per input, it will only show one.
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





// -----------------------------------------------------
// SETUP
// -----------------------------------------------------



void setup (void)
{


    setup_RPM();


}




// -----------------------------------------------------
// LOOP
// -----------------------------------------------------



void loop (void)
{

    // Run once a second.
    // sample_RPM();

    temperature_loop();


}

