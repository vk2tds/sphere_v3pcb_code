/* Copyright (c) 2023-2024 Radioactive Networks Pty Ltd */
/* All Rights Reserved                                  */
/* darryl@radio-active.net.au                           */

#ifndef HMIOCTOPUS_H
#define HMIOCTOPUS_H




void infoSerial(void);
void infoLed(void);
void infoAddress(void);
void infoSpeed(void);
void infoOutput(void);
void CmdSpeed(int argc, char *argv[]);
void CmdReset(int argc, char *argv[]);
void CmdAddress(int argc, char *argv[]);
void CmdLed(int argc, char *argv[]);
void CmdCp(int argc, char *argv[]);
void CmdHelp(int argc, char *argv[]);
void CmdInfo(int argc, char *argv[]);
void CmdFactory(int argc, char *argv[]);
void CmdRoute(int argc, char *argv[]);
void CmdSerial(int argc, char *argv[]);
void CmdStats(int argc, char *argv[]);
void CmdHardware(int argc, char *argv[]);
void CmdOutput(int argc, char *argv[]);
void CmdShow(int argc, char *argv[]);




#endif