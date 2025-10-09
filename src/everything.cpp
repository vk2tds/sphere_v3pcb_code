#include "everything.h"
#include "defines.h"

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





// TODO:
// State machine details for reading ADC
// Storing temperatures
// CLI
// More structure to the code
// Find the STM32F417 CPU - and define a CPU
// Define boards




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








void hardwareDoor (uint8_t door, uint8_t dooraction){
  switch (dooraction){
    case 0:
      doorstorage[door].DoorState = false;
    case 1:
      doorstorage[door].DoorState = true;
    case 2: // In the future this might change to something else. This is to unlock then lock the door
      doorstorage[door].DoorState = true;
    }
    sync();
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

void mqtt_door (uint8_t door_number, bool state){
  // TODO: Send MQTT
}





// ----------
// Fans - PWM
// ----------


// Interface
void setFanPWM(uint8_t fan, uint8_t percent)
{
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

// Interface
void setFanPWMs(void)
{
  for (uint8_t i = 0; i < MAX_FAN_PWM_RPM; i++){
    setFanPWM(i, fan_pwm_rpm[i].pwm);
  }  
}


void setup_PWM (void)
{


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
      door_pin_a.begin (doorpins[door_number].pin_a)
        .lead (600)
        .on()
        .blink (1000)
        .off();
      if (state){
        // Only needed if we are chanting the state of pin B
        door_pin_b.begin (doorpins[door_number].pin_b)
        .lead(500)
        .blink (1200)
        .off();
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
        door_pin_a.begin (doorpins[door_number].pin_a)
          .off()
          .pause(500)
          .blink(1000)
          .off();
        door_pin_b.begin (doorpins[door_number].pin_b)
          .off();
      }
      if (state == true){
        door_pin_a.begin (doorpins[door_number].pin_a)
          .off();
        door_pin_b.begin (doorpins[door_number].pin_b)
          .off()
          .pause(500)
          .blink(1000)
          .off();
      }




      // Set direction -> Turn power on -> turn off -> unset direction
      // time pin_a pin_b
      // 0    False False - Neutral State just in case
      // 500  False DIR   - Set direction
      // 600  True  DIR   - Turn power on
      // 1600 False DIR   - Turn power off
      // 1700 False False - Neutral State
      door_pin_a.begin (doorpins[door_number].pin_a)
        .lead (600)
        .on()
        .blink (1000)
        .off();
      if (state){
        // Only needed if we are chanting the state of pin B
        door_pin_b.begin (doorpins[door_number].pin_b)
        .lead(500)
        .blink (1200)
        .off();
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
  adcstorage[adc_index].readADC(adc_index & 0x03) = 0; // The value actually gets stored here.
  adcstorage[adc_index].secondsSinceStart = secondsSinceStart;





}


void adc_callback_state_1_set_multiplexor (int idx, int v, int up)
{
  // Set Multiplexor
  uint8_t multiplexor = adc_index >> 2;

  // ToDo: Read ADC
  ADC[adc_index].readADC(adc_index & 0x03);

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
    ADC[index] = new ADS1115 (0x048 + device, &Wire1);
  } else {
    ADC[index] = new ADS1115 (0x048 + device, &Wire2);
  }

  ADC[index].begin();
  ADC[index].setGain(0);         //  0 == 6.144 volt, default
  ADC[index].setDataRate(7);     //  0 = slow   4 = medium   7 = fast
  ADC[index].setMode(0);         //  0 == continuous mode
  ADC[index].readADC(channel_1); //  0 == default channel,  trigger first read

}



void setup_adc (void)
{

  // Init ADCs
  for (i=0; i < 4; i++){
    init_adc(0, i);
    init_adc(1, i);
  }





  adc_loop_timer.begin (30) // 30 mSec is about 1000 mSec / 32
    .onTimer (adc_callback)
    .start();
}

















// -----------------------------------------------------
// SETUP
// -----------------------------------------------------



void setup (void)
{


    setup_RPM();
    setup_adc();

}





// // -----------------------------------------------------
// // SYNC
// // -----------------------------------------------------


// void sync (void){

//   // DOORS
//   for (uint8_t i=0; i < MAX_DOORS;i++){
//      (if doorstorage[i].DoorState != doorpins[i].state)



//   }


// }









// -----------------------------------------------------
// LOOP
// -----------------------------------------------------



void loop (void)
{
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
  }





    // Run once a second.
    // sample_RPM();

    temperature_loop();



  // Run the state machine.
  automaton.run();

}

