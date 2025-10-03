/* Copyright (c) 2023-2024 Radioactive Networks Pty Ltd */
/* All Rights Reserved                                  */
/* darryl@radio-active.net.au                           */

#include "everything.h"

extern ParseCommands pCmd;
extern Status statusFactory;
extern Status statusLive;
extern boolean pauseSave;         // Pause saving to flash.





// Commands
//
// door - last status
// door [DoorNumber] {1|0|open|close|lock|unlock} - lock and unlock doors
// 
// temp 
// temp mode {single|string}
// temp mode [StringNumber] {single|string}
// temp alarm stuff?
//
// ip...
//     DHCP, IP, Mask, Gateway, DNS1/2
//
// mqtt...
//     host, port, encrypt, usernamem password, which ports, {primary|secondary|round robin}, subscriptions
//     CLI over MQTT
//
// port... what data goes where
// port {com1|com2|com3|usb|ethernet} {cli|mqtt|slave|hvac}
//     serial speeds too etc
//
// fan
// fan [FanNumber]
// fan [FanNumber] pwm [0-100]
// fan [FanNumber] {power|pwr} [1|0|on|off]
//
// led... Is this flash mode?
//
// power {12v|48v|supply|fans|lights} [CircuitNumber] {0|1|On|Off|flash|toggle with time}
// 
// serial - display serial number/MAC/firmware/compile date etc









// Todo: Need to move these to setup() somehow. Not sure how :(
struct ParseCommands::command_t commandList[] = {
    // command, callback function
    "help", CmdHelp,
    "reset", CmdReset,
    "restart", CmdReset,
    "info", CmdInfo,
    "FACTORY", CmdFactory,
    "osdp", CmdOsdp,
    "route", CmdRoute,
    "encrypt", CmdEncrypt,
    "enable", CmdEnable,
    "fake", CmdFake,
    "led", CmdLed,
    "serial", CmdSerial,
    "mask", CmdMask,
    "card", CmdCard,
    "stats", CmdStats,
    "offset", CmdOffset,
    "hardware", CmdHardware,
    "address", CmdAddress,
    "debugosdp", CmdDebugosdp,
    "password", CmdPassword,
    "save", CmdSave,
    "pause", CmdPause,
    "basic", CmdBasic,
    "master", CmdMaster,
    "speed", CmdSpeed,
    "output", CmdOutput,
    "protect", CmdProtect, 
    "show", CmdShow, 
    "scan", CmdScan,
    "cp", CmdCp, 
    "inject", CmdInject,
    NULL, NULL // END OF LIST (NEEDED)
};
ParseCommands pCmd(commandList, 64, 12); // Constructor


bool check_if_true(int argc, char *argv[], int position)
{
  if (position > argc)
  {
    char buf[64];
    snprintf(buf, 64, "Out of Bounds in check_if_true!!!");
    hmiPuts(buf, HMI_CLI);
    return false;
  }
  if ((argv[position][0] == '1') | (argv[position][0] == 't') | (argv[position][0] == 'T') | (argv[position][0] == 'y') | (argv[position][0] == 'Y')) 
  {
    return true;
  } else {
    return false;
  }
}



// Pause actually saving settings
void CmdPause(int argc, char *argv[])
{
  printCommand();

  if (argc >= 1)
  {
    uint8_t p = parseInt(argv[0]);
    if (p != 0)pauseSave = true;
    else pauseSave = false;

  } else {
    pauseSave = !pauseSave;
  }

  if (pauseSave)
  {
    char buf[64];
    snprintf(buf, 64, "Saving of settings is paused");
    hmiPuts(buf, HMI_CLI);
  }
}

// Test Inputs
void CmdScan(int argc, char *argv[])
{
  printCommand();
  test_inputs();
  // never return. Ever

}

void CmdProtect(int argc, char *argv[])
{
  printCommand();
  if (argc == 0)
  {
    infoProtect();
    return;
  }

  uint8_t p = parseInt(argv[0]);;

  settings.protect = p;
  settings_save();
}


void infoCp_print_wiegand (uint8_t w_port, uint8_t state)
{
  char buf[96];
  char buf2[64];
  char buf3[32];

  // CP WIEGAND [WIEGAND]   LED     [HIGH|LOW] TEMP  [ON TIME] [OFF TIME] [COUNT] [ON COLOR] [OFF COLOR] = 10
  // CP WIEGAND [WIEGAND]   BUZ     [HIGH|LOW] TEMP  [ON TIME] [OFF TIME] [COUNT] = 8
  // CP WIEGAND [WIEGAND]   LED     [HIGH|LOW] PERM  [ON TIME] [OFF TIME] [ON COLOR] [OFF COLOR] = 9
  // CP WIEGAND [WIEGAND]   BUZ     [HIGH|LOW] PERM  [ON TIME] [OFF TIME] = 7

  if (state == 0)
  {
    snprintf (buf3, 32, "CP WIEGAND %d LED LOW  ", w_port+1);
  } else {
    snprintf (buf3, 32, "CP WIEGAND %d LED HIGH ", w_port+1);
  }

  // Add decoded colors
  snprintf (buf2, 64, "TEMP %03d %03d %d %d %d", 
          settings.cp_led_temp_time_on[w_port][state], 
          settings.cp_led_temp_time_off[w_port][state], 
          settings.cp_led_temp_count[w_port][state],
          settings.cp_led_temp_color_on[w_port][state],
          settings.cp_led_temp_color_off[w_port][state]);
  buf[0] = 0;
  safe_strcat(buf, buf3, 96);
  safe_strcat(buf, buf2, 96);
  hmiPuts (buf, HMI_CLI);

  // Add decoded colors
  snprintf (buf2, 64, "PERM %03d %03d %d %d",
          settings.cp_led_perm_time_on[w_port][state], 
          settings.cp_led_perm_time_off[w_port][state], 
          settings.cp_led_perm_color_on[w_port][state],
          settings.cp_led_perm_color_off[w_port][state]);
  buf[0] = 0;
  safe_strcat(buf, buf3, 96);
  safe_strcat(buf, buf2, 96);
  hmiPuts (buf, HMI_CLI);

  if (state == 0)
  {
    snprintf (buf3, 32, "CP WIEGAND %d BUZ LOW  ", w_port+1);
  } else {
    snprintf (buf3, 32, "CP WIEGAND %d BUZ HIGH ", w_port+1);
  }



  snprintf (buf2, 96, "TEMP %03d %03d %03d", 
          settings.cp_buz_temp_time_on[w_port][state], 
          settings.cp_buz_temp_time_off[w_port][state], 
          settings.cp_buz_temp_count[w_port][state]);
  buf[0] = 0;
  safe_strcat(buf, buf3, 96);
  safe_strcat(buf, buf2, 96);
  hmiPuts (buf, HMI_CLI);

}


