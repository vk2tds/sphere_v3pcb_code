// -------------------------------------------------

// void infoCp_print_wiegand(uint8_t w_port, uint8_t state) {
//     char buf[96];
//     char buf2[64];
//     char buf3[32];

//     // CP WIEGAND [WIEGAND]   LED     [HIGH|LOW] TEMP  [ON TIME] [OFF TIME]
//     // [COUNT] [ON COLOR] [OFF COLOR] = 10 CP WIEGAND [WIEGAND]   BUZ [HIGH|LOW]
//     // TEMP  [ON TIME] [OFF TIME] [COUNT] = 8 CP WIEGAND [WIEGAND]   LED
//     // [HIGH|LOW] PERM  [ON TIME] [OFF TIME] [ON COLOR] [OFF COLOR] = 9 CP
//     // WIEGAND [WIEGAND]   BUZ     [HIGH|LOW] PERM  [ON TIME] [OFF TIME] = 7

//     if (state == 0) {
//         snprintf(buf3, 32, "CP WIEGAND %d LED LOW  ", w_port + 1);
//     } else {
//         snprintf(buf3, 32, "CP WIEGAND %d LED HIGH ", w_port + 1);
//     }

//     // Add decoded colors
//     snprintf(buf2, 64, "TEMP %03d %03d %d %d %d",
//              settings.cp_led_temp_time_on[w_port][state],
//              settings.cp_led_temp_time_off[w_port][state],
//              settings.cp_led_temp_count[w_port][state],
//              settings.cp_led_temp_color_on[w_port][state],
//              settings.cp_led_temp_color_off[w_port][state]);
//     buf[0] = 0;
//     safe_strcat(buf, buf3, 96);
//     safe_strcat(buf, buf2, 96);
//     hmiPuts(buf, HMI_CLI);

//     // Add decoded colors
//     snprintf(buf2, 64, "PERM %03d %03d %d %d",
//              settings.cp_led_perm_time_on[w_port][state],
//              settings.cp_led_perm_time_off[w_port][state],
//              settings.cp_led_perm_color_on[w_port][state],
//              settings.cp_led_perm_color_off[w_port][state]);
//     buf[0] = 0;
//     safe_strcat(buf, buf3, 96);
//     safe_strcat(buf, buf2, 96);
//     hmiPuts(buf, HMI_CLI);

//     if (state == 0) {
//         snprintf(buf3, 32, "CP WIEGAND %d BUZ LOW  ", w_port + 1);
//     } else {
//         snprintf(buf3, 32, "CP WIEGAND %d BUZ HIGH ", w_port + 1);
//     }

//     snprintf(buf2, 96, "TEMP %03d %03d %03d",
//              settings.cp_buz_temp_time_on[w_port][state],
//              settings.cp_buz_temp_time_off[w_port][state],
//              settings.cp_buz_temp_count[w_port][state]);
//     buf[0] = 0;
//     safe_strcat(buf, buf3, 96);
//     safe_strcat(buf, buf2, 96);
//     hmiPuts(buf, HMI_CLI);
// }

// void CmdCp(int argc, char* argv[]) {
//     // CP 0       1           2       3           4         5          6 7 8 CP
//     // WIEGAND [WIEGAND]   LED     [HIGH|LOW] TEMP  [ON TIME] [OFF TIME] [COUNT]
//     // [ON COLOR] [OFF COLOR] = 10 CP WIEGAND [WIEGAND]   BUZ     [HIGH|LOW]
//     // TEMP  [ON TIME] [OFF TIME] [COUNT] = 8 CP WIEGAND [WIEGAND]   LED
//     // [HIGH|LOW] PERM  [ON TIME] [OFF TIME] [ON COLOR] [OFF COLOR] = 9 NOT THIS
//     // ONE CP WIEGAND [WIEGAND]   BUZ     [HIGH|LOW] PERM  [ON TIME] [OFF TIME]
//     // = 7

