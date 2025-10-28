/* Copyright (c) 2023-2024 Radioactive Networks Pty Ltd */
/* All Rights Reserved                                  */
/* darryl@radio-active.net.au                           */


#ifndef HMI_H
#define HMI_H


#include <Arduino.h>

void hmiPrintCommandPrompt(void);
int hmiPuts(uint8_t port, char *str, uint8_t mode);
int hmiPuts(const char *str, uint8_t mode);
int hmiPutsTrace(const char *str);
void hmi (void);
void printCommand (void);

#define SPACE ' '

#define  COMMAND_BUFFER_LENGTH        64                     


uint8_t parseInt (char *arg);
uint16_t parse16Int (char *arg);
uint32_t parse32Int (char *arg);

bool getCommandLineFromSerialPort(char  * commandLine);



#endif