void infoCp_print_osdp (uint8_t o_port)
{
    char buf[64];

  // CP OSDP    [OSDP PORT] ADDRESS [OSDP ADDRESS] - All ArgC == 4
  // CP OSDP    [OSDP PORT] WIEGAND [WIEGAND PORT]
  // CP OSDP    [OSDP PORT] ENCRYPTION xxx


   snprintf(buf, 64, "CP OSDP %d ENCRYPT %02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x", o_port + 1,
             settings.cp_osdp_scbk[o_port][0],
             settings.cp_osdp_scbk[o_port][1],
             settings.cp_osdp_scbk[o_port][1],
             settings.cp_osdp_scbk[o_port][2],
             settings.cp_osdp_scbk[o_port][3],
             settings.cp_osdp_scbk[o_port][4],
             settings.cp_osdp_scbk[o_port][5],
             settings.cp_osdp_scbk[o_port][6],
             settings.cp_osdp_scbk[o_port][7],
             settings.cp_osdp_scbk[o_port][8],
             settings.cp_osdp_scbk[o_port][9],
             settings.cp_osdp_scbk[o_port][10],
             settings.cp_osdp_scbk[o_port][11],
             settings.cp_osdp_scbk[o_port][12],
             settings.cp_osdp_scbk[o_port][13],
             settings.cp_osdp_scbk[o_port][14],
             settings.cp_osdp_scbk[o_port][15]);
    hmiPuts(buf, HMI_CLI);


  snprintf (buf, 64, "CP OSDP %d ADDRESS %d", o_port + 1, settings.cp_osdp_address[o_port] );
  hmiPuts (buf, HMI_CLI);

  snprintf (buf, 64, "CP OSDP %d WIEGAND %d", o_port + 1, settings.cp_osdp_wiegand[o_port] + 1);
  hmiPuts (buf, HMI_CLI);


  if (settings.cp_osdp_secure[o_port])
  {
    snprintf (buf, 64, "CP OSDP %d SECURE TRUE ", o_port + 1);
    hmiPuts (buf, HMI_CLI);
  } else {
    snprintf (buf, 64, "CP OSDP %d SECURE FALSE", o_port + 1);
    hmiPuts (buf, HMI_CLI);
  }

  if (settings.cp_osdp_install[o_port])
  {
    snprintf (buf, 64, "CP OSDP %d INSTALL TRUE ", o_port + 1);
    hmiPuts (buf, HMI_CLI);
  } else {
    snprintf (buf, 64, "CP OSDP %d INSTALL FALSE", o_port = 1);
    hmiPuts (buf, HMI_CLI);
  }

  if (settings.cp_osdp_enabled[o_port])
  {
    snprintf (buf, 64, "CP OSDP %d ENABLE TRUE ", o_port + 1);
    hmiPuts (buf, HMI_CLI);
  } else {
    snprintf (buf, 64, "CP OSDP %d ENABLE FALSE", o_port + 1);
    hmiPuts (buf, HMI_CLI);
  }

}

void infoCp()
{

  char buf[64];

  for (uint8_t w_port = 0; w_port < OSDP_MAX_WIEGAND_COUNT; w_port++)
  {
    for (uint8_t state = 0; state < 2; state++)
    {
      infoCp_print_wiegand (w_port, state);
    }
  }
  for (uint8_t o_port = 0; o_port < OSDP_MAX_CP; o_port++)
  {
    infoCp_print_osdp(o_port);
  }

  // This MUST be last...
  if (settings.mode_cp)
  {
    snprintf (buf, 64, "CP MODE TRUE");
  } else {
    snprintf (buf, 64, "CP MODE FALSE");
  }
  hmiPuts (buf, HMI_CLI);


}


