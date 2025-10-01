/* Copyright (c) 2023-2024 Radioactive Networks Pty Ltd */
/* All Rights Reserved                                  */
/* darryl@radio-active.net.au                           */

#ifndef PINS_H
#define PINS_H

#define NONE 0xCDEF // Pin not assigned... 

typedef enum 
{
  BOARD_NUCLEO = 0,
  BOARD_PCB_V1,
  BOARD_PCB_V1_1
} BOARD;

//BOARD config_board;


typedef enum 
{
    PRODUCT_OCTOPUS = 0,
    PRODUCT_OCTOPUS_PRO,
    PRODUCT_SQUID
} PRODUCT;

//PRODUCT config_product;

//#include "main.h"

void pins_setup (void);


#define FLOWS_COUNT 6
#define TEMPS_COUNT 6
#define FANS_COUNT 6





#endif 