//     // CP OSDP    [OSDP PORT] ADDRESS [OSDP ADDRESS] - All ArgC == 4
//     // CP OSDP    [OSDP PORT] WIEGAND [WIEGAND PORT]
//     // CP OSDP    [OSDP PORT] ENCRYPTION xxx
//     // CP OSDP    [OSDP PORT] SECURE [Y/N]
//     // CP OSDP    [OSDP PORT] INIT [Y/N] ???
//     // CP OSDP    [OSDP PORT] ENABLE [Y/N]

//     // CP MODE [Y/N]

//     char buf[64];

//     printCommand();

//     if (argc == 0) {
//         infoCp();
//         return;
//     }

//     // CP MODE
//     // Check to see if we want to change mode.

//     if (argc >= 2) {
//         // CP MODE TRUE or FALSE
//         if ((argv[0][0] == 'm') | (argv[0][0] == 'M')) {  // Mode
//             bool state = false;
//             if (check_if_true(argc, argv, 1))  // CP MODE TRUE
//             {
//                 state = true;
//             } else {
//                 state = false;
//             }

//             if (settings.mode_cp != state) {
//                 snprintf(buf, 64, "CLI: Changing Mode!!! Restarting!!!");
//                 hmiPuts(buf, HMI_CLI);
//                 // printf ("\r\nCLI: Changing Mode!!! Restarting Now!!!\r\n");
//                 delay(500);

//                 settings.mode_cp = state;
//                 settings_save();  //_restart(state); // special for
//                 printf("\r\nRestarting...\r\n");
//                 delay(500);

//                 if (settings.mode_cp) {
//                     snprintf(buf, 64, "Mode CP");
//                     hmiPuts(buf, HMI_CLI);
//                 } else {
//                     snprintf(buf, 64, "Mode PD");
//                     hmiPuts(buf, HMI_CLI);
//                 }

//                 printf("\r\nRestarting...\r\n");
//                 delay(500);
//                 CmdReset(0, NULL);
//             }
//             return;
//         }  // Mode
//     }

//     // The command is Too Short
//     if (argc < 3) {
//         char buf[64];
//         snprintf(buf, 64, "Err: Syntax Error");
//         hmiPuts(buf, HMI_CLI);
//         return;
//     }

//     // CP WIEGAND
//     if ((argv[0][0] == 'w') | (argv[0][0] == 'W')) {
//         if (argc < 6) {
//             char buf[64];
//             snprintf(buf, 64, "Err: Syntax Error");
//             hmiPuts(buf, HMI_CLI);
//             return;
//         }
//         uint8_t wPort = parseInt(argv[1]);  // CP WIEGAND n
//         uint8_t ledbuzrel = 0;              // LED Buz Relay
//         bool tempperm = true;
//         bool highlow = false;
//         uint8_t on_time = 0;
//         uint8_t off_time = 0;
//         uint8_t count = 0;
//         uint8_t on_color = 1;   // red
//         uint8_t off_color = 0;  // off

//         if ((wPort > OSDP_MAX_WIEGAND_COUNT)) {
//             char buf[64];
//             snprintf(buf, 64, "Err: Wiegand port is bad");
//             hmiPuts(buf, HMI_CLI);
//             return;
//         }

//         // CP WIEGAND n LED
//         if ((argv[2][0] == 'l') | (argv[2][0] == 'L')) {
//             // LED
//             ledbuzrel = 0;
//         } else if ((argv[2][0] == 'b') |
//                    (argv[2][0] == 'B'))  // CP WIEGAND n BUZ
//         {
//             // Buzzer
//             ledbuzrel = 1;
//         } else {  // // CP WIEGAND n RELAY
//             // Relay
//             ledbuzrel = 2;
//         }

//         // CP WIEGAND n LEDBUZ
//         // HIGH or LOW
//         if ((argv[3][0] == 'H') | (argv[3][0] == 'h')) {
//             // TEMP
//             highlow = true;
//         } else {
//             // PERM
//             highlow = false;
//         }

//         if ((argv[4][0] == 't') | (argv[4][0] == 'T')) {
//             // TEMP
//             tempperm = true;
//         } else {
//             // PERM
//             tempperm = false;
//         }

//         on_time = parseInt(argv[5]);
//         off_time = parseInt(argv[6]);