void CmdCp(int argc, char *argv[])
{
  // CP 0       1           2       3           4         5          6         7          8 
  // CP WIEGAND [WIEGAND]   LED     [HIGH|LOW] TEMP  [ON TIME] [OFF TIME] [COUNT] [ON COLOR] [OFF COLOR] = 10
  // CP WIEGAND [WIEGAND]   BUZ     [HIGH|LOW] TEMP  [ON TIME] [OFF TIME] [COUNT] = 8
  // CP WIEGAND [WIEGAND]   LED     [HIGH|LOW] PERM  [ON TIME] [OFF TIME] [ON COLOR] [OFF COLOR] = 9
  // NOT THIS ONE CP WIEGAND [WIEGAND]   BUZ     [HIGH|LOW] PERM  [ON TIME] [OFF TIME] = 7

  // CP OSDP    [OSDP PORT] ADDRESS [OSDP ADDRESS] - All ArgC == 4
  // CP OSDP    [OSDP PORT] WIEGAND [WIEGAND PORT]
  // CP OSDP    [OSDP PORT] ENCRYPTION xxx
  // CP OSDP    [OSDP PORT] SECURE [Y/N]
  // CP OSDP    [OSDP PORT] INIT [Y/N] ???
  // CP OSDP    [OSDP PORT] ENABLE [Y/N]

  // CP MODE [Y/N]

  char buf[64];


  printCommand();

  if (argc == 0)
  {
    infoCp();
    return;
  }


  // CP MODE
  // Check to see if we want to change mode. 

  if (argc >= 2)
  {
    // CP MODE TRUE or FALSE
    if ((argv[0][0] == 'm') | (argv[0][0] == 'M')) 
    { // Mode
      bool state = false;
      if (check_if_true(argc, argv, 1)) // CP MODE TRUE
      {
        state = true;
      } else {
        state = false;
      }

      if (settings.mode_cp != state)
      {
        snprintf(buf, 64, "CLI: Changing Mode!!! Restarting!!!");
        hmiPuts(buf, HMI_CLI);
        //printf ("\r\nCLI: Changing Mode!!! Restarting Now!!!\r\n");
        delay (500);

        settings.mode_cp = state;
        settings_save(); //_restart(state); // special for 
        printf ("\r\nRestarting...\r\n");
        delay (500);

        if (settings.mode_cp)
        {
          snprintf(buf, 64, "Mode CP");
          hmiPuts(buf, HMI_CLI);
        } else {
          snprintf(buf, 64, "Mode PD");
          hmiPuts(buf, HMI_CLI);
        }

        printf ("\r\nRestarting...\r\n");
        delay (500);
        CmdReset(0, NULL);
      }
      return;
    } // Mode
  }

  // The command is Too Short
  if (argc < 3){
    char buf[64];
    snprintf(buf, 64, "Err: Syntax Error");
    hmiPuts(buf, HMI_CLI);
    return;
  }


  // CP WIEGAND
  if ((argv[0][0] == 'w') | (argv[0][0] == 'W'))
  { 

    if (argc < 6){
      char buf[64];
      snprintf(buf, 64, "Err: Syntax Error");
      hmiPuts(buf, HMI_CLI);
      return;
    }
    uint8_t wPort = parseInt(argv[1]); // CP WIEGAND n
    uint8_t ledbuzrel = 0; // LED Buz Relay
    bool tempperm = true;
    bool highlow = false;
    uint8_t on_time = 0;
    uint8_t off_time = 0;
    uint8_t count = 0;
    uint8_t on_color = 1; // red
    uint8_t off_color = 0; // off



    if ((wPort > OSDP_MAX_WIEGAND_COUNT))
    {
      char buf[64];
      snprintf(buf, 64, "Err: Wiegand port is bad");
      hmiPuts(buf, HMI_CLI);
      return;
    }

    // CP WIEGAND n LED
    if ((argv[2][0] == 'l') | (argv[2][0] == 'L'))
    {
      // LED
      ledbuzrel = 0;
    } else if ((argv[2][0] == 'b') | (argv[2][0] == 'B')) // CP WIEGAND n BUZ
    { 
      // Buzzer
      ledbuzrel = 1;
    } else { // // CP WIEGAND n RELAY
      // Relay
      ledbuzrel = 2;
    }

    // CP WIEGAND n LEDBUZ 
    // HIGH or LOW
    if ((argv[3][0] == 'H') | (argv[3][0] == 'h'))
    {
      // TEMP
      highlow = true;
    } else {
      // PERM
      highlow = false;
    }


    if ((argv[4][0] == 't') | (argv[4][0] == 'T'))
    {
      // TEMP
      tempperm = true;
    } else {
      // PERM
      tempperm = false;
    }


    on_time = parseInt (argv[5]);
    off_time = parseInt (argv[6]);

    if (ledbuzrel == 1) // Buzzer
    {
      if (argc < 8){
        char buf[64];
        snprintf(buf, 64, "Err: Syntax Error < 8");
        hmiPuts(buf, HMI_CLI);
        return;
      }
      count = parseInt (argv[7]);
      settings.cp_buz_temp_time_on[wPort-1][highlow] =  on_time;
      settings.cp_buz_temp_time_off[wPort-1][highlow] = off_time;
      settings.cp_buz_temp_count[wPort-1][highlow] = count;

      settings_save();
      return;
    } else { // LED
      if (tempperm == false) // Permament setting
      {
        if (argc < 9){
          char buf[64];
          snprintf(buf, 64, "Err: Syntax Error < 9");
          hmiPuts(buf, HMI_CLI);
          return;
        }
        on_color = parseInt(argv[7]);
        off_color = parseInt(argv[8]);
        settings.cp_led_perm_time_on[wPort-1][highlow] =  on_time;
        settings.cp_led_perm_time_off[wPort-1][highlow] = off_time;
        settings.cp_led_perm_color_on[wPort-1][highlow] =  on_color;
        settings.cp_led_perm_color_off[wPort-1][highlow] = off_color;
        settings_save();
        return;
      } else { // Temporary Setting
        if (argc < 10){
          char buf[64];
          snprintf(buf, 64, "Err: Syntax Error < 10");
          hmiPuts(buf, HMI_CLI);
          return;
        }
        count = parseInt (argv[7]);
        on_color = parseInt(argv[8]);
        off_color = parseInt(argv[9]);
        settings.cp_led_temp_time_on[wPort-1][highlow] =  on_time;
        settings.cp_led_temp_time_off[wPort-1][highlow] = off_time;
        settings.cp_led_temp_color_on[wPort-1][highlow] =  on_color;
        settings.cp_led_temp_color_off[wPort-1][highlow] = off_color;
        settings.cp_led_temp_count[wPort-1][highlow] = count;
        settings_save();
        return;
      }

  // CP WIEGAND [WIEGAND]   LED     [HIGH|LOW] TEMP  [ON TIME] [OFF TIME] [COUNT] [ON COLOR] [OFF COLOR] = 10
  // CP WIEGAND [WIEGAND]   BUZ     [HIGH|LOW] TEMP  [ON TIME] [OFF TIME] [COUNT] = 8
  // CP WIEGAND [WIEGAND]   LED     [HIGH|LOW] PERM  [ON TIME] [OFF TIME] [ON COLOR] [OFF COLOR] = 9



    }
    char buf[64];
    snprintf(buf, 64, "Err: Something went wrong");
    hmiPuts(buf, HMI_CLI);

    return;
  } 
  
  
  // WIEGAND

  // CP OSDP    [OSDP PORT] ADDRESS [OSDP ADDRESS]
  // CP OSDP    [OSDP PORT] WIEGAND [WIEGAND PORT]
  // CP OSDP    [OSDP PORT] ENCRYPTION xxx
  // CP OSDP    [OSDP PORT] SECURE [Y/N]
  // CP OSDP    [OSDP PORT] INIT [Y/N] ???
  // CP OSDP    [OSDP PORT] ENABLE [Y/N]


  if ((argv[0][0] == 'O') | (argv[0][0] == 'o'))
  { // OSDP
    if (argc < 3){
      char buf[64];
      snprintf(buf, 64, "Err: Syntax Error");
      hmiPuts(buf, HMI_CLI);
      return;
    }

    uint8_t oPort = parseInt(argv[1]);
    if ((oPort > OSDP_MAX_CP) | (oPort == 0))
    {
      char buf[64];
      snprintf(buf, 64, "Err: OSDP port is bad");
      hmiPuts(buf, HMI_CLI);
      return;
    }

    if ((argv[2][0] == 'A') | (argv[2][0] == 'a')) // Address
    {
      uint8_t address = parseInt (argv[3]);
      settings.cp_osdp_address[oPort-1] = address;
      settings_save();
      return;
    } // Address
    if ((argv[2][0] == 'W') | (argv[2][0] == 'w')) // Wiegand Port
    {
      uint8_t wPort = parseInt (argv[3]);
      if ((wPort > OSDP_MAX_WIEGAND_COUNT))
      {
        char buf[64];
        snprintf(buf, 64, "Err: OSDP port is bad");
        hmiPuts(buf, HMI_CLI);
        return;
      }
      settings.cp_osdp_wiegand[oPort-1] = wPort;
      settings_save();
      return;
    } // Wiegand Port
    if ((strcmp(argv[2], "ENCRYPTION") == 0) | (strcmp(argv[2], "encryption") == 0) | (strcmp(argv[2], "ENCRYPT") == 0) | (strcmp(argv[2], "encrypt") == 0)) // Encryption
    {
      uint8_t scbk_local[16];
      if (strcmp(argv[3], "default") == 0)
      {
        for (uint8_t i = 0; i < 16; i++)
        {
          settings.cp_osdp_scbk[oPort - 1][i] = 0x30 + i;
        }
        settings_save();
        return;
      }
      if (strlen(argv[3]) != 32)
      {
        char buf[128];
        snprintf(buf, 128, "Err: Encryption Key in HEX must be exactly 32 characters or `default`. Sorry.");
        hmiPuts(buf, HMI_CLI);
        return;
      }
      for (int8_t i = 0; i < 32; i++)
      {
        if ((argv[3][i] < '0') |
            ((argv[3][i] > '9') & (argv[3][i] < 'A')) |
            ((argv[3][i] > 'Z') & (argv[3][i] < 'a')) |
            (argv[3][i] > 'z'))
        {
          char buf[64];
          snprintf(buf, 64, "Err: Invalid Characters");
          hmiPuts(buf, HMI_CLI);
          return;
        }
      }
      uint8_t index = 0;
      uint8_t offset = 0;
      for (int8_t i = 0; i < 16; i++)
      {
        scbk_local[i] = 0;
        offset = 4;
        if ((argv[3][index] >= '0') & (argv[3][index] <= '9'))
        {
          scbk_local[i] = scbk_local[i] | (((argv[3][index] - '0') & 0x0F) << offset);
        }
        else
        {
          scbk_local[i] = scbk_local[i] | ((((argv[3][index] - 'A') & 0x07) + 10) << offset);
        }
        index++;

        offset = 0;
        if ((argv[3][index] >= '0') & (argv[3][index] <= '9'))
        {
          scbk_local[i] = scbk_local[i] | (((argv[3][index] - '0') & 0x0F) << offset);
        }
        else
        {
          scbk_local[i] = scbk_local[i] | ((((argv[3][index] - 'A') & 0x07) + 10) << offset);
        }
        index++;
      }
      // Save encryption
      for (uint8_t i = 0; i < 16; i++)
      {
        settings.cp_osdp_scbk[oPort - 1][i] = scbk_local[i];
      }
      settings_save();
      return;
    } // Encryption

    bool state = false;
    if (check_if_true (argc, argv, 3))
    //if ((argv[3][0] == '1') || (argv[3][0] == 't') || (argv[3][0] == 'T'))
    {
      state = true;
    }
    if ((strcmp(argv[2], "SECURE") == 0) | (strcmp(argv[2], "secure") == 0)) // Secure
    {
      settings.cp_osdp_secure[oPort -1] = state;
      settings_save();
      return;
    }
    if ((strcmp(argv[2], "ENABLE") == 0) | (strcmp(argv[2], "enable") == 0)) // Secure
    {
      settings.cp_osdp_enabled[oPort -1] = state;
      settings_save();
      return;
    }
    if ((strcmp(argv[2], "INSTALL") == 0) | (strcmp(argv[2], "install") == 0)) // Secure
    {
      settings.cp_osdp_install[oPort -1] = state;
      settings_save();
      return;
    }
    return;
  } // OSDP


}


