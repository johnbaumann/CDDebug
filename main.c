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

//
// Protos
//

void DoStuff();

//
// TIM and Sprite data
//

// out of 1000
ulong springiness = 830;


enum UIState
{
    Command_List,
    Parameter_Input,
    Command_Result
} uistate = Command_List;

char is_running = 1;


char paramNibbleSelection = 0;
char paramByteSelection = 0;


char menu_glyph = ' ';
char menu_start_index = 0;
char menu_index = 0;

const int items_per_page = 20;
const int total_num_pages = (CD_CMD_COUNT + (items_per_page - 1)) / items_per_page;

void ResetParamInput()
{

    paramByteSelection = 0;   // 0 -> paramCount
    paramNibbleSelection = 0; // 0/1
}

// TODO: special case for 19h/test
void HandleParamInput(int inCommand)
{

    Blah("\n\n");
    Blah("  Command: %s(0x%02x)\n", commands[menu_index].displayName, menu_index);
    Blah("  Parameters: ");

    if (commands[menu_index].paramCount == 0)
    {
        Blah("  None");
        Blah("\n\n");
        Blah("  X = Send\n");
        return;
    }

    int xOffset = 15;
    int yOFfset = 6;

    if (Released(PADLright))
    {
        paramNibbleSelection++;
        if (paramNibbleSelection > 1)
        {
            paramNibbleSelection = 0;
            paramByteSelection++;
            if (paramByteSelection >= commands[menu_index].paramCount)
            {
                paramByteSelection = 0;
                paramNibbleSelection = 0;
            }
        }
    }
    if (Released(PADLleft))
    {
        paramNibbleSelection--;
        if (paramNibbleSelection < 0)
        {
            paramNibbleSelection = 0;
            paramByteSelection--;
            if (paramByteSelection < 0)
            {
                paramByteSelection = commands[menu_index].paramCount - 1;
                paramNibbleSelection = 1;
            }
        }
    }

    int highlighterX = (xOffset * CHARWIDTH) + (CHARWIDTH * 3 * paramByteSelection) + (CHARWIDTH * paramNibbleSelection);
    highlighterX -= HALFCHAR;
    int highlighterY = yOFfset * CHARWIDTH;
    highlighterY -= HALFCHAR;

    Highlight(highlighterX, highlighterY, 8, 8);

    // handle up down, etc
    // TODO: could be good to move this to scratchpad so you can keep values between reboots
    unsigned char paramByte = commands[menu_index].params[paramByteSelection];
    char nibble = (paramNibbleSelection) ? (paramByte & 0xF) : ((paramByte & 0xF0) >> 4);

    if (Released(PADLup))
    {
        nibble++;
    }

    if (Released(PADLdown))
    {
        nibble--;
    }

    if (Released(PADL1))
    {
        nibble = 0;
    }

    if (Released(PADR1))
    {
        nibble = 0xF;
    }

    if (Released(PADR2))
    {
        nibble = 8;
    }

    if (Released(PADL2))
    {
        nibble = 8;
    }

    if (nibble < 0x0)
        nibble = 0xF;
    if (nibble > 0xF)
        nibble = 0x0;

    if (paramNibbleSelection)
    {
        // the right hand nibble
        paramByte &= 0xF0;            // just keep the most significant part
        paramByte |= (nibble & 0x0F); // then stick the nibble in
    }
    else
    {
        // the left hand nibble;
        paramByte &= 0x0F;                   // just keep the least significant part
        paramByte |= ((nibble << 4) & 0xF0); // then stick the nibble in (sounds like pure gibberish)
    }

    // write the byte back
    commands[menu_index].params[paramByteSelection] = paramByte;

    for (int i = 0; i < commands[menu_index].paramCount; i++)
    {
        Blah("%02x ", commands[menu_index].params[i]);
    }

    // this could probably wait, but it's 5 bytes; I'm sure we'll live.
    NewMemcpy(cmd_params, commands[menu_index].params, sizeof(cmd_params));

    Blah("\n\n");
    Blah(" Push X for make thing heppen\n");

    /*
    Blah( "%d\n", nibble );
    for (int i = 0; i < commands[menu_index].paramCount; i++)
    {
        Blah("%02x ", cmd_params[i] );
    }
    Blah( "\n byteSel %d, nibbleSel %d, nibbleVal %d\n", paramByteSelection, paramNibbleSelection, nibble );
    */
}

void SetState(int inState)
{

    ResetParamInput();

    uistate = inState;
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
    NewPrintf("d86 = %4X\n", *(volatile unsigned short *)0x1F801D86);
    NewPrintf("d84 = %4X\n", *(volatile unsigned short *)0x1F801D84);
    NewPrintf("d82 = %4X\n", *(volatile unsigned short *)0x1F801D82);
    NewPrintf("d80 = %4X\n", *(volatile unsigned short *)0x1F801D80);
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
                Blah("%c%s\n", menu_glyph, commands[i].displayName);
            }
            break;

        case Parameter_Input:

            HandleParamInput(menu_index);
            break;

        case Command_Result:
            Blah("COMMAND: %s(", commands[menu_index].displayName);
            for (int i = 0; i < commands[menu_index].paramCount; i++)
            {
                Blah("0x%02x", cmd_params[i]);
                if (i + 1 < commands[menu_index].paramCount)
                    Blah(",");
            }
            Blah(")\n");
            Blah("RESULT = INT%i(", lastInt, lastResponse);
            for (int i = 0; i < lastResponseLength; i++)
            {
                Blah("0x%02x", cdResponseBuffer[i]);
                if (i + 1 < lastResponseLength)
                    Blah(",");
            }
            Blah(")\n");
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
            SetState(Command_List);
        }
        break; // Parameter_Input

    case Command_Result:

        break; // Command_Result
    }          // switch (uistate)

    if (Released(PADRright))
    {

        if (uistate != Command_List)
        {
            SetState(Command_List);
        }
        else
        {
            is_running = 0;
        }
    }

    if (Released(PADRdown))
    {
        switch (uistate)
        {
        case Command_List:
            SetState(Parameter_Input);
            break;

        case Parameter_Input:
            // Send command
            SendTheCommand(menu_index);
            SetState(Command_Result);
            break;

        case Command_Result:
            SetState(Command_List);
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
            SetState(Command_List);
            break;

        case Command_Result:
            SetState(Command_List);
            break;
        }
    }
}

#endif // ! MAINC