//         if (ledbuzrel == 1)  // Buzzer
//         {
//             if (argc < 8) {
//                 char buf[64];
//                 snprintf(buf, 64, "Err: Syntax Error < 8");
//                 hmiPuts(buf, HMI_CLI);
//                 return;
//             }
//             count = parseInt(argv[7]);
//             // settings.cp_buz_temp_time_on[wPort-1][highlow] =  on_time;
//             // settings.cp_buz_temp_time_off[wPort-1][highlow] = off_time;
//             // settings.cp_buz_temp_count[wPort-1][highlow] = count;

//             settings_save();
//             return;
//         } else {                    // LED
//             if (tempperm == false)  // Permament setting
//             {
//                 if (argc < 9) {
//                     char buf[64];
//                     snprintf(buf, 64, "Err: Syntax Error < 9");
//                     hmiPuts(buf, HMI_CLI);
//                     return;
//                 }
//                 on_color = parseInt(argv[7]);
//                 off_color = parseInt(argv[8]);
//                 // settings.cp_led_perm_time_on[wPort-1][highlow] =  on_time;
//                 // settings.cp_led_perm_time_off[wPort-1][highlow] = off_time;
//                 // settings.cp_led_perm_color_on[wPort-1][highlow] =  on_color;
//                 // settings.cp_led_perm_color_off[wPort-1][highlow] = off_color;
//                 // settings_save();
//                 return;
//             } else {  // Temporary Setting
//                 if (argc < 10) {
//                     char buf[64];
//                     snprintf(buf, 64, "Err: Syntax Error < 10");
//                     hmiPuts(buf, HMI_CLI);
//                     return;
//                 }
//                 count = parseInt(argv[7]);
//                 on_color = parseInt(argv[8]);
//                 off_color = parseInt(argv[9]);
//                 // settings.cp_led_temp_time_on[wPort-1][highlow] =  on_time;
//                 // settings.cp_led_temp_time_off[wPort-1][highlow] = off_time;
//                 // settings.cp_led_temp_color_on[wPort-1][highlow] =  on_color;
//                 // settings.cp_led_temp_color_off[wPort-1][highlow] = off_color;
//                 // settings.cp_led_temp_count[wPort-1][highlow] = count;
//                 // settings_save();
//                 return;
//             }

//             // CP WIEGAND [WIEGAND]   LED     [HIGH|LOW] TEMP  [ON TIME] [OFF
//             // TIME] [COUNT] [ON COLOR] [OFF COLOR] = 10 CP WIEGAND [WIEGAND]
//             // BUZ     [HIGH|LOW] TEMP  [ON TIME] [OFF TIME] [COUNT] = 8 CP
//             // WIEGAND [WIEGAND]   LED     [HIGH|LOW] PERM  [ON TIME] [OFF TIME]
//             // [ON COLOR] [OFF COLOR] = 9
//         }
//         char buf[64];
//         snprintf(buf, 64, "Err: Something went wrong");
//         hmiPuts(buf, HMI_CLI);

//         return;
//     }

//     // WIEGAND

//     // CP OSDP    [OSDP PORT] ADDRESS [OSDP ADDRESS]
//     // CP OSDP    [OSDP PORT] WIEGAND [WIEGAND PORT]
//     // CP OSDP    [OSDP PORT] ENCRYPTION xxx
//     // CP OSDP    [OSDP PORT] SECURE [Y/N]
//     // CP OSDP    [OSDP PORT] INIT [Y/N] ???
//     // CP OSDP    [OSDP PORT] ENABLE [Y/N]

//     if ((argv[0][0] == 'O') | (argv[0][0] == 'o')) {  // OSDP
//         if (argc < 3) {
//             char buf[64];
//             snprintf(buf, 64, "Err: Syntax Error");
//             hmiPuts(buf, HMI_CLI);
//             return;
//         }

//         uint8_t oPort = parseInt(argv[1]);
//         if ((oPort > OSDP_MAX_CP) | (oPort == 0)) {
//             char buf[64];
//             snprintf(buf, 64, "Err: OSDP port is bad");
//             hmiPuts(buf, HMI_CLI);
//             return;
//         }