void CmdOutput(int argc, char *argv[])
{
  if (argc == 0)
  {
    infoOutput();
    return;
  }

  if (argc < 2){
    char buf[64];
    snprintf(buf, 64, "Err: Syntax Error");
    hmiPuts(buf, HMI_CLI);
  }

  uint8_t oPort = parseInt(argv[0]);;
  uint8_t wPort = parseInt(argv[1]);; 

  if ((argv[1][0] == 'f') || (argv[1][0] == 'o')){
    wPort = 0;
  }

  if ((oPort > OSDP_MAX_OSDP_COUNT) || (oPort == 0))
  {
    char buf[64];
    snprintf(buf, 64, "Err: OSDP port is bad");
    hmiPuts(buf, HMI_CLI);
    return;
  }

  if ((wPort > OSDP_MAX_WIEGAND_COUNT))
  {
    char buf[64];
    snprintf(buf, 64, "Err: OSDP port is bad");
    hmiPuts(buf, HMI_CLI);
    return;
  }

  if (wPort == 0){
    settings.wiegand_output[oPort-1] = 0xff;
  } else {
    settings.wiegand_output[oPort-1] = wPort - 1;
  }
  char buf[64];
  if (settings.wiegand_output[oPort-1] == 0xff){
    snprintf(buf, 64, "Restart required");
    hmiPuts(buf, HMI_CLI);
  } else {
    snprintf(buf, 64, "Restart suggested");
    hmiPuts(buf, HMI_CLI);
    pinMode(reader[settings.wiegand_output[oPort-1]].in_D0, OUTPUT);
    pinMode(reader[settings.wiegand_output[oPort-1]].in_D1, OUTPUT);
    digitalWrite (reader[settings.wiegand_output[oPort-1]].in_D0, HIGH);
    digitalWrite (reader[settings.wiegand_output[oPort-1]].in_D1, HIGH);

  }

  settings_save();
}


void CmdSpeed(int argc, char *argv[])
{
  printCommand();
  if (argc == 0)
  {
    infoSpeed();
    return;
  }
  uint32_t speed = parse32Int(argv[0]);
  char buf[128];

  switch (speed){
    case 9600:
    case 19200:
    case 38400:
    case 57600:
    case 115200:
      settings.speed = speed / 9600;
      settings_save();
      snprintf (buf, 120, "OSDP speed set. Please restart unit to enable");
      hmiPuts(buf, HMI_CLI);
      return;
      break;
    default:
      snprintf (buf, 128, "Speed must be 9600, 19200, 38400, 57600 or 115200");
      hmiPuts(buf, HMI_CLI);
      return;
  }
}


 
void CmdShow(int argc, char *argv[])
{
  printCommand();
  if (!checkPassword())
    return; // Console locked

  char buf[64];
  char buf2[64];

  for (uint8_t i = 0; i < MAX_CARD_READS; i++)
  {
    uint8_t p = (i + nextCard) % MAX_CARD_READS;
    uint32_t now = millis();
    snprintf (buf, 32, "");
    if (lastCardReads[p].timestamp != 0)
    {
      snprintf (buf2, 64, "Port:%d Card:", lastCardReads[p].wiegand_port + 1);
      safe_strcat(buf, buf2, 64);

      uint8_t bytes = (lastCardReads[p].card_bits + 7) / 8;
      for (uint8_t j = 0; j < bytes; j++)
      {
        snprintf(buf2, 64, "%x%x", lastCardReads[p].card[j] >> 4, lastCardReads[p].card[j] & 0xF);
        safe_strcat(buf, buf2, 64);
      }
      snprintf (buf2, 64, " Bits:%d ", lastCardReads[p].card_bits);
      safe_strcat(buf, buf2, 64);
      
      if (now - lastCardReads[p].timestamp < 0){
        snprintf (buf2, 64, "Ago:%d ", (0xFFFF + now - lastCardReads[p].timestamp) / 1000);
        safe_strcat(buf, buf2, 64);
      } else {
        snprintf (buf2, 64, "Ago:%d ", (now - lastCardReads[p].timestamp) / 1000);
        safe_strcat(buf, buf2, 64);
      }
      hmiPuts(buf, HMI_STATUS);

    }
  }

}




void infoSpeed(void)
{
  char buf[64];
  snprintf (buf, 64, "speed %d", settings.speed * 9600);
  hmiPuts(buf, HMI_CLI);
}

void infoProtect(void)
{
  char buf[64];
  snprintf (buf, 64, "protect %d", settings.protect);
  hmiPuts(buf, HMI_CLI);
}



void CmdSave(int argc, char *argv[])
{
  printCommand();
  if (!checkPassword())
    return; // Console locked

  settings_save();
}



void CmdLed(int argc, char *argv[])
{
  printCommand();

  if (!checkPassword())
    return; // Console locked

  if (argc == 0)
  {
    infoLed();
    return;
  }
  if (argc >= 1)
  {
    uint8_t led_flash = parseInt(argv[0]);
    if (led_flash < 4){
      settings.flash_LED_on_serial = led_flash;
    } else {
      settings.flash_LED_on_serial = 0;
    }

    settings_save();
    infoLed();
  }
}

void CmdFactory(int argc, char *argv[])
{
  printCommand();

  if (!checkPassword())
    return; // Console locked

  char buf[64];
  snprintf(buf, 64, "Resetting to factory settings");
  hmiPuts(buf, HMI_CLI);

  initStatus(&statusFactory);
  initStatus(&statusLive);
  settings_destroy();
  CmdReset(0, NULL);
}

void CmdSerial(int argc, char *argv[])
{
  printCommand();
  infoSerial();
}















void CmdStats(int argc, char *argv[])
{
  printCommand();

  if (!checkPassword())
    return; // Console locked

  infoStatus();
}

void CmdCard(int argc, char *argv[])
{
  printCommand();

  if (!checkPassword())
    return; // Console locked

  if (argc == 0)
  {
    infoFake(); // Calls fake instead of Card.
    return;
  }
  if (argc >= 1)
  {
    int8_t oPort = parseInt(argv[0]);
    if ((oPort == 0) || (oPort > OSDP_MAX_WIEGAND_COUNT))
    {
      char buf[64];
      snprintf(buf, 64, "Err: Wiegand Port is bad");
      hmiPuts(buf, HMI_CLI);
      return;
    }
    card(oPort - 1);
    return;
  }
  char buf[64];
  snprintf(buf, 64, "Err: Syntax Error");
  hmiPuts(buf, HMI_CLI);
}

void CmdRoute(int argc, char *argv[])
{
  printCommand();

  if (!checkPassword())
    return; // Console locked

  if (argc == 0)
  {
    infoRoute();
    return;
  }
  if (argc >= 2)
  {
    uint8_t oPort = parseInt(argv[0]);
    uint8_t oOSDP = parseInt(argv[1]);
    if ((oPort >= 0) & (oOSDP >= 0))
    {
      if ((oPort == 0) || (oPort > OSDP_MAX_WIEGAND_COUNT))
      {
        char buf[64];
        snprintf(buf, 64, "Err: Wiegand Port is bad");
        hmiPuts(buf, HMI_CLI);
        return;
      }
      if ((oOSDP == 0) || (oOSDP > OSDP_MAX_OSDP_COUNT))
      {
        char buf[64];
        snprintf(buf, 64, "Err: OSDP Port is bad");
        hmiPuts(buf, HMI_CLI);
        return;
      }

      char buf[64];
      snprintf(buf, 64, "Routing Wiegand port %d from OSDP port %d to OSDP port %d", oPort, settings.wiegand_uplink_phy[oPort - 1] + 1, oOSDP);
      hmiPuts(buf, HMI_CLI);

      settings.wiegand_uplink_phy[oPort-1] = oOSDP - 1;
      settings_save();
      return;
    }
  }
  char buf[64];
  snprintf(buf, 64, "Err: Syntax Error");
  hmiPuts(buf, HMI_CLI);
}

void CmdAddress(int argc, char *argv[])
{
  printCommand();
  if (!checkPassword())
    return; // Console locked

  infoAddress();
}

