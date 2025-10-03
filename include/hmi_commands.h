/* Copyright (c) 2023-2024 Radioactive Networks Pty Ltd */
/* All Rights Reserved                                  */
/* darryl@radio-active.net.au                           */

#ifndef HMIOCTOPUS_H
#define HMIOCTOPUS_H




void infoStatus(void);
void infoEnable(void);
void infoSerial(void);
void infoFake(void);
void infoOsdp(void);
void infoRoute(void);
void infoOffset(void);
void infoEncrypt(void);
void infoLed(void);
void infoAddress(void);
void infoDebugosdp(void);
void infoSpeed(void);
void infoOutput(void);
void infoMaster(void);
void infoProtect(void);
void CmdProtect(int argc, char *argv[]);
void CmdMaster(int argc, char *argv[]);
void CmdSpeed(int argc, char *argv[]);
void CmdReset(int argc, char *argv[]);
void CmdSave(int argc, char *argv[]);
void CmdBasic(int argc, char *argv[]);
void CmdPause(int argc, char *argv[]);
void CmdPassword(int argc, char *argv[]);
void CmdDebugosdp(int argc, char *argv[]);
void CmdAddress(int argc, char *argv[]);
void CmdLed(int argc, char *argv[]);
void CmdOsdp(int argc, char *argv[]);
void CmdCp(int argc, char *argv[]);
void CmdHelp(int argc, char *argv[]);
void CmdInfo(int argc, char *argv[]);
void CmdFactory(int argc, char *argv[]);
void CmdRoute(int argc, char *argv[]);
void CmdEnable(int argc, char *argv[]);
void CmdEncrypt(int argc, char *argv[]);
void CmdMask(int argc, char *argv[]);
void CmdCard(int argc, char *argv[]);
void CmdOffset(int argc, char *argv[]);
void CmdFake(int argc, char *argv[]);
void CmdSerial(int argc, char *argv[]);
void CmdStats(int argc, char *argv[]);
void CmdHardware(int argc, char *argv[]);
void CmdSecure(int argc, char *argv[]);
void CmdInstall(int argc, char *argv[]);
void CmdOutput(int argc, char *argv[]);
void CmdShow(int argc, char *argv[]);
void CmdScan(int argc, char *argv[]);
void CmdInject(int argc, char *argv[]);




#endif