//         if ((argv[2][0] == 'A') | (argv[2][0] == 'a'))  // Address
//         {
//             uint8_t address = parseInt(argv[3]);
//             // settings.cp_osdp_address[oPort-1] = address;
//             // settings_save();
//             return;
//         }  // Address
//         if ((argv[2][0] == 'W') | (argv[2][0] == 'w'))  // Wiegand Port
//         {
//             uint8_t wPort = parseInt(argv[3]);
//             if ((wPort > OSDP_MAX_WIEGAND_COUNT)) {
//                 char buf[64];
//                 snprintf(buf, 64, "Err: OSDP port is bad");
//                 hmiPuts(buf, HMI_CLI);
//                 return;
//             }
//             settings.cp_osdp_wiegand[oPort - 1] = wPort;
//             settings_save();
//             return;
//         }  // Wiegand Port
//         if ((strcmp(argv[2], "ENCRYPTION") == 0) |
//             (strcmp(argv[2], "encryption") == 0) |
//             (strcmp(argv[2], "ENCRYPT") == 0) |
//             (strcmp(argv[2], "encrypt") == 0))  // Encryption
//         {
//             uint8_t scbk_local[16];
//             if (strcmp(argv[3], "default") == 0) {
//                 for (uint8_t i = 0; i < 16; i++) {
//                     settings.cp_osdp_scbk[oPort - 1][i] = 0x30 + i;
//                 }
//                 settings_save();
//                 return;
//             }
//             if (strlen(argv[3]) != 32) {
//                 char buf[128];
//                 snprintf(buf, 128,
//                          "Err: Encryption Key in HEX must be exactly 32 "
//                          "characters or `default`. Sorry.");
//                 hmiPuts(buf, HMI_CLI);
//                 return;
//             }
//             for (int8_t i = 0; i < 32; i++) {
//                 if ((argv[3][i] < '0') |
//                     ((argv[3][i] > '9') & (argv[3][i] < 'A')) |
//                     ((argv[3][i] > 'Z') & (argv[3][i] < 'a')) |
//                     (argv[3][i] > 'z')) {
//                     char buf[64];
//                     snprintf(buf, 64, "Err: Invalid Characters");
//                     hmiPuts(buf, HMI_CLI);
//                     return;
//                 }
//             }
//             uint8_t index = 0;
//             uint8_t offset = 0;
//             for (int8_t i = 0; i < 16; i++) {
//                 scbk_local[i] = 0;
//                 offset = 4;
//                 if ((argv[3][index] >= '0') & (argv[3][index] <= '9')) {
//                     scbk_local[i] = scbk_local[i] |
//                                     (((argv[3][index] - '0') & 0x0F) << offset);
//                 } else {
//                     scbk_local[i] =
//                         scbk_local[i] |
//                         ((((argv[3][index] - 'A') & 0x07) + 10) << offset);
//                 }
//                 index++;

//                 offset = 0;
//                 if ((argv[3][index] >= '0') & (argv[3][index] <= '9')) {
//                     scbk_local[i] = scbk_local[i] |
//                                     (((argv[3][index] - '0') & 0x0F) << offset);
//                 } else {
//                     scbk_local[i] =
//                         scbk_local[i] |
//                         ((((argv[3][index] - 'A') & 0x07) + 10) << offset);
//                 }
//                 index++;
//             }
//             // Save encryption
//             for (uint8_t i = 0; i < 16; i++) {
//                 settings.cp_osdp_scbk[oPort - 1][i] = scbk_local[i];
//             }
//             settings_save();
//             return;
//         }  // Encryption

//         bool state = false;
//         if (check_if_true(argc, argv, 3))
//         // if ((argv[3][0] == '1') || (argv[3][0] == 't') || (argv[3][0] ==
//         // 'T'))
//         {
//             state = true;
//         }
//         if ((strcmp(argv[2], "SECURE") == 0) |
//             (strcmp(argv[2], "secure") == 0))  // Secure
//         {
//             settings.cp_osdp_secure[oPort - 1] = state;
//             settings_save();
//             return;
//         }
//         if ((strcmp(argv[2], "ENABLE") == 0) |
//             (strcmp(argv[2], "enable") == 0))  // Secure
//         {
//             settings.cp_osdp_enabled[oPort - 1] = state;
//             settings_save();
//             return;
//         }
//         if ((strcmp(argv[2], "INSTALL") == 0) |
//             (strcmp(argv[2], "install") == 0))  // Secure
//         {
//             settings.cp_osdp_install[oPort - 1] = state;
//             settings_save();
//             return;
//         }
//         return;
//     }  // OSDP
// }


