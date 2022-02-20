#include "cd.h"

#include "hwregs.h"

#include <stdint.h>
#include "littlelibc.h"

 COMMANDS commands[31] = {
    "CdlSync",
    0,
    1,
    {0, 0, 0, 0, 0},
    "CdlGetStat",
    0,
    1,
    {0, 0, 0, 0, 0},
    "CdlSetloc",
    3,
    1,
    {0, 0, 0, 0, 0},
    "CdlPlay",
    1,
    1,
    {0, 0, 0, 0, 0},
    "CdlForward",
    0,
    1,
    {0, 0, 0, 0, 0},
    "CdlBackward",
    0,
    1,
    {0, 0, 0, 0, 0},
    "CdlReadN",
    0,
    1,
    {0, 0, 0, 0, 0},
    "CdlStandby",
    0,
    2,
    {0, 0, 0, 0, 0},
    "CdlStop",
    0,
    2,
    {0, 0, 0, 0, 0},
    "CdlPause",
    0,
    2,
    {0, 0, 0, 0, 0},
    "CdlReset",
    0,
    2,
    {0, 0, 0, 0, 0},
    "CdlMute",
    0,
    1,
    {0, 0, 0, 0, 0},
    "CdlDemute",
    0,
    1,
    {0, 0, 0, 0, 0},
    "CdlSetfilter",
    2,
    1,
    {0, 0, 0, 0, 0},
    "CdlSetmode",
    1,
    1,
    {0, 0, 0, 0, 0},
    "CdlGetparam",
    0,
    1,
    {0, 0, 0, 0, 0},
    "CdlGetlocL",
    0,
    1,
    {0, 0, 0, 0, 0},
    "CdlGetlocP",
    0,
    1,
    {0, 0, 0, 0, 0},
    "CdlReadT",
    1,
    2,
    {0, 0, 0, 0, 0},
    "CdlGetTN",
    0,
    1,
    {0, 0, 0, 0, 0},
    "CdlGetTD",
    1,
    1,
    {0, 0, 0, 0, 0},
    "CdlSeekL",
    0,
    1,
    {0, 0, 0, 0, 0},
    "CdlSeekP",
    0,
    1,
    {0, 0, 0, 0, 0},
    "CdlSetclock",
    0,
    1,
    {0, 0, 0, 0, 0},
    "CdlGetclock",
    0,
    1,
    {0, 0, 0, 0, 0},
    "CdlTest",
    1,
    1,
    {0, 0, 0, 0, 0},
    "CdlID",
    0,
    2,
    {0, 0, 0, 0, 0},
    "CdlReadS",
    0,
    1,
    {0, 0, 0, 0, 0},
    "CdlInit",
    0,
    1,
    {0, 0, 0, 0, 0},
    "CdlGetQ",
    2,
    1,
    {0, 0, 0, 0, 0},
    "CdlReadToc",
    0,
    2,
    {0, 0, 0, 0, 0},
};

// added 2 for the 0x19 test commands
unsigned char cmd_params[5] = {0x00, 0x00, 0x00};


ulong lastInt = 0;
ulong lastResponse = 0;
ulong lastResponseLength = 0;
char cdResponseBuffer[CD_RESPONSE_LENGTH];
char *GetCDResponseBuffer() { return (char *)&cdResponseBuffer; }


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

void SendTheCommand(char cmd_index)
{
    CDStartCommand();

    for (int i = 0; i < commands[cmd_index].paramCount; i++)
    {
        CDWriteParam(cmd_params[i]);
    }

    CDWriteCommand(cmd_index);

    for (int i = 0; i < commands[cmd_index].ackCount; i++)
    {
        lastInt = CDWaitInt();
        lastResponseLength = CDReadResponses(cdResponseBuffer, CD_RESPONSE_LENGTH);
        CDClearInts();
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

void CD_initvol()
{
    short *spu_base_address;

    spu_base_address = pSPUVOICE0;
    //if((pSPUVOICE0 )
    //if 

    // spu main vol L & R
    *(volatile unsigned short *)0x1F801D80 = 0x3fff;
    *(volatile unsigned short *)0x1F801D82 = 0x3fff;

    // cd vol L & R
    *(volatile unsigned short *)0x1F801DB0 = 0x3fff;
    *(volatile unsigned short *)0x1F801DB2 = 0x3fff;

    // SPU Control register
    *(volatile unsigned short *)0x1F801DAA = 0xc001; // CD audio enable

    pCDREG0 = 2;
    pCDREG2 = 0x80;
    pCDREG3 = 0x00;
    pCDREG0 = 3;
    pCDREG1 = 0x80;
    pCDREG2 = 0x00;
    pCDREG3 = 0x20;
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

    /*pCDREG0 = 1;
    pCDREG3 = 0x1f;
    pCDREG2 = 0x1f;

    pCDREG0 = 0;
    pCDREG3 = 0;*/

    CD_initvol();
}