/* Copyright (c) 2023-2024 Radioactive Networks Pty Ltd */
/* All Rights Reserved                                  */
/* darryl@radio-active.net.au                           */

#include "everything.h"
#include "defines.h"
#include "hmi.h"

#include <ParseCommands.h>

extern ParseCommands pCmd;


extern struct Hardware h[];
extern uint8_t h_elementcount;

extern struct ADCstorage adcstorage [MAX_ADC]; 


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
//     reporting frequency
//
// port... what data goes where
// port {com1|com2|com3|usb|ethernet} {cli|mqtt|slave|hvac}
//     serial speeds too etc
//

//
// led... Is this flash mode?
//
// power {12v|48v|supply|fans|lights} [CircuitNumber] {0|1|On|Off|flash|toggle with time}
// 
// serial - display serial number/MAC/firmware/compile date etc
//
// trace - see messages coming in on each port
//
// reboot period automatic

// MQTT
//
// Send serial number, software version, software date etc
// Send messages on change of state of objects
// Commands to do stuff, like restart hardware, 









// Todo: Need to move these to setup() somehow. Not sure how :(
struct pcmd_command_t commandList[] = {
    // command, callback function
    "help", CmdHelp,
    "reset", CmdReset,
    "restart", CmdReset,
    "info", CmdInfo,
    "FACTORY", CmdFactory,
    "stats", CmdStats,
    "door", CmdDoor,                // Done
    "temp", CmdTemp,                // Done
    "power", CmdPower,
    "ip", CmdIp,
    "mqtt", CmdMqtt,
    "port", CmdPort,
    "fan", CmdFan,
    "led", CmdLed,
    "serial", CmdSerial,
    "trace", Cmdtrace,
    "hardware", CmdHardware,
    "speed", CmdSpeed,
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




void infoDoor (void)
{
  char buf[64];
  snprintf(buf, 64, "door [Door Number] {0|lock|1|unlock|open}");
  hmiPuts(buf, HMI_CLI);

  for (uint8_t i = 0; i < MAX_DOORS; i++){
    switch (doorstorage[i].DoorState){
        case (false):
            snprintf (buf, 64, "door %d close", d+1);
            hmiPuts(buf, HMI_CLI);
            break;
        case (true):
            snprintf (buf, 64, "door %d open", d+1);
            hmiPuts(buf, HMI_CLI);
            break;
    }
  }
}


void CmdDoor(int argc, char *argv[])
{
    if (argc == 0)
    {
        infoDoor();
        return;
    }

    if (argc < 2){
        char buf[64];
        snprintf(buf, 64, "Err: Syntax Error");
        hmiPuts(buf, HMI_CLI);
        return;
    }

    uint8_t dPort = parseInt(argv[0]);
    uint8_t dooraction = parseInt(argv[1]); // parse 0 or 1

    if ((dPort > MAX_DOORS) || (dPort == 0)){
        char buf[64];
        snprintf(buf, 64, "Err: Invalid door value");
        hmiPuts(buf, HMI_CLI);
        return;
    }

    if ((argv[1][0] == 'u') || (argv[1][0] == 'U')){ // Unlock
        dooraction = 1;
    }

    if ((argv[1][0] == 'o') || (argv[1][0] == 'O')){ // Open - Not used at the moment with these locks
        dooraction = 2;
    }
    door (dPort-1, dooraction);

}






void infoTemp (void)
{
    char buf[64];
    snprintf(buf, 64, "temp [temp string] {temperature} {age}");
    hmiPuts(buf, HMI_CLI);

    for (uint8_t i = 0; i < MAX_TEMP; i++){
        // TODO: Printing floats
        uint32_t diff;
        diff = secondsSinceStart - tempstorage[i].secondsSinceStart;
        snprintf ("temp %d %2.1f %lds", i, tenpstorage[i].temperature, diff);
    }
}


void CmdTemp(int argc, char *argv[])
{
    if (argc == 0)
    {
        infoTemp();
        return;
    }

    char buf[64];
    snprintf(buf, 64, "Err: Additional Temp Settings Not Implemented");
    hmiPuts(buf, HMI_CLI);
    return;

}


void infoPower (void)
{
  char buf[128];
  snprintf(buf, 128, "power {12v|48v|supply|fans|lights} [CircuitNumber] {0|1|On|Off|flash|toggle with time} [On Time]");
  hmiPuts(buf, HMI_CLI);
  snprintf(buf, 128, "  - WARNING: Output SUPPLY/0 is INVERTED. ON turns the output OFF");
  hmiPuts(buf, HMI_CLI);

  char buf2[16];
  char buf3[16];

  char amp_buf[16];
  char volt_buf[16];


  for (uint8_t i = 0; i < h_elementcount; i++){
      snprintf (buf2, 64,   "ERROR  ");
      
      if (h[i].mode == voltage_12){
        snprintf (buf2, 64, "12v    ");
      }
      if (h[i].mode == voltage_48){
        snprintf (buf2, 64, "48v    ");
      }
      if (h[i].mode == voltage_supply){
        snprintf (buf2, 64, "supply ");
      }
      if (h[i].mode == voltage_fans){
        snprintf (buf2, 64, "fans   ");
      }
      if (h[i].mode == voltage_lights){
        snprintf (buf2, 64, "lights ");
      }

      snprintf (buf3, 16, "ALWAYS ON ");
      if (h[i].power_pin != NO_PIN){
      if (digitalRead (h[i].power_pin)){
        snprintf (buf3, 16, "on       ");
      } else {
        snprintf (buf3, 16, "off      ");
      }

      snprintf (amp_buf, 16, "       ");
      snprintf (volt_buf, 16, "       ");

      if (h[i].current_adc_address != INVALID_ADC_ADDRESS){
        snprintf (amp_buf, 16, "%2.3fA", adcstorage[h[i].current_adc_address]);
      }

      if (h[i].voltage_adc_address != INVALID_ADC_ADDRESS){
        snprintf (amp_buf, 16, "%2.2f V", adcstorage[h[i].current_adc_address]);
      }

      snprintf (buf, 128, "%s %02d %s %s %s", buf2, h[i].index, buf3, amp_buf, volt_buf);
      hmiPuts(buf, HMI_CLI);
  }
}


//  char *command_line = "power {12v|48v|supply|fans|lights} [CircuitNumber] {0|1|On|Off|flash|toggle with time} [On Time]";


void CmdPower(int argc, char *argv[])
{
    if (argc == 0)
    {
        infoPower();
        return;
    }

    if (argc < 3){
        char buf[64];
        snprintf(buf, 64, "Err: Syntax Error");
        hmiPuts(buf, HMI_CLI);
        return;
    }

    uint8_t mode;

    if (strcasecmp (argv[1], "12v") == 0){
      mode = voltage_12;
    }
    if (strcasecmp (argv[1], "48v") == 0){
      mode = voltage_48;
    }
    if (strcasecmp (argv[1], "supply") == 0){
      mode = voltage_supply;
    }
    if (strcasecmp (argv[1], "fans") == 0){
      mode = voltage_fans;
    }
    if (strcasecmp (argv[1], "lights") == 0){
      mode = voltage_lights;
    }

    uint8_t circuit = parseInt (argv[2]);

    uint8_t state = parseInt (argv[3]); // Picks up 1 & 0

    if (strcasecmp (argv[3], "on") == 0){
      state = 1;
    }

    if (strcasecmp (argv[3], "pulse") == 0){
      state = 2;
    }

    if (strcasecmp (argv[3], "flash") == 0){
      state = 3;
    }

    if (state >1){
      if (argc < 3){
        char buf[64];
        snprintf(buf, 64, "Err: Syntax Error");
        hmiPuts(buf, HMI_CLI);
        return;
      }
    }

    for (uint8_t i = 0; i < h_elementcount; i++){
      if ((h[i].mode == mode) && (h[i].index == circuit)){
        if (state == 0) digitalWrite (h[i].power_pin, false);
        if (state == 1) digitalWrite (h[i].power_pin, true);
        if (state > 1){
          char buf[64];
          snprintf(buf, 64, "Err: Output not found");
          hmiPuts(buf, HMI_CLI);
          return;
        }
        return;
      }
    }
}

















// -------------------------------------------------


















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
      // settings.cp_buz_temp_time_on[wPort-1][highlow] =  on_time;
      // settings.cp_buz_temp_time_off[wPort-1][highlow] = off_time;
      // settings.cp_buz_temp_count[wPort-1][highlow] = count;

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
        // settings.cp_led_perm_time_on[wPort-1][highlow] =  on_time;
        // settings.cp_led_perm_time_off[wPort-1][highlow] = off_time;
        // settings.cp_led_perm_color_on[wPort-1][highlow] =  on_color;
        // settings.cp_led_perm_color_off[wPort-1][highlow] = off_color;
        // settings_save();
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
        // settings.cp_led_temp_time_on[wPort-1][highlow] =  on_time;
        // settings.cp_led_temp_time_off[wPort-1][highlow] = off_time;
        // settings.cp_led_temp_color_on[wPort-1][highlow] =  on_color;
        // settings.cp_led_temp_color_off[wPort-1][highlow] = off_color;
        // settings.cp_led_temp_count[wPort-1][highlow] = count;
        // settings_save();
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
      // settings.cp_osdp_address[oPort-1] = address;
      // settings_save();
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

// void infoHardware()
// {
//   char buf[64];
//   char buf2[64];
//   snprintf(buf, 64, "Hardware Status");
//   hmiPuts(buf, HMI_CLI);

//   for (uint8_t i = 0; i < OSDP_MAX_WIEGAND_COUNT; i++)
//   {
//     if (pins[i][0] != NONE)
//     {
//       snprintf(buf, 64, "wiegand %02d %d %d %d %d %d", i + 1,
//                digitalRead(pins[i][0]),
//                digitalRead(pins[i][1]),
//                digitalRead(pins[i][2]),
//                digitalRead(pins[i][3]),
//                digitalRead(pins[i][4]));
//       hmiPuts(buf, HMI_CLI);
//     }
//     else
//     {
//       snprintf(buf, 64, "wiegand %02d - - - - -", i + 1);
//       hmiPuts(buf, HMI_CLI);
//     }
//   }
// }

// void CmdHardware(int argc, char *argv[])
// {
//   // hardware wiegand port pin state
//   //     -1      0      1   2    3
//   printCommand();
//   if (!checkPassword())
//     return; // Console locked

//   // hardware osdp 1 2 1
//   // hardware oddp 1 3 1

//   if (argc == 0)
//   {
//     infoHardware();
//     return;
//   }

//   if (argc >= 3)
//   {
//     uint8_t oPort = parseInt(argv[1]);
//     uint8_t oPin = parseInt(argv[2]);
//     uint8_t mode = MODE_NONE;

//     if (strcmp(argv[0], "wiegand") == 0)
//       mode = MODE_WIEGAND;
//     // if (strcmp(argv[0], "osdp") == 0)
//     //   mode = MODE_OSDP;

//     if (mode == MODE_NONE)
//     {
//       char buf[64];
//       snprintf(buf, 64, "Err: Syntax Error");
//       hmiPuts(buf, HMI_CLI);
//       return;
//     }

//     if (oPort == 0)
//     {
//       char buf[64];
//       snprintf(buf, 64, "Err: Port is bad");
//       hmiPuts(buf, HMI_CLI);
//     }

//     if (oPin >= 5)
//     {
//       char buf[64];
//       snprintf(buf, 64, "Err: Pin is bad");
//       hmiPuts(buf, HMI_CLI);
//     }

//     switch (mode)
//     {
//     case MODE_WIEGAND:
//       if (oPort > OSDP_MAX_WIEGAND_COUNT)
//       {
//         char buf[64];
//         snprintf(buf, 64, "Err: Port too big");
//         hmiPuts(buf, HMI_CLI);

//         return;
//       }
//       break;
//     }

//     bool state = false;
//     if (check_if_true(argc, argv, 3))
//     //if ((argv[3][0] == '1') || (argv[3][0] == 't') || (argv[3][0] == 'T'))
//     {
//       state = true;
//     }
//     switch (mode)
//     {
//     case MODE_WIEGAND:
//       switch (oPin)
//       {
//       case 0:
//         break;
//       case 1:
//         break;
//       case 2:
//         if (state)
//         {
//           reader[oPort - 1].LED_perm.blink(1000, 0).start();
//           reader[oPort - 1].LED_temp.blink(1000, 0).start();
//         }
//         else
//         {
//           reader[oPort - 1].LED_perm.trigger(reader[oPort - 1].LED_temp.EVT_OFF);
//         }
//         break;
//       case 3:
//         if (state)
//         {
//           reader[oPort - 1].buzzer.blink(1000, 0).start();
//         }
//         else
//         {
//           reader[oPort - 1].buzzer.trigger(reader[oPort - 1].buzzer.EVT_OFF);
//         }
//         break;
//       case 4:
//         break;
//       }

//       break;
//     }
//     return;
//   }
//   char buf[64];
//   snprintf(buf, 64, "Err: Syntax Error");
//   hmiPuts(buf, HMI_CLI);
// }






// Commands
//
//
// ip...
//     DHCP, IP, Mask, Gateway, DNS1/2
//
// mqtt...
//     host, port, encrypt, usernamem password, which ports, {primary|secondary|round robin}, subscriptions
//     CLI over MQTT
//     reporting frequency
//
// port... what data goes where
// port {com1|com2|com3|usb|ethernet} {cli|mqtt|slave|hvac}
//     serial speeds too etc
//
//
// led... Is this flash mode?
//
// 
// serial - display serial number/MAC/firmware/compile date etc
//
// trace - see messages coming in on each port
//
// reboot period automatic

// MQTT
//
// Send serial number, software version, software date etc
// Send messages on change of state of objects
// Commands to do stuff, like restart hardware, 
//
// Amps and Voltage????



#define HELP_VALUE_DOOR 1
#define HELP_VALUE_TEMP 2




void printhelp(char *command, char *text, uint16_t values, uint8_t warning)
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

  if (values & HELP_VALUE_DOOR)
  {
    snprintf (buf2, 256, " - [Door Number] - 1-MAX_DOORS");
    hmiPuts(buf2, HMI_CLI);
  }
  if (values & HELP_VALUE_TEMP)
  {
    snprintf (buf2, 256, " - [Temperature Port] - 1-MAX_TEMP");
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

    if (strcasecmp ("door", argv[0]) == 0)
    {
      char *command_line = "door [Door] [DoorState]... ";
      char *command_help = "Unlock, Lock or Open a door";
      uint8_t warning = 0;
      uint16_t values = HELP_VALUE_DOOR;
      printhelp (command_line, command_help, values, warning);
      return;
    }

    if (strcasecmp ("temp", argv[0]) == 0)
    {
      char *command_line = "temp mode [Temp String]\r\n"
                            "temp alarm [Temp String] xxxxxxx";
      char *command_help = "Unlock, Lock or Open a door";
      uint8_t warning = 0;
      uint16_t values = HELP_VALUE_TEMP;
      printhelp (command_line, command_help, values, warning);
      return;
    }

    if (strcasecmp ("power", argv[0]) == 0)
    {
      char *command_line = "power {12v|48v|supply|fans|lights} [CircuitNumber] {0|1|On|Off|flash|toggle with time} [On Time]";
      char *command_help = "Turn on a circuit on and off";
      uint8_t warning = 0;
      uint16_t values = CIRCUIT_TYPE | CIRCUIT NUMBER | ON_TIME;
      printhelp (command_line, command_help, values, warning);
      return;
    }

    if (strcasecmp ("fan", argv[0]) == 0)
    {
      char *command_line = "fan [Fan Number] pwm [PWM Value]\r\n"
                           "fam {pwr|power} [Fan State]";
      char *command_help = "Turn on a circuit on and off";
      uint8_t warning = 0;
      uint16_t values = FAN_NUMBER | FAN_STATE | PWR_VALUE];
      printhelp (command_line, command_help, values, warning);
      return;
    }
    



// fan
// fan [FanNumber]
// fan [FanNumber] pwm [0-100]
// fan [FanNumber] {power|pwr} [1|0|on|off] - clone on power






    if (strcasecmp ("reset", argv[0]) == 0)
    {
      char *command_line = "reset";
      char *command_help = "This command simply causes the device to restart as if it was power cycled.";
      uint8_t warning = 1;
      uint16_t values = 0;
      printhelp (command_line, command_help, values, warning);
      return;
    }



    if (strcasecmp ("info", argv[0]) == 0)
    {
      char *command_line = "info";
      char *command_help = "Return the value of all settings.";
      uint8_t warning = 0;
      uint16_t values = 0;
      printhelp (command_line, command_help, values, warning);
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
    " ",
    "### Control Commands",
    "door         - Unlock, Lock or Open a door",
    "temp         - Temperature functions",
    "power        - Turn circuits on and off",
    "fan          - Control the cooling fans",
    "info         - display all settings",
    "stats        - display system statistics",
    "help         - display help on individual commands",
    "led          - change the meaning of the on board LED",
    "speed        - adjust the comms speed",
    " ",
    "\0"
  };

  for (uint8_t i = 0; i < 32; i++)
  {
    hmiPuts (cmds[i], HMI_CLI);
  }

}