void infoAddress(void)
{
  if (settings.mode_cp == false)
  {
    // PD Mode
    for (uint8_t i = 0; i < OSDP_MAX_OSDP_COUNT; i++)
    {
      if (powerup_enable_osdp[i])
      {
        char buf[64];
        snprintf(buf, 64, "Address OSDP Port %d: %04x ", i + 1, (settings.base_serial_no & 0xFFF0) + i);
        hmiPuts(buf, HMI_CLI);
      }
    }
  } else {
    // CP Mode
    char buf[64];
    snprintf(buf, 64, "No OSDP Address - CP Mode");
    hmiPuts(buf, HMI_CLI);
  }
}


void infoLed(void)
{
  char buf[64];

  switch (settings.flash_LED_on_serial)
  {
  case 0:
    snprintf(buf, 64, "led 0 # flash LED as watchdog");
    hmiPuts(buf, HMI_CLI);
    break;
  case 1:
    snprintf(buf, 64, "led 1 # flash LED on serial activity");
    hmiPuts(buf, HMI_CLI);
    break;
  case 2:
    snprintf(buf, 64, "led 2 # flash LED on wiegand activity");
    hmiPuts(buf, HMI_CLI);
    break;
  case 3:
    snprintf(buf, 64, "led 3 # flash LED on libosdp activity");
    hmiPuts(buf, HMI_CLI);
    break;
  default:
    snprintf(buf, 64, "led %d unknown", settings.flash_LED_on_serial);
    hmiPuts(buf, HMI_CLI);
    break;
  }
}


void infoSerial(void)
{
  char buf[64];

  snprintf(buf, 64, "serial %08x", settings.base_serial_no);
  hmiPuts(buf, HMI_CLI);
}

void infoOffset(void)
{
  for (uint8_t i = 0; i < OSDP_MAX_WIEGAND_COUNT; i++)
  {
    uint8_t bytes = 0;
    switch (settings.fake_cards_bits[i])
    {
    case 26:
      bytes = 4;
      break;
    case 34:
      bytes = 5;
      break;
    default:
      bytes = 5;
    }

    char buf[64];
    char buf2[32];

    snprintf(buf, 64, "offset %d ", i + 1);

    for (uint8_t j = 0; j < bytes; j++)
    {
      snprintf(buf2, 32, "%02x", settings.offset_cards[i][j]);
      safe_strcat(buf, buf2, 64);
    }
    snprintf(buf2, 32, " %d", settings.offset_cards_bits[i]);
    safe_strcat(buf, buf2, 64);
    hmiPuts(buf, HMI_CLI);
  }
}

void infoFake(void)
{
  char buf[64];
  char buf2[32];

  for (uint8_t i = 0; i < OSDP_MAX_WIEGAND_COUNT; i++)
  {
    uint8_t bytes = 0;
    switch (settings.fake_cards_bits[i])
    {
    case 26:
      bytes = 4;
      break;
    case 34:
      bytes = 5;
      break;
    default:
      bytes = 5;
    }
    snprintf(buf, 64, "fake %d ", i + 1);
    for (uint8_t j = 0; j < bytes; j++)
    {
      snprintf(buf2, 32, "%02x", settings.fake_cards[i][j]);
      safe_strcat(buf, buf2, 64);
    }
    snprintf(buf2, 32, " %d", settings.fake_cards_bits[i]);
    safe_strcat(buf, buf2, 64);
    hmiPuts(buf, HMI_CLI);
  }
}


void infoOutput (void)
{
  char buf[64];
  snprintf(buf, 64, "output osdp_port wiegand_port");
  hmiPuts(buf, HMI_CLI);

  for (uint8_t i = 0; i < OSDP_MAX_OSDP_COUNT; i++){
    if (settings.wiegand_output[i] == 0xff){
      snprintf (buf, 64, "output %d off", i+1);
    } else {
      snprintf (buf, 64, "output %d %d", i+1, settings.wiegand_output[i]+1);
    }
    hmiPuts (buf, HMI_CLI);
  }


}



void infoOsdp(void)
{
  char buf[64];
  snprintf(buf, 64, "osdp port address");
  hmiPuts(buf, HMI_CLI);

  for (uint8_t i = 0; i < OSDP_MAX_OSDP_COUNT; i++)
  {
    bool encrypt = false;
    for (uint8_t j = 0; j < 16; j++)
    {
      if (settings.scbk[i][j] != (0x30 + j))
      {
        encrypt = true;
      }
    }

    if (encrypt)
    {
      if (settings.enable_osdp[i])
      {
        snprintf(buf, 64, "osdp %d %d encrypted", i + 1, settings.osdp_address[i]);
        hmiPuts(buf, HMI_CLI);
      }
      else
      {
        snprintf(buf, 64, "osdp %d %d encrypted disabled", i + 1, settings.osdp_address[i]);
        hmiPuts(buf, HMI_CLI);
      }
    }
    else
    {
      if (settings.enable_osdp[i])
      {
        snprintf(buf, 64, "osdp %d %d default", i + 1, settings.osdp_address[i]);
        hmiPuts(buf, HMI_CLI);
      }
      else
      {
        snprintf(buf, 64, "osdp %d %d default disabled", i + 1, settings.osdp_address[i]);
        hmiPuts(buf, HMI_CLI);
      }
    }
  }
}

void CmdInfo(int argc, char *argv[])
{
  printCommand();

  if (!checkPassword())
    return; // Console locked

  char buf[64];
  snprintf(buf, 64, "Information...");
  hmiPuts(buf, HMI_CLI);

  infoRoute();
  infoOsdp();
  infoFake();
  infoOffset();
  infoEncrypt();
  infoOutput();
  infoSerial();
  infoStatus();
  infoEnable();
  infoCp(); // This ***MUST*** be last. Changing setting.mode_cp ***WILL*** force a restart!

}

void CmdReset(int argc, char *argv[])
{
  printCommand();
  if (!checkPassword())
    return; // Console locked

  char buf[64];
  snprintf(buf, 64, "Resetting...");
  hmiPuts(buf, HMI_CLI);
  Serial.flush();
  NVIC_SystemReset();
}



