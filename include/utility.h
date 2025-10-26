
#ifndef UTILITY_H
#define UTILITY_H


#include <ArduinoUniqueID.h>

int safe_strcat(char *s1, char *s2, size_t s1_size);
unsigned long uid(void);
unsigned long eeprom_crc(int start, int length);
unsigned long string_crc(char str[]);
void print_eeprom (void);
void settings_save (void);
void settings_destroy (void);
void utilityLoop(void);
void myhexdump(uint8_t *buf, uint16_t len, char *desc);
void settings_save_restart(bool new_mode_cp);



#endif