void CmdShow(int argc, char* argv[]) {
    printCommand();
    if (!checkPassword()) return;  // Console locked

    char buf[64];
    char buf2[64];

    for (uint8_t i = 0; i < MAX_CARD_READS; i++) {
        uint8_t p = (i + nextCard) % MAX_CARD_READS;
        uint32_t now = millis();
        snprintf(buf, 32, "");
        if (lastCardReads[p].timestamp != 0) {
            snprintf(buf2, 64,
                     "Port:%d Card:", lastCardReads[p].wiegand_port + 1);
            safe_strcat(buf, buf2, 64);

            uint8_t bytes = (lastCardReads[p].card_bits + 7) / 8;
            for (uint8_t j = 0; j < bytes; j++) {
                snprintf(buf2, 64, "%x%x", lastCardReads[p].card[j] >> 4,
                         lastCardReads[p].card[j] & 0xF);
                safe_strcat(buf, buf2, 64);
            }
            snprintf(buf2, 64, " Bits:%d ", lastCardReads[p].card_bits);
            safe_strcat(buf, buf2, 64);

            if (now - lastCardReads[p].timestamp < 0) {
                snprintf(buf2, 64, "Ago:%d ",
                         (0xFFFF + now - lastCardReads[p].timestamp) / 1000);
                safe_strcat(buf, buf2, 64);
            } else {
                snprintf(buf2, 64, "Ago:%d ",
                         (now - lastCardReads[p].timestamp) / 1000);
                safe_strcat(buf, buf2, 64);
            }
            hmiPuts(buf, HMI_STATUS);
        }
    }
}

void CmdRoute(int argc, char* argv[]) {
    printCommand();

    if (!checkPassword()) return;  // Console locked

    if (argc == 0) {
        infoRoute();
        return;
    }
    if (argc >= 2) {
        uint8_t oPort = parseInt(argv[0]);
        uint8_t oOSDP = parseInt(argv[1]);
        if ((oPort >= 0) & (oOSDP >= 0)) {
            if ((oPort == 0) || (oPort > OSDP_MAX_WIEGAND_COUNT)) {
                char buf[64];
                snprintf(buf, 64, "Err: Wiegand Port is bad");
                hmiPuts(buf, HMI_CLI);
                return;
            }
            if ((oOSDP == 0) || (oOSDP > OSDP_MAX_OSDP_COUNT)) {
                char buf[64];
                snprintf(buf, 64, "Err: OSDP Port is bad");
                hmiPuts(buf, HMI_CLI);
                return;
            }

            char buf[64];
            snprintf(
                buf, 64,
                "Routing Wiegand port %d from OSDP port %d to OSDP port %d",
                oPort, settings.wiegand_uplink_phy[oPort - 1] + 1, oOSDP);
            hmiPuts(buf, HMI_CLI);

            settings.wiegand_uplink_phy[oPort - 1] = oOSDP - 1;
            settings_save();
            return;
        }
    }
    char buf[64];
    snprintf(buf, 64, "Err: Syntax Error");
    hmiPuts(buf, HMI_CLI);
}

void CmdAddress(int argc, char* argv[]) {
    printCommand();
    if (!checkPassword()) return;  // Console locked

    infoAddress();
}

void infoAddress(void) {
    if (settings.mode_cp == false) {
        // PD Mode
        for (uint8_t i = 0; i < OSDP_MAX_OSDP_COUNT; i++) {
            if (powerup_enable_osdp[i]) {
                char buf[64];
                snprintf(buf, 64, "Address OSDP Port %d: %04x ", i + 1,
                         (settings.base_serial_no & 0xFFF0) + i);
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
//           reader[oPort - 1].LED_perm.trigger(reader[oPort -
//           1].LED_temp.EVT_OFF);
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