void printhelp(char *command, char *text, uint16_t values, uint8_t warning, uint8_t action)
{
  char buf[256];
  char buf2[256];

  hmiPuts(command, HMI_CLI);
  hmiPuts("", HMI_CLI);
  hmiPuts(text, HMI_CLI);

  if ((values > 0) | (action > 0))
  {
    hmiPuts("", HMI_CLI);
  }


// | Function                          | Minimum       | Maximum       | Example                           |
// |-----------------------------------|---------------|---------------|-----------------------------------|
// | 1 PD OSDP Port                      | 1             | 8             |                                   |
// | 2 CP OSDP Port                      | 1             | 16            |                                   |
// | 4 OSDP Address                      | 0             | 126           |                                   |
// | 8 OSDP Physical Address (Read Only) | 4 Hex Digits  | 4 Hex Digits  | 3FDC                              |
// | 16 Wiegand Port                      | 1             | 8             |                                   |
// | 32 Encryption Key                    | 32 Hex Digits | 32 Hex Digits | 303132333435363738393A3B3C3D3E3F  |
// | 64 RFID Card Number                  | 8 Hex Digits  | 10 Hex Digits | 22015993D5C1                      |
// | 128 On Time and Off Time              | 0             | 255           | 5 = 0.5 Seconds; 12 = 1.2 Seconds |

  if (values & VALUE_OSDP_PORT_PD)
  {
    snprintf (buf2, 256, " - [PD OSDP PORT] - 1-8");
    hmiPuts(buf2, HMI_CLI);
  }
  if (values & VALUE_OSDP_PORT_CP)
  {
    snprintf (buf2, 256, " - [CP OSDP PORT] - 1-8");
    hmiPuts(buf2, HMI_CLI);
  }
  if (values & VALUE_OSDP_ADDRESS)
  {
    snprintf (buf2, 256, " - [OSDP Address] - 0-256");
    hmiPuts(buf2, HMI_CLI);
  }
  if (values & VALUE_OSDP_ADDRESS_PHY)
  {
    snprintf (buf2, 256, " - [OSDP Physical Address] - Read Only - 4 Hex Digits - 0000-FFFF");
    hmiPuts(buf2, HMI_CLI);
  }
  if (values & VALUE_WIEGAND_PORT)
  {
    snprintf (buf2, 256, " - [Wiegand Port] - 1-8");
    hmiPuts(buf2, HMI_CLI);
  }
  if (values & VALUE_ENCRYPTION)
  {
    snprintf (buf2, 256, " - [Encryption Key] - 32 Hex Digits - 00000000000000000000000000000000-FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF");
    hmiPuts(buf2, HMI_CLI);
  }
  if (values & VALUE_RFID_CARD)
  {
    snprintf (buf2, 256, " - [RFID Card Number] - 10 Hex Digits - 0000000000-FFFFFFFFF");
    hmiPuts(buf2, HMI_CLI);
  }
  if (values & VALUE_ON_OFF_TIME)
  {
    snprintf (buf2, 256, " - [On Time and Off Time] - 0-255 - 255 = 25.5 Seconds. ");
    hmiPuts(buf2, HMI_CLI);
  }
  if (values & VALUE_TRUE)
  {
    snprintf (buf2, 256, " - [state] - The state may be 1, 0, true or false, yes or no");
    hmiPuts(buf2, HMI_CLI);
  }
  if (values & VALUE_LED)
  {
    snprintf (buf2, 256, " - [LED] - 0=Off. 1=Red. 2=Green. 3=Amber. 4=Blue. 5=Magenta. 6=Cyan. 7=White");
    hmiPuts(buf2, HMI_CLI);
  }

  switch (action)
  {
    case MODE_CP:
      snprintf (buf2, 256, " - Applies to Control Panel (CP) mode only");
      hmiPuts(buf2, HMI_CLI);
      break;
    case MODE_PD:
      snprintf (buf2, 256, " - Applies to Peripheral Device (PD) mode only");
      hmiPuts(buf2, HMI_CLI);
      break;
    case MODE_PD | MODE_CP:
      snprintf (buf2, 256, " - Applies to Peripheral Device (PD) and Control Panel (CP) modes");
      break;
  }

  if (warning & 0x01)
  {
    snprintf (buf2, 256, " - WARNING: This command will cause the device to restart");
    hmiPuts(buf2, HMI_CLI);
  }
  if (warning & 0x02)
  {
    snprintf (buf2, 256, " - WARNING: Restart is recommended");
    hmiPuts(buf2, HMI_CLI);
  }
}

