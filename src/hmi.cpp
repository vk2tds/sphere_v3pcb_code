/* Copyright (c) 2023-2024 Radioactive Networks Pty Ltd */
/* All Rights Reserved                                  */
/* darryl@radio-active.net.au                           */

#include "main.h"

extern Flash settings;

uint16_t password_timeout = 0; // Stores time until console locks. 8 per minute. Counts down. 0 = Locked.

extern ParseCommands pCmd;
extern Status statusFactory;
extern Status statusLive;

char CommandLine[COMMAND_BUFFER_LENGTH + 1]; // Read  commands into this buffer from Serial.  +1 in length for a termination char

bool haveUsedHmiPuts = false; // If we have used hmiPuts in this iteration.

// Print the command line
void printCommand(void)
{
    char buf[64];
    snprintf(buf, 64, "cmd >%s", CommandLine);
    hmiPuts(buf, HMI_CLI);
}

boolean checkPassword(void)
{
    if ((password_timeout == 0) && (settings.protection != 0))
    {
        char buf[96];
        snprintf(buf, 96, "Console locked. Type 'password' followed by the password to unlock");
        hmiPuts(buf, HMI_CLI);
        return false;
    }
    else
    {
        // char buf[64];
        // snprintf (buf, 64, "Timeout %d protection %d", password_timeout, settings.protection);
        // hmiPuts(buf, HMI_CLI);
        password_timeout = PASSWORDTIMEOUT;

        return true;
    }
}

// hmiPuts is normally used for libosdp logging.. and used to sync when messages come in.
int hmiPuts(const char *str, uint8_t mode)
{
    switch (mode)
    {
    case HMI_ALWAYS:
        break;
    case HMI_CLI:
        break;
    case HMI_CONFIG:
        break;
    case HMI_STATUS:
        if (
            ((password_timeout == 0) && (settings.protection != 0)) && 
            (settings.protect != 0)
           )
            return 0;
        break;
    case HMI_TRACE:
        break;
    }

    if (!haveUsedHmiPuts)
    {
        for (uint16_t i = 0; i < strlen(CommandLine) + 5; i++)
        {
            Serial.print("\b");
        }
        for (uint16_t i = 0; i < strlen(CommandLine) + 5; i++)
        {
            Serial.print(" ");
        }
        for (uint16_t i = 0; i < strlen(CommandLine) + 5; i++)
        {
            Serial.print("\b");
        }
    }
    bool found = false; 
    for (uint16_t i = 0; i < strlen(str); i++)
    {
        if (str[i] < 0x20)
        {
            if (!found)
            {
                Serial.println ("");
            }
            found = true;
        } else {
            Serial.print (str[i]);
            found = false;
        }
    }
    if (!found)
    {
        Serial.println ("");
    }

    // if ((strchr(str, '\r') == NULL) || (strchr(str, '\n') == NULL))
    // { // If we already have crlf dont do it a second time
    //     Serial.println(str);
    // }
    // else
    // {
    //     Serial.print(str);
    // }
    haveUsedHmiPuts = true;

    // do we need phy OSDP logged? Dont think so.
    if (strstr(str, "SC Active with SCBK-D"))
    {
        // PD lost the packet and the CP recovered
        statusLive.osdp_lost_cp++;
        statusFactory.osdp_lost_cp++;
    }
    if (strstr(str, "received a sequence repeat packet"))
    {
        // CP lost the packet and they sent it again
        statusLive.osdp_lost_pd++;
        statusFactory.osdp_lost_pd++;
    }

    return 0;
}

int hmiPutsTrace(const char *str)
{
    return hmiPuts(str, HMI_TRACE);
}

void hmiPrintCommandPrompt(void)
{
    if (haveUsedHmiPuts)
    {
        haveUsedHmiPuts = false;
        Serial.print("cmd >");
        for (uint8_t pos = 0; pos < COMMAND_BUFFER_LENGTH; pos++)
        {
            if (CommandLine[pos] == 0x00)
                break;
            if ((CommandLine[pos] != '\r') && (CommandLine[pos] != '\n'))
            { // Dont print CRLF
                Serial.write(CommandLine[pos]);
            }
        }
    }
}

