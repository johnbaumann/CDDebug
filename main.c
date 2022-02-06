// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.
//
// Standard kinda fare though, no explicit or implied warranty.
// Any issues arising from the use of this software are your own.
//
// https://github.com/JonathanDotCel
//

#ifndef MAINC
#define MAINC

// variadic logging
#include <stdarg.h>
#include "main.h"

//
// Includes
//

#include "littlelibc.h"
#include "utility.h"
#include "drawing.h"
#include "pads.h"
#include "ttyredirect.h"
#include "config.h"
#include "gpu.h"
#include "timloader.h"
#include "cd.h"

#define CD_CMD_COUNT 31
#define CD_RESPONSE_LENGTH 0x20

//
// Protos
//

void DoStuff();

//
// TIM and Sprite data
//

// out of 1000
ulong springiness = 830;

const unsigned char cmdCount = 31;

const char cmdStrings[CD_CMD_COUNT][13] = {
    "CdlSync",
    "CdlGetStat",
    "CdlSetloc",
    "CdlPlay",
    "CdlForward",
    "CdlBackward",
    "CdlReadN",
    "CdlStandby",
    "CdlStop",
    "CdlPause",
    "CdlReset",
    "CdlMute",
    "CdlDemute",
    "CdlSetfilter",
    "CdlSetmode",
    "CdlGetparam",
    "CdlGetlocL",
    "CdlGetlocP",
    "CdlReadT",
    "CdlGetTN",
    "CdlGetTD",
    "CdlSeekL",
    "CdlSeekP",
    "CdlSetclock",
    "CdlGetclock",
    "CdlTest",
    "CdlID",
    "CdlReadS",
    "CdlInit",
    "CdlGetQ",
    "CdlReadToc"};

static const unsigned char paramCount[CD_CMD_COUNT] = {0, 0, 3, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 1, 0,
                                                       0, 0, 1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 2, 0};

static const unsigned char ackCount[CD_CMD_COUNT] = {1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 1, 1, 1, 1, 1,
                                                     1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 2};

unsigned char cmd_params[3] = {0x00, 0x00, 0x00};

char menu_glyph = ' ';
char menu_start_index = 0;
char menu_index = 0;

const int items_per_page = 20;
const int total_num_pages = (CD_CMD_COUNT + (items_per_page - 1)) / items_per_page;

static ulong lastInt = 0;
static ulong lastResponse = 0;
static char cdResponseBuffer[CD_RESPONSE_LENGTH];
char *GetCDResponseBuffer() { return (char *)&cdResponseBuffer; }

enum UIState
{
    Command_List,
    Parameter_Input,
    Command_Result
} uistate = Command_List;

char is_running = 1;

static __attribute__((always_inline)) int CDClearInts()
{

    pCDREG0 = 1;
    pCDREG3 = 0x1F;

    // pCDREG0 = 0;
}

#pragma GCC push options
#pragma GCC optimize("-O0")
// Warning: does not ack or clear ints
// Call CDMultiAck() instead if you want to ack
// an interrupt and dump the values to the default buffer.
char CDReadResponses(char *inBuffer, ulong maxLength)
{

    // Ack() first

    char lastReadVal = 0;

    int numRead = 0;

    while ((pCDREG0 & CDREG0_DATA_IN_RESPONSEFIFO) != 0)
    {

        lastReadVal = CDReadResponse() & 0xFF;
        *inBuffer++ = lastReadVal;

        numRead++;
    }

    return numRead;

    // ClearInts() after
}
#pragma GCC pop optionss

int CDReadResponse()
{

    // select response Reg1, index1 : Response fifo
    pCDREG0 = 0x01;
    char returnValue = pCDREG1;

    return returnValue;
}

void CDStartCommand()
{

    int i;

    while ((pCDREG0 & CDREG0_DATA_IN_DATAFIFO) != 0)
        ;
    while ((pCDREG0 & CDREG0_DATA_BUSY) != 0)
        ;

    // Select Reg3,Index 1 : 0x1F resets all IRQ bits
    CDClearInts();

    // Reg2 Index 0 = param fifo
    pCDREG0 = 0;
}

int CDWaitInt()
{

    // would break ability to get multiple responses
    // from the 2nd int
    // CDClearInts();

    // Reg 3 index 1 = Interrupt flags
    // note: shell puts 'reg0=1' inside the while loop
    pCDREG0 = 1;
    while ((pCDREG3 & 0x07) == 0)
        ;

    int returnInt = (pCDREG3 & 0x07);

    return returnInt;
}

void CDWriteParam(uchar inParam)
{

    // pCDREG0 = 0;                //not required in a loop, but good practice?
    pCDREG2 = inParam;
}

