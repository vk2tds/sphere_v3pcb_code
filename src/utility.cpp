/* Copyright (c) 2023-2024 Radioactive Networks Pty Ltd */
/* All Rights Reserved                                  */
/* darryl@radio-active.net.au                           */

#include "main.h" // Needed for EEPROM_ADDRESS_SETTINGS

#include <FlashStorage_STM32.h>

#include "alarm_octopus.h"

//#include <Wire.h>
//#include <SerialRAM.h>


//SerialRAM ram;





void myhexdump(uint8_t *buffer, uint16_t len, char *desc)
{
  char buf[128];
  char buf2[32];
  snprintf (buf, 128, "HEXDUMP: %s", desc);
  hmiPuts(buf, HMI_TRACE);

  uint16_t offset = 0;

  for (uint16_t i = 0; i < len; i++)
  {
    if ((i%16) == 0)
    {
      snprintf (buf, 128, "| %04x | ", (i/16) * 16);
    }
    snprintf (buf2, 32, "%02x ", buffer[i]);
    safe_strcat (buf, buf2, 128);
    if ((i%16) == 15)
    {
      hmiPuts (buf, HMI_TRACE);
      buf[0] = 0;
    }
  }
  if (buf[0] != 0) 
  {
      hmiPuts (buf, HMI_TRACE);
  }
}

void utilityLoop(void)
{

  //ram.begin();


  while (1==1)
  {

  uint8_t buffer = 0x00;
  uint8_t randomByte = random(256);
  uint16_t randomAddress = random(0x0200);
  
  

  //ram.write(randomAddress, randomByte);
  //buffer = ram.read(randomAddress);

  Serial.print("Wrote byte: 0x");
  Serial.print(randomByte, HEX);
  Serial.print(" at address 0x");
  Serial.print(randomAddress, HEX);
  Serial.print(" - Read back value: 0x");
  Serial.print(buffer, HEX);

  if(randomByte == buffer){
    Serial.println(" - OK!");
  }
  else{
    Serial.println(" ERROR! Values do not match! Check your pullup resistors and wiring");
  }
  
  delay(1000);

  }
}








// From alarm_octopus
extern Flash settings;
extern boolean pauseSave;

int safe_strcat(char *s1, char *s2, size_t s1_size)
{
  if (strlen(s1) + strlen(s2) + 1 > s1_size)
  {
    strncat(s1, s2, s1_size - 1 - strlen(s1));
    return 0;
  }
  strcat(s1, s2);
  return 1;
}

unsigned long eeprom_crc(int start, int length)
{
  // From https://github.com/khoih-prog/FlashStorage_STM32/blob/main/examples/EEPROM_CRC/EEPROM_CRC.ino
  const unsigned long crc_table[16] =
      {
          0x00000000, 0x1db71064, 0x3b6e20c8, 0x26d930ac,
          0x76dc4190, 0x6b6b51f4, 0x4db26158, 0x5005713c,
          0xedb88320, 0xf00f9344, 0xd6d6a3e8, 0xcb61b38c,
          0x9b64c2b0, 0x86d3d2d4, 0xa00ae278, 0xbdbdf21c};

  unsigned long crc = ~0L;

  for (int index = start; index < length; ++index)
  {
    crc = crc_table[(crc ^ EEPROM.read(index)) & 0x0f] ^ (crc >> 4);
    crc = crc_table[(crc ^ (EEPROM.read(index) >> 4)) & 0x0f] ^ (crc >> 4);
    crc = ~crc;
  }

  return crc;
}

unsigned long uid(void)
{
  // Hardware Serial Number
  // From https://github.com/khoih-prog/FlashStorage_STM32/blob/main/examples/EEPROM_CRC/EEPROM_CRC.ino
  const unsigned long crc_table[16] =
      {
          0x00000000, 0x1db71064, 0x3b6e20c8, 0x26d930ac,
          0x76dc4190, 0x6b6b51f4, 0x4db26158, 0x5005713c,
          0xedb88320, 0xf00f9344, 0xd6d6a3e8, 0xcb61b38c,
          0x9b64c2b0, 0x86d3d2d4, 0xa00ae278, 0xbdbdf21c};

  unsigned long crc = ~0L;

  for (int index = 0; index < UniqueIDsize; ++index)
  {
    crc = crc_table[(crc ^ UniqueID[index]) & 0x0f] ^ (crc >> 4);
    crc = crc_table[(crc ^ (UniqueID[index] >> 4)) & 0x0f] ^ (crc >> 4);
    crc = ~crc;
  }

  return crc;
}