void CmdHelp(int argc, char *argv[])
{
  printCommand();

  if (argc >= 1)
  {
    if (strcasecmp ("xyzzy", argv[0]) == 0)
    {
      char *command_line = "Xyzzy";
      char *command_help = "Nothing happens. Try Y2.";
      uint16_t values = 0;
      uint8_t warning = 0;
      uint8_t action = MODE_PD | MODE_CP; 
      printhelp (command_line, command_help, values, warning, action);
      return;
    }

    if (strcasecmp ("reset", argv[0]) == 0)
    {
      char *command_line = "reset";
      char *command_help = "This command simply causes the device to restart as if it was power cycled.";
      uint8_t warning = 1;
      uint16_t values = 1;
      uint8_t action = MODE_PD | MODE_CP; 
      printhelp (command_line, command_help, values, warning, action);
      return;
    }

    if (strcasecmp ("serial", argv[0]) == 0)
    {
      char *command_line = "serial";
      char *command_help = "Display the serial number of the device";
      uint8_t warning = 0;
      uint16_t values = 0;
      uint8_t action = MODE_PD | MODE_CP; 
      printhelp (command_line, command_help, values, warning, action);
      return;
    }

    if (strcasecmp ("address", argv[0]) == 0)
    {
      char *command_line = "address";
      char *command_help = "Display OSDP serial numbers for each port";
      uint8_t warning = 0;
      uint16_t values = 0;
      uint8_t action = MODE_PD; 
      printhelp (command_line, command_help, values, warning, action);
      return;
    }

    if (strcasecmp ("basic", argv[0]) == 0)
    {
      char *command_line = "basic";
      char *command_help = "Set some basic settings to make it easier to configure the device.";
      uint8_t warning = 0;
      uint16_t values = 0;
      uint8_t action = MODE_PD; 
      printhelp (command_line, command_help, values, warning, action);
      return;
    }

    if (strcasecmp ("pause", argv[0]) == 0)
    {
      char *command_line = "pause";
      char *command_help = "This command pauses saving any settings into memory until the command is issued again.";
      uint8_t warning = 0;
      uint16_t values = 0;
      uint8_t action = MODE_PD | MODE_CP; 
      printhelp (command_line, command_help, values, warning, action);
      return;
    }

    if (strcasecmp ("info", argv[0]) == 0)
    {
      char *command_line = "info";
      char *command_help = "Return the value of all settings.";
      uint8_t warning = 0;
      uint16_t values = 0;
      uint8_t action = MODE_PD | MODE_CP; 
      printhelp (command_line, command_help, values, warning, action);
      return;
    }

    if (strcasecmp ("save", argv[0]) == 0)
    {
      char *command_line = "save";
      char *command_help = "Saves all commands to memory when in pause mode. Can be used in scripts just in case.";
      uint8_t warning = 0;
      uint16_t values = 0;
      uint8_t action = MODE_PD | MODE_CP; 
      printhelp (command_line, command_help, values, warning, action);
      return;
    }

    if (strcasecmp ("stats", argv[0]) == 0)
    {
      char *command_line = "stats";
      char *command_help = "Display verious system statistics";
      uint8_t warning = 0;
      uint16_t values = 0;
      uint8_t action = MODE_PD | MODE_CP; 
      printhelp (command_line, command_help, values, warning, action);
      return;
    }

    if (strcasecmp ("show", argv[0]) == 0)
    {
      char *command_line = "show";
      char *command_help = "Display the last few Wiegand card reads, along with the port and the age. The `ago` number\r\n"
                           "is the approximate number of seconds since the card was read. ";
      uint8_t warning = 0;
      uint16_t values = 0;
      uint8_t action = MODE_PD | MODE_CP; 
      printhelp (command_line, command_help, values, warning, action);
      return;
    }

    if (strcasecmp ("factory", argv[0]) == 0)
    {
      char *command_line = "FACTORY";
      char *command_help = "This command is in UPPER CASE. Sending this command will cause the device to be set back to factory \r\nsettings and then restarted.";
      uint16_t values = 0;
      uint8_t warning = 1;
      uint8_t action = MODE_PD; 
      printhelp (command_line, command_help, values, warning, action);
      return;
    }

    if (strcasecmp ("password", argv[0]) == 0)
    {
      char *command_line = "password\r\n"
                            "password [Password]\r\n"
                            "password set [New Password | Blank | Serial]";
      char *command_help = "Typed by itself, this commands locks the console. \r\n"
                            "To unlock the console, type the command 'password' followed by the actual password\r\n"
                            "To set the password, tyoe 'password set' followed by the new password. To remove the password type 'password set blank'. "
                            "To set the password to the unit serial number type 'password set serial'";
      uint8_t warning = 0;
      uint16_t values = 0;
      uint8_t action = MODE_PD | MODE_CP; 
      printhelp (command_line, command_help, values, warning, action);
      return;
    }


    if (strcasecmp ("mask", argv[0]) == 0)
    {
      char *command_line = "mask";
      char *command_help = "The on board LED has a number of modes. Mode 0 has the led changing state as the main loop executes. In mode 2 the LED toggles in response to OSDP transmissions. Mode 3 momentarily flashes the LED in line with Wiegand output.";
      uint16_t values = 0;
      uint8_t warning = 0;
      uint8_t action = MODE_PD | MODE_CP; 
      printhelp (command_line, command_help, values, warning, action);
      return;
    }

    if (strcasecmp ("speed", argv[0]) == 0)
    {
      char *command_line = "speed [OSDP Speed]";
      char *command_help = "Sets or displays the speed of the OSDP interface. Valid speeds are 9600, 19200, 38400, 57600 or 115200.";
      uint16_t values = VALUE_OSDP_PORT_PD | VALUE_OSDP_ADDRESS;
      uint8_t warning = 1;
      uint8_t action = MODE_PD; 
      printhelp (command_line, command_help, values, warning, action);
      return;
    }


    if (strcasecmp ("osdp", argv[0]) == 0)
    {
      char *command_line = "osdp [PD OSDP Port] [OSDP Address]";
      char *command_help = "This device contains a number of OSDP interfaces. This command without any parameters will\r\nprint the OSDP address for each port";
      uint16_t values = VALUE_OSDP_PORT_PD | VALUE_OSDP_ADDRESS;
      uint8_t warning = 0;
      uint8_t action = MODE_PD; 
      printhelp (command_line, command_help, values, warning, action);
      return;
    }

    if (strcasecmp ("route", argv[0]) == 0)
    {
      char *command_line = "route [Wiegand Port] [PD OSDP Port]";
      char *command_help = "Unlike most Wiegand to OSDP interfaces, this device lets you assign multiple Wiegand readers to a \r\n"
                          "single OSDP port. There is no limit to the number of Wiegand devices that can be assigned to an OSDP port.";
      uint16_t values = VALUE_OSDP_PORT_PD | VALUE_WIEGAND_PORT;
      uint8_t warning = 0;
      uint8_t action = MODE_PD; 
      printhelp (command_line, command_help, values, warning, action);
      return;
    }

    if (strcasecmp ("encrypt", argv[0]) == 0)
    {
      char *command_line = "encrypt [PD OSDP Port] [Encryption Key]";
      char *command_help = "Set the encryption key for each OSDP port. Absolutely the worst is using the default encryption \r\n"
                            "key of '303132333435363738393A3B3C3D3E3F'. This key is often used to initially lock the device \r\n"
                            "before the Control Panel sends a new encryption key to the device. Some Control Panels will provide\r\n"
                            "the encryption key to the user, and it will be their responsibility to transfer it onto a Periheperal\r\n"
                            "Device. This command assists in that regard";
      uint16_t values = VALUE_OSDP_PORT_PD | VALUE_ENCRYPTION;
      uint8_t warning = 0;
      uint8_t action = MODE_PD; 
      printhelp (command_line, command_help, values, warning, action);
      return;
    }

    if (strcasecmp ("enable", argv[0]) == 0)
    {
      char *command_line =  "enable [Wiegand|led|buzzer|relay] [Wiegand Port] [state]\r\n"
                            "enable [osdp|install|secure]      [PD OSDP Port] [state]";
      char *command_help = "Enable and disable Wiegand, LED, Buzzer, Relay and OSDP ports, as well as install and secure modes on OSDP ports\r\n"
                            "OSDP Port commands only operate in Peripheral Mode";
      uint16_t values = VALUE_WIEGAND_PORT | VALUE_OSDP_PORT_PD | VALUE_TRUE;
      uint8_t warning = 0;
      uint8_t action = MODE_PD | MODE_CP; 
      printhelp (command_line, command_help, values, warning, action);
      return;
    }

    if (strcasecmp ("fake", argv[0]) == 0)
    {
      char *command_line = "fake [Wiegand Port] [RFID Card Number]";
      char *command_help = "Set a fake card for each Wiegand port to be sent on button press";
      uint16_t values = VALUE_WIEGAND_PORT | VALUE_RFID_CARD;
      uint8_t warning = 0;
      uint8_t action = MODE_PD | MODE_CP; 
      printhelp (command_line, command_help, values, warning, action);
      return;
    }

    if (strcasecmp ("inject", argv[0]) == 0)
    {
      char *command_line = "inject [Wiegand Port] [RFID Card Number]";
      char *command_help = "Inject RFID from the command line as if it was sent by an actual card reader. It works on an OSDP \r\n"
                            "port. The Card number needs to be eight or ten hex digits long. This command can be used to integrate\r\n"
                            "other hardware to the alarm system.";
      uint16_t values = VALUE_WIEGAND_PORT | VALUE_RFID_CARD;
      uint8_t warning = 0;
      uint8_t action = MODE_PD | MODE_CP; 
      printhelp (command_line, command_help, values, warning, action);
      return;
    }

    if (strcasecmp ("master", argv[0]) == 0)
    {
      char *command_line = "master [RFID Card Number]";
      char *command_help = "Set an RFID Card Number to unlock the console instead of a password. ";
      uint16_t values = VALUE_RFID_CARD;
      uint8_t warning = 0;
      uint8_t action = MODE_PD | MODE_CP; 
      printhelp (command_line, command_help, values, warning, action);
      return;
    }

    if (strcasecmp ("led", argv[0]) == 0)
    {
      char *command_line = "led [mode]";
      char *command_help = "The on board LED has a number of modes. Mode 0 has the led changing state as the main loop executes. \r\n"
                          "In mode 2 the LED toggles in response to OSDP transmissions. Mode 3 momentarily flashes the LED in line\r\n" 
                          "with Wiegand output.";
      uint16_t values = 0;
      uint8_t warning = 0;
      uint8_t action = MODE_PD | MODE_CP; 
      printhelp (command_line, command_help, values, warning, action);
      return;
    }

    if (strcasecmp ("card", argv[0]) == 0)
    {
      char *command_line = "card [Wiegand Port]";
      char *command_help = "As noted by the 'fake' command, it is possible to send a preset card read to the Control Panel when a button\r\n"
                            "is pressed. For testing, the 'card' command has been created to emulate that button being pressed. It can be\r\n"
                            "used on any of the Wiegand ports.";
      uint16_t values = VALUE_WIEGAND_PORT;
      uint8_t warning = 0;
      uint8_t action = MODE_PD | MODE_CP; 
      printhelp (command_line, command_help, values, warning, action);
      return;
    }

    if (strcasecmp ("offset", argv[0]) == 0)
    {
      char *command_line = "offset [Wiegand Port] [RFID Card Number]`";
      char *command_help = "Modify Wiegand RFID reads such algorithmicly. This us useful when combining multiple Wiegand ports to a single OSDP port";
      uint16_t values = VALUE_WIEGAND_PORT | VALUE_RFID_CARD;
      uint8_t warning = 0;
      uint8_t action = MODE_PD; 
      printhelp (command_line, command_help, values, warning, action);
      return;
    }

    if (strcasecmp ("output", argv[0]) == 0)
    {
      char *command_line =  "enable [PD OSDP Port] [Wiegand Port]";
      char *command_help = "Route OSDP port card reads to the specified Wiegand Port as an output. Set the Wiegand Port to 0 to disable";
      uint16_t values = VALUE_WIEGAND_PORT | VALUE_OSDP_PORT_PD;
      uint8_t warning = 2;
      uint8_t action = MODE_PD; 
      printhelp (command_line, command_help, values, warning, action);
      return;
    }

    if (strcasecmp ("protect", argv[0]) == 0)
    {
      char *command_line =  "protect [state]\r\n";
      char *command_help = "By default, very little information is shown on the serial console unless the user has entered a password. By \r\n"
      "setting `protect 0`, all status information will be displayed even if the console is locked. A warning, however, is that the console\r\n"
      "has the potential to leak information that could be used to bypass security.";
      uint16_t values = VALUE_TRUE;
      uint8_t warning = 0;
      uint8_t action = MODE_PD | MODE_CP; 
      printhelp (command_line, command_help, values, warning, action);
      return;
    }

    if (strcasecmp ("debugosdp", argv[0]) == 0)
    {
      char *command_line =  "debugosdp [PD OSDP Port | CP OSDP PORT] [Flags]";
      char *command_help = "It provides low level debugging of the communications between the control panel and the board. Debugging is \r\non a per OSDP port basis."
            "Flags get added up to the value needed. 1 = Data Trace. 2 = Packet Trace. \r\n4 = Monitor POLL packets. 128 for debug of LED and BUZZER. To disable,"
            "use a Flag value \r\nof 0. This may need to be sent without seeing your text on the screen.";
      uint16_t values = VALUE_OSDP_PORT_CP | VALUE_OSDP_PORT_PD;
      uint8_t warning = 0;
      uint8_t action = MODE_PD | MODE_CP; 
      printhelp (command_line, command_help, values, warning, action);
      return;
    }

    if (strcasecmp ("cp", argv[0]) == 0)
    {
      char *command_line =  "cp mode [state]\r\n"
                            "cp osdp    [CP OSDP Port] ENABLE     [state]\r\n"
                            "cp osdp    [CP OSDP PORT] ADDRESS    [OSDP ADDRESS]\r\n"
                            "cp osdp    [CP OSDP PORT] WIEGAND    [WIEGAND PORT]\r\n"
                            "cp osdp    [CP OSDP PORT] SECURE     [state]\r\n"
                            "cp osdp    [CP OSDP PORT] INSTALL    [state]\r\n"
                            "cp osdp    [CP OSDP PORT] ENCRYPTION [encryption]\r\n"
                            "cp wiegand [WIEGAND PORT] LED        [HIGH|LOW] TEMP  [ON TIME] [OFF TIME] [COUNT] [ON COLOR] [OFF COLOR]\r\n"
                            "cp wiegand [WIEGAND PORT] BUZ        [HIGH|LOW] TEMP  [ON TIME] [OFF TIME] [COUNT]\r\n"
                            "cp wiegabd [WIEGAND PORT] LED        [HIGH|LOW] PERM  [ON TIME] [OFF TIME] [ON COLOR] [OFF COLOR]";
      char *command_help = "CP Commands. 'cp mode' turns CP mode ON or OFF. A restart is needed after this command. OSDP commands act on \r\n"
                            "an OSDP connection. Wiegand commands work on a Wiegand port"
                            "Setting the Wiegand \r\nPort to 0 will disable that mapping. ";
      uint16_t values = VALUE_OSDP_PORT_CP | VALUE_WIEGAND_PORT | VALUE_TRUE | VALUE_ENCRYPTION | VALUE_ON_OFF_TIME | VALUE_LED;
      uint8_t warning = 2;
      uint8_t action = MODE_CP; 
      printhelp (command_line, command_help, values, warning, action);
      return;
    }

  }


  char *cmds[] = {
    "### System Commands",
    "reset        - restart the device",
    "FACTORY      - reset to factory settings",
    "serial       - display the hardware serial number",
    "speed        - adjust the OSDP speed",
    "password     - enter or change the password",
    " ",
    "### Controp Commands",
    "basic        - apply sample settings to make it easier to get up and running",
    "pause        - toggle auto-save of settings",
    "save         - save settings when auto-save is paused",
    "info         - display all settings",
    "stats        - display system statistics",
    "help         - display help on individual commands",
    "led          - change the meaning of the on board LED",
    " ",
    "### OSDP Commands",
    "osdp         - set the OSDP address for each port",
    "route        - route Wiegand readers to OSDP ports",
    "encrypt      - set the OSDP encryption on each port",
    "enable       - enable and disable OSDP, Wiegand, LED, Buzzer and relay ports",
    "debugosdp    - enable low level debugging",
    "mask         - display the status of the OSDP connections",
    "address      - display addresses for each OSDP connection",
    " ",
    "### Wiegand Commands",
    "fake         - assign a fake RFID card to each port",
    "info         - display recent RFID card reads",
    "card         - send an RFID card as if it was read directly",
    " ",
    "### CP Commands",
    "cp           - CP related commands. Also enter and exit CP mode",
    "\0"
  };

  for (uint8_t i = 0; i < 32; i++)
  {
    hmiPuts (cmds[i], HMI_CLI);
  }




}