#pragma GCC push options
#pragma GCC optimize("-O0")
void CDWriteCommand(uchar inCommand)
{
    // Finish by writing the command
    pCDREG0 = 0;
    pCDREG1 = inCommand;
}
#pragma gcc pop options

#pragma GCC options push
#pragma GCC optimise("-O0") // required to get the printf in the right order
// returns: last response value
int CDAck()
{
    lastInt = CDWaitInt();
    lastResponse = CDReadResponse();
    CDClearInts();
    return lastResponse;
}
#pragma GCC options pop

void SendTheCommand()
{
    CDStartCommand();

    for (int i = 0; i < paramCount[menu_index]; i++)
    {
        CDWriteParam(cmd_params[i]);
    }

    CDWriteCommand(menu_index);

    for(int i = 0; i < ackCount[menu_index]; i++)
    {
        CDAck();
    }
}

void CDSendCommand_GetStat()
{
    CDStartCommand();
    CDWriteCommand(CD_CMD_GETSTAT);
}

static void CDSendCommand_Init()
{
    CDStartCommand();
    CDWriteCommand(CD_CMD_INIT);
}

void InitCD()
{
    CDClearInts();

    pCDREG0 = 0;
    pCDREG3 = 0;
    pCOM_DELAY = 4901;

    CDSendCommand_GetStat();
    CDAck();
    CDSendCommand_GetStat();
    CDAck();
    CDSendCommand_Init();
    CDAck();
}

int main()
{
    //ResetEntryInt();
    ExitCritical();

    // Clear the text buffer
    InitBuffer();

    // Init the pads after the GPU so there are no active
    // interrupts and we get one frame of visual confirmation.

    //NewPrintf("Init GPU...\n");
    InitGPU();

    //NewPrintf("Init Pads...\n");
    InitPads();

    //RemoveTTY();
    //InstallTTY();

    InitCD();

    // Main loop
    while (is_running)
    {

        MonitorPads();

        ClearScreenText();
        DrawBG();

        Blah("\n\n");

        switch (uistate)
        {
        case Command_List:
            Blah("CD Commands: Page %i/%i\n", (menu_start_index / items_per_page) + 1, total_num_pages);
            for (int i = menu_start_index; i < menu_start_index + items_per_page && i < CD_CMD_COUNT; i++)
            {
                menu_glyph = (i == menu_index) ? '>' : ' ';
                Blah("%c%s\n", menu_glyph, cmdStrings[i]);
            }
            break;

        case Parameter_Input:
            Blah("Command: %s(0x%02x)\n", cmdStrings[menu_index], menu_index);
            Blah("Parameters: ");

            if (paramCount[menu_index] == 0)
            {
                Blah("None");
            }
            else
            {
                for (int i = 0; i < paramCount[menu_index]; i++)
                {
                    Blah("%02x", cmd_params[i]);
                    if (i + 1 < paramCount[menu_index])
                        Blah(",");
                }
            }
            break;

        case Command_Result:
            Blah("RESULT = INT%i(0x%02x)\n", lastInt, lastResponse);
            break;
        }

        DoStuff();

        Draw();
    }

    return 1;
}

void DoStuff()
{
    switch (uistate)
    {
    case Command_List:
        if (Released(PADLup))
        {
            if (menu_index > 0)
            {
                menu_index--;
            }
            else
            {
                menu_index = CD_CMD_COUNT - 1;
            }
            menu_start_index = items_per_page * (menu_index / items_per_page);
        }
        if (Released(PADLdown))
        {
            if (menu_index < CD_CMD_COUNT - 1)
            {
                menu_index++;
            }
            else
            {
                menu_index = 0;
            }
            menu_start_index = items_per_page * (menu_index / items_per_page);
        }
        break; // Command_List

    case Parameter_Input:
        if (Released(PADRup))
        {
            uistate = Command_List;
        }
        break; // Parameter_Input

    case Command_Result:

        break; // Command_Result
    }          // switch (uistate)

    if (Released(PADRright))
    {
        is_running = 0;
    }

    if (Released(PADRdown))
    {
        switch (uistate)
        {
        case Command_List:
            uistate = Parameter_Input;
            break;

        case Parameter_Input:
            // Send command
            SendTheCommand();
            uistate = Command_Result;
            break;

        case Command_Result:
            uistate = Command_List;
            break;
        }
    }

    if (Released(PADRup))
    {
        switch (uistate)
        {
        case Command_List:
            menu_index = 0;
            menu_start_index = 0;
            break;

        case Parameter_Input:
            uistate = Command_List;
            break;

        case Command_Result:
            uistate = Command_List;
            break;
        }
    }
}

#endif // ! MAINC
