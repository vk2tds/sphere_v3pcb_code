/* Copyright (c) 2023-2024 Radioactive Networks Pty Ltd */
/* All Rights Reserved                                  */
/* darryl@radio-active.net.au                           */

#include <ParseCommands.h>

#include "hmi.h"
#include "defines.h"


uint16_t password_timeout = 0; // Stores time until console locks. 8 per minute. Counts down. 0 = Locked.

extern ParseCommands pCmd;

extern struct PortInformation portinformation[];
extern uint8_t portinformation_elementcount;

//char CommandLine[COMMAND_BUFFER_LENGTH + 1]; // Read  commands into this buffer from Serial.  +1 in length for a termination char

//bool haveUsedHmiPuts = false; // If we have used hmiPuts in this iteration.

// Print the command line
void printCommand(uint8_t port)
{
    PortInformation &pi = portinformation[port];
    char buf[BUFFER_SIZE_RX];
    snprintf(buf, BUFFER_SIZE_RX, "cmd >%s", pi.RxBuffer);
    hmiPuts(port, buf, HMI_CLI);
}




int hmiPuts(uint8_t port, char *str, uint8_t mode)
{
    PortInformation &pi = portinformation[port];
    switch (mode)
    {
    case HMI_ALWAYS:
        break;
    case HMI_CLI:
        break;
    case HMI_CONFIG:
        break;
    case HMI_STATUS:
        break;
    case HMI_TRACE:
        break;
    }

    if (! pi.haveUsedHMIputs)
    {
        for (uint16_t i = 0; i < strlen((const char *)pi.RxBuffer) + 5; i++)
        {
            pi.s.print("\b");
        }
        for (uint16_t i = 0; i < strlen((const char *)pi.RxBuffer) + 5; i++)
        {
            pi.s.print(" ");
        }
        for (uint16_t i = 0; i < strlen((const char *)pi.RxBuffer) + 5; i++)
        {
            pi.s.print("\b");
        }
    }
    bool found = false; 
    for (uint16_t i = 0; i < strlen(str); i++)
    {
        if (str[i] < 0x20)
        {
            if (!found)
            {
                pi.s.println ("");
            }
            found = true;
        } else {
            pi.s.print (str[i]);
            found = false;
        }
    }
    if (!found)
    {
        pi.s.println ("");
    }

    pi.haveUsedHMIputs = true;



    return 0;
}

int hmiPuts(char *str, uint8_t mode)
{
    for (uint8_t i = 0; i < portinformation_elementcount; i++){
        if (mode == HMI_CLI){
            printf ("YOU SHOULD NOT GET HERE\r\n");
        } else {
            hmiPuts (i, str, mode);
        }
    }
}



int hmiPutsTrace(const char *str)
{
    return hmiPuts(str, HMI_TRACE);
}

void hmiPrintCommandPrompt(uint8_t port)
{
    PortInformation &pi = portinformation[port];
    if (pi.haveUsedHMIputs)
    {
        pi.haveUsedHMIputs = false;
        pi.s.print("cmd >");
        for (uint8_t pos = 0; pos < COMMAND_BUFFER_LENGTH; pos++)
        {
            if (pi.RxBuffer[pos] == 0x00)
                break;
            if ((pi.RxBuffer[pos] != '\r') && (pi.RxBuffer[pos] != '\n'))
            { // Dont print CRLF
                pi.s.write(pi.RxBuffer[pos]);
            }
        }
    }
}

// void hmi(void)
// {
//     bool received = getCommandLineFromSerialPort(&CommandLine[0]); // global CommandLine is defined in hmi.h
//     int16_t err = true;

//     int16_t err_1;
//     int16_t err_2;
//     if (received)
//     {
//         for (uint8_t pos = 0; pos < COMMAND_BUFFER_LENGTH; pos++)
//         {
//             if (CommandLine[pos] == 0x00)
//                 break;
//             err = pCmd.read(CommandLine[pos]);
//         }

//         err = false;
//         err_1 = pCmd.read('\r');
//         if (!err_1)
//         {
//             err = pCmd.getError();
//             // Serial.println (err);
//         }
//         err_2 = pCmd.read('\n');
//         if (!err_2)
//         {
//             err = pCmd.getError();
//             // Serial.println (err);
//         }

//         if (
//             ((charLast == '\r') && (charBeforeThat == '\n')) ||
//             ((charLast == '\n') && (charBeforeThat == '\r')))
//         {
//             // Strangely, we can ignire if there is a CR/LF or LF/CR, becasue we see CR __OR__ LF as
//             // the end of line, and have aready dealt with it...
//             charLast = ' ';
//             charBeforeThat = ' ';
//             return;
//         }

//         switch (err)
//         {
//         case -5:
//         case -6:
//             char buf[64];
//             snprintf(buf, 64, "cmd >%s", CommandLine);
//             hmiPuts(buf, HMI_CLI);

//             hmiPuts("Eh?", HMI_CLI);
//             break;
//         case 0:
//             // Serial.print ("cmd >");
//             // Serial.println (CommandLine);
//             break;
//         default:
//             Serial.println("");
//             break;
//         }

//         CommandLine[0] = 0;

//         haveUsedHmiPuts = true;
//         hmiPrintCommandPrompt();
//     }
// }

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
