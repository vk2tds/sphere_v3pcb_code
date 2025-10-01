/* Copyright (c) 2023-2024 Radioactive Networks Pty Ltd */
/* All Rights Reserved                                  */
/* darryl@radio-active.net.au                           */

#include "pins.h"
#include "Arduino.h"

//uint32_t pins[OSDP_MAX_WIEGAND_COUNT][5];

BOARD config_board;
PRODUCT config_product;
uint32_t config_rs485_rx;
uint32_t config_rs485_tx;
uint32_t config_rs485_ptt;
uint32_t config_board_led;
uint32_t config_board_pushbutton;

uint16_t pins_flows[FLOWS_COUNT];
uint16_t pins_temps[TEMPS_COUNT];
uint16_t pins_fans_pwr[FANS_COUNT];
uint16_t pins_fans_rpm[FANS_COUNT];
uint16_t pins_fans_pwm[FANS_COUNT];

uint16_t pins_aux_pwr[4];


void pins_setup (void)
{


    
    if (config_board == BOARD_PCB_V1){

        // uint16_t pins_flows[FLOWS_COUNT];
        // uint16_t pins_temps[TEMPS_COUNT];
        // uint16_t pins_fans_pwr[FANS_COUNT];
        // uint16_t pins_fans_rpm[FANS_COUNT];
        // uint16_t pins_fans_pwm[FANS_COUNT];

        pins_flows[0] = PE_9; // Off by one
        pins_flows[1] = PE_10;
        pins_flows[2] = PE_11;
        pins_flows[3] = PE_12;
        pins_flows[4] = PE_13;
        pins_flows[5] = PE_14;

        // ALSO NEEDS TO BE USED IN MAIN.


        pins_temps[0] = PD_0;
        pins_temps[1] = PD_1;
        pins_temps[2] = PD_2;
        pins_temps[3] = PD_3;
        pins_temps[4] = PD_4;
        pins_temps[5] = PD_5;

        pins_fans_pwr[0] = PD_6; pins_fans_rpm[0] = PB_15; pins_fans_pwm[0] = PA_0;
        pins_fans_pwr[1] = PD_7; pins_fans_rpm[1] = PC_0; pins_fans_pwm[1] = PA_1;
        pins_fans_pwr[2] = PB_5; pins_fans_rpm[2] = PC_1; pins_fans_pwm[2] = PA_6;
        pins_fans_pwr[3] = PB_6; pins_fans_rpm[3] = PC_2; pins_fans_pwm[3] = PA_7;
        pins_fans_pwr[4] = PB_7; pins_fans_rpm[4] = PA_4; pins_fans_pwm[4] = PB_0;
        pins_fans_pwr[5] = PB_8; pins_fans_rpm[5] = PA_5; pins_fans_pwm[5] = PB_1;


        for (uint8_t i = 0; i < FLOWS_COUNT; i++){
            pinMode (pins_flows[i], INPUT_PULLUP);
        }

        for (uint8_t i = 0; i < TEMPS_COUNT; i++){
            pinMode (pins_temps[i], INPUT_PULLUP);
        }

        for (uint8_t i = 0; i < FANS_COUNT; i++){
            pinMode (pins_fans_rpm[i], INPUT_PULLUP);
            pinMode (pins_fans_pwr[i], OUTPUT);            
            pinMode (pins_fans_pwm[i], OUTPUT);            
        }

        pins_aux_pwr[0] = PE_6;
        pins_aux_pwr[1] = PE_7;
        pins_aux_pwr[2] = PE_8;
        pins_aux_pwr[3] = PC_9;


        // D0 D1 LED BUZZER InputSwitch // Green White Blue Yellow

        // pins[0][0] = PE_1; pins[0][1] = PE_0; pins[0][2] = PB_6; pins[0][3] = PB_5; pins[0][4] = PB_7; // A
        // pins[1][0] = PD_7; pins[1][1] = PD_6; pins[1][2] = PD_4; pins[1][3] = PD_3; pins[1][4] = PD_5; // B
        // pins[2][0] = PD_2; pins[2][1] = PD_1; pins[2][2] = PC_12; pins[2][3] = PC_11; pins[2][4] = PD_0; // C
        // pins[3][0] = PD_14; pins[3][1] = PD_13; pins[3][2] = PD_11; pins[3][3] = PD_10; pins[3][4] = PD_12; // D
        // pins[4][0] = PD_9; pins[4][1] = PD_8; pins[4][2] = PB_14; pins[4][3] = PB_13; pins[4][4] = PB_15; // E
        // pins[5][0] = PE_15; pins[5][1] = PE_14; pins[5][2] = PE_12; pins[5][3] = PE_11; pins[5][4] = PE_13; // F
        // pins[6][0] = PA_0; pins[6][1] = PC_1; pins[6][2] = PC_15; pins[6][3] = PC_14; pins[6][4] = PC_0; // G
        // pins[7][0] = PE_6; pins[7][1] = PE_5; pins[7][2] = PE_3; pins[7][3] = PE_2; pins[7][4] = PE_4; // H

        // // Because my pin mapping is wrong on the STM32F411VETx
        // pins[0][0] = PE_0; pins[0][1] = PD_15; pins[0][2] = PB_6; pins[0][3] = PB_5; pins[0][4] = PB_7; // A
        // pins[1][0] = PD_6; pins[1][1] = PD_5; pins[1][2] = PD_3; pins[1][3] = PD_2; pins[1][4] = PD_4; // B
        // pins[2][0] = PD_1; pins[2][1] = PD_0; pins[2][2] = PC_11; pins[2][3] = PC_10; pins[2][4] = PC_15; // C
        // pins[3][0] = PD_13; pins[3][1] = PD_12; pins[3][2] = PD_10; pins[3][3] = PD_9; pins[3][4] = PD_11; // D
        // pins[4][0] = PD_8; pins[4][1] = PD_7; pins[4][2] = PB_13; pins[4][3] = PB_12; pins[4][4] = PB_14; // E
        // pins[5][0] = PE_14; pins[5][1] = PE_13; pins[5][2] = PE_11; pins[5][3] = PE_10; pins[5][4] = PE_12; // F
        // pins[6][0] = PA_0; pins[6][1] = PC_0; pins[6][2] = PC_14; pins[6][3] = PC_13; pins[6][4] = PB_15; // G
        // pins[7][0] = PE_5; pins[7][1] = PE_4; pins[7][2] = PE_2; pins[7][3] = PE_1; pins[7][4] = PE_3; // H

        // pins[0][1] = PE_0; pins[0][0] = PD_15;


        config_rs485_rx = PA_10;
        config_rs485_tx = PA_9;
        config_rs485_ptt = PC_3;
        config_board_led = PA_4; 
        config_board_pushbutton = PC_13; 
        // Because my pin mapping is wrong
        config_board_pushbutton = PC_12; 
    }




}
