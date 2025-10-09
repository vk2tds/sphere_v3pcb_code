/* Copyright (c) 2023-2024 Radioactive Networks Pty Ltd */
/* All Rights Reserved                                  */
/* darryl@radio-active.net.au                           */


#ifndef MAIN_H
#define MAIN_H


#define COPYRIGHT "INIT: COPYRIGHT 2023-2025 RADIOACTIVE NETWORKS PTY LTD"

// fallback if not set by platformio
#ifndef LAST_BUILD_TIME
#define LAST_BUILD_TIME 2024
#endif

#include <Unistd.h>


#include <Arduino.h>
#include <TimeLib.h>
#include <stdlib.h>
#include <stdio.h>

#include "CRC.h"
#include "CRC8.h"
#include <IWatchdog.h>
#include <Automaton.h>
#include <ArduinoUniqueID.h>

#include <OneWire.h>
#include <DallasTemperature.h>
#include <NonBlockingDallas.h>



#include "pins.h"




#define EEPROM_ADDRESS_SETTINGS 0 // Address of the settings

int pd_send_func(void *data, uint8_t *buf, int len);
int pd_recv_func(void *data, uint8_t *buf, int len);

extern uint8_t serialBuf[64];
extern uint8_t serialBufPos;



void welcome (void);
void main_setup(void);
void pins_setup (void);



extern uint16_t pins_flows[FLOWS_COUNT];
extern uint16_t pins_temps[TEMPS_COUNT];
extern uint16_t pins_fans_pwr[FANS_COUNT];
extern uint16_t pins_fans_rpm[FANS_COUNT];
extern uint16_t pins_fans_pwm[FANS_COUNT];
extern uint16_t pins_aux_pwr[4];



#endif