unsigned long string_crc(char str[])
{
  // From https://github.com/khoih-prog/FlashStorage_STM32/blob/main/examples/EEPROM_CRC/EEPROM_CRC.ino
  const unsigned long crc_table[16] =
      {
          0x00000000, 0x1db71064, 0x3b6e20c8, 0x26d930ac,
          0x76dc4190, 0x6b6b51f4, 0x4db26158, 0x5005713c,
          0xedb88320, 0xf00f9344, 0xd6d6a3e8, 0xcb61b38c,
          0x9b64c2b0, 0x86d3d2d4, 0xa00ae278, 0xbdbdf21c};

  unsigned long crc = ~0L;

  for (int index = 0; index < strlen(str); ++index)
  {
    crc = crc_table[(crc ^ str[index]) & 0x0f] ^ (crc >> 4);
    crc = crc_table[(crc ^ (str[index] >> 4)) & 0x0f] ^ (crc >> 4);
    crc = ~crc;
  }

  return crc;
}

void print_eeprom(void)
{
  char buf[64];

  snprintf(buf, 64, "INIT: Start Flash on %s %s", BOARD_NAME, FLASH_STORAGE_STM32_VERSION);
  hmiPuts(buf, HMI_CONFIG);

  snprintf(buf, 64, "INIT: EEPROM lenth: %d", EEPROM.length());
  hmiPuts(buf, HMI_CONFIG);
}

void settings_save(void)
{
  unsigned long signature;
  uint8_t *settingsPtr = (uint8_t *)&settings;

  if (pauseSave)
  {
    char buf[96];
    snprintf(buf, 96, "EPROM settings NOT saved. Saving is paused. Type 'pause' to resume saving");
    hmiPuts(buf, HMI_CONFIG);
    return;
  }

  EEPROM.put(EEPROM_ADDRESS_SETTINGS, settings);                         // Save then update the CRC because CRC works on EPROM only
  signature = eeprom_crc(EEPROM_ADDRESS_SETTINGS, sizeof(settings) - 4); // Hopefully this is correct and doesn't include the signature.
  settings.signature = signature;
  EEPROM.put(EEPROM_ADDRESS_SETTINGS, settings);
  char buf[64];
  snprintf(buf, 64, "EPROM settings saved");
  hmiPuts(buf, HMI_CONFIG);
}

void settings_save_restart(bool new_mode_cp)
{
  unsigned long signature;
  Flash settings2;

  memcpy (&settings2, &settings, sizeof (settings));
  settings2.mode_cp = new_mode_cp;

  //uint8_t *settingsPtr = (uint8_t *)&settings2;

  EEPROM.put(EEPROM_ADDRESS_SETTINGS, settings2);                         // Save then update the CRC because CRC works on EPROM only
  signature = eeprom_crc(EEPROM_ADDRESS_SETTINGS, sizeof(settings2) - 4); // Hopefully this is correct and doesn't include the signature.
  settings.signature = signature;
  EEPROM.put(EEPROM_ADDRESS_SETTINGS, settings2);
  char buf[64];
  snprintf(buf, 64, "EPROM settings saved");
  hmiPuts(buf, HMI_CONFIG);
}



void settings_destroy(void)
{
  settings.signature = 0;
  EEPROM.put(EEPROM_ADDRESS_SETTINGS, settings);
  char buf[64];
  snprintf(buf, 64, "EPROM settings invalidated");
  hmiPuts(buf, HMI_CONFIG);
}

void settings_load(void)
{

  unsigned long signature;

  EEPROM.get(EEPROM_ADDRESS_SETTINGS, settings);

  signature = eeprom_crc(EEPROM_ADDRESS_SETTINGS, sizeof(settings) - 4); // Hopefully this is correct and doesn't include the signature.
  settings.base_serial_no = uid();

  if (signature == settings.signature)
  {
    char buf[64];
    snprintf(buf, 64, "INIT: Settings Loaded");
    hmiPuts(buf, HMI_CONFIG);
    //settings.mode_cp = false;

    if (settings.flash_LED_on_serial == 255)
    {
      settings.flash_LED_on_serial = 1;
    }
  }
  else
  {
    settings_default();
  }

  settings_override(); // Override settings. For debugging
  settings_load_part2(); // These are specific to the product
}