uint8_t charLast = '\0';
uint8_t charBeforeThat = '\0';

void hmi(void)
{
    bool received = getCommandLineFromSerialPort(&CommandLine[0]); // global CommandLine is defined in hmi.h
    int16_t err = true;

    int16_t err_1;
    int16_t err_2;
    if (received)
    {
        for (uint8_t pos = 0; pos < COMMAND_BUFFER_LENGTH; pos++)
        {
            if (CommandLine[pos] == 0x00)
                break;
            err = pCmd.read(CommandLine[pos]);
        }

        err = false;
        err_1 = pCmd.read('\r');
        if (!err_1)
        {
            err = pCmd.getError();
            // Serial.println (err);
        }
        err_2 = pCmd.read('\n');
        if (!err_2)
        {
            err = pCmd.getError();
            // Serial.println (err);
        }

        if (
            ((charLast == '\r') && (charBeforeThat == '\n')) ||
            ((charLast == '\n') && (charBeforeThat == '\r')))
        {
            // Strangely, we can ignire if there is a CR/LF or LF/CR, becasue we see CR __OR__ LF as
            // the end of line, and have aready dealt with it...
            charLast = ' ';
            charBeforeThat = ' ';
            return;
        }

        switch (err)
        {
        case -5:
        case -6:
            char buf[64];
            snprintf(buf, 64, "cmd >%s", CommandLine);
            hmiPuts(buf, HMI_CLI);

            hmiPuts("Eh?", HMI_CLI);
            break;
        case 0:
            // Serial.print ("cmd >");
            // Serial.println (CommandLine);
            break;
        default:
            Serial.println("");
            break;
        }

        CommandLine[0] = 0;

        haveUsedHmiPuts = true;
        hmiPrintCommandPrompt();
    }
}

uint8_t parseInt(char *arg)
{
    char *ptr;
    long ret;
    if ((arg[0] >= '0') & (arg[0] <= '9'))
    {
        ret = strtoimax(arg, &ptr, 10);
        return (ret);
    }
    else
    {
        return -1;
    }
}

uint16_t parse16Int(char *arg)
{
    char *ptr;
    long ret;
    if ((arg[0] >= '0') & (arg[0] <= '9'))
    {
        ret = strtoimax(arg, &ptr, 10);
        return (ret);
    }
    else
    {
        return -1;
    }
}

uint32_t parse32Int(char *arg)
{
    char *ptr;
    long ret;
    if ((arg[0] >= '0') & (arg[0] <= '9'))
    {
        ret = strtoimax(arg, &ptr, 10);
        return (ret);
    }
    else
    {
        return -1;
    }
}

/*************************************************************************************************************
    getCommandLineFromSerialPort()
      Return the string of the next command.  Commands are delimited by return"
      Handle BackSpace character
      Make  all chars lowercase
*************************************************************************************************************/

bool getCommandLineFromSerialPort(char *cmdLine)
{
    static uint8_t charsRead = 0; // note:  COMAND_BUFFER_LENGTH must be less than 255 chars long
    // read asynchronously  until full command input
    while (Serial.available())
    {
        char c = Serial.read();
        charBeforeThat = charLast;
        charLast = c;
        switch (c)
        {
        case '\n':
        case '\r':                     // likely have full command in buffer now,  commands are terminated by CR and/or LS
            cmdLine[charsRead] = '\0'; // null terminate our command char array
            if (charsRead > 0)
            {
                charsRead = 0; // charsRead is static,  so have to reset
                return true;
            }
            return true;
            break;
        case 0x7f:
        case '\b': //  handle backspace in input: put a space in last char
            if (charsRead > 0)
            { // and adjust commandLine and charsRead
                cmdLine[--charsRead] = '\0';
                Serial.print("\b \b"); // no idea  how this works, found it on the Internet
            }
            break;
        default:
            // c = tolower(c);
            if (charsRead < COMMAND_BUFFER_LENGTH)
            {
                cmdLine[charsRead++] = c;
            }
            Serial.print(c);
            cmdLine[charsRead] = '\0'; // just in case
            break;
        }
    }
    return false;
}