void infoHardware()
{
  char buf[64];
  char buf2[64];
  snprintf(buf, 64, "Hardware Status");
  hmiPuts(buf, HMI_CLI);

  for (uint8_t i = 0; i < OSDP_MAX_WIEGAND_COUNT; i++)
  {
    if (pins[i][0] != NONE)
    {
      snprintf(buf, 64, "wiegand %02d %d %d %d %d %d", i + 1,
               digitalRead(pins[i][0]),
               digitalRead(pins[i][1]),
               digitalRead(pins[i][2]),
               digitalRead(pins[i][3]),
               digitalRead(pins[i][4]));
      hmiPuts(buf, HMI_CLI);
    }
    else
    {
      snprintf(buf, 64, "wiegand %02d - - - - -", i + 1);
      hmiPuts(buf, HMI_CLI);
    }
  }
}

void CmdHardware(int argc, char *argv[])
{
  // hardware wiegand port pin state
  //     -1      0      1   2    3
  printCommand();
  if (!checkPassword())
    return; // Console locked

  // hardware osdp 1 2 1
  // hardware oddp 1 3 1

  if (argc == 0)
  {
    infoHardware();
    return;
  }

  if (argc >= 3)
  {
    uint8_t oPort = parseInt(argv[1]);
    uint8_t oPin = parseInt(argv[2]);
    uint8_t mode = MODE_NONE;

    if (strcmp(argv[0], "wiegand") == 0)
      mode = MODE_WIEGAND;
    // if (strcmp(argv[0], "osdp") == 0)
    //   mode = MODE_OSDP;

    if (mode == MODE_NONE)
    {
      char buf[64];
      snprintf(buf, 64, "Err: Syntax Error");
      hmiPuts(buf, HMI_CLI);
      return;
    }

    if (oPort == 0)
    {
      char buf[64];
      snprintf(buf, 64, "Err: Port is bad");
      hmiPuts(buf, HMI_CLI);
    }

    if (oPin >= 5)
    {
      char buf[64];
      snprintf(buf, 64, "Err: Pin is bad");
      hmiPuts(buf, HMI_CLI);
    }

    switch (mode)
    {
    case MODE_WIEGAND:
      if (oPort > OSDP_MAX_WIEGAND_COUNT)
      {
        char buf[64];
        snprintf(buf, 64, "Err: Port too big");
        hmiPuts(buf, HMI_CLI);

        return;
      }
      break;
    }

    bool state = false;
    if (check_if_true(argc, argv, 3))
    //if ((argv[3][0] == '1') || (argv[3][0] == 't') || (argv[3][0] == 'T'))
    {
      state = true;
    }
    switch (mode)
    {
    case MODE_WIEGAND:
      switch (oPin)
      {
      case 0:
        break;
      case 1:
        break;
      case 2:
        if (state)
        {
          reader[oPort - 1].LED_perm.blink(1000, 0).start();
          reader[oPort - 1].LED_temp.blink(1000, 0).start();
        }
        else
        {
          reader[oPort - 1].LED_perm.trigger(reader[oPort - 1].LED_temp.EVT_OFF);
        }
        break;
      case 3:
        if (state)
        {
          reader[oPort - 1].buzzer.blink(1000, 0).start();
        }
        else
        {
          reader[oPort - 1].buzzer.trigger(reader[oPort - 1].buzzer.EVT_OFF);
        }
        break;
      case 4:
        break;
      }

      break;
    }
    return;
  }
  char buf[64];
  snprintf(buf, 64, "Err: Syntax Error");
  hmiPuts(buf, HMI_CLI);
}
