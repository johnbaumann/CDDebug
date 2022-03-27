#pragma once

//#define CD_DEBUG 1

// ulong
#include <stdlib.h>

#include "config.h"
#include "hwregs.h"
#include "littlelibc.h"

#define COM_DELAY 0xBF801020
#define pCOM_DELAY *(volatile ulong*)COM_DELAY

#define CD_CMD_SYNC 0x00
#define CD_CMD_GETSTAT 0x01  // also known as nop
#define CD_CMD_SETLOC 0x02
#define CD_CMD_PLAY 0x03
#define CD_CMD_FORWARD 0x04
#define CD_CMD_BACKWARD 0x05
#define CD_CMD_READN 0x06
#define CD_CMD_PAUSE 0x09
#define CD_CMD_INIT 0x0A
#define CD_CMD_SETMODE 0x0E
#define CD_CMD_GETLOCP 0x11
#define CD_CMD_SETSESSION 0x12
#define CD_CMD_GET_TD 0x14
#define CD_CMD_SEEKL 0x15
#define CD_CMD_SEEKP 0x16

#define CD_CMD_TEST 0x19
#define CD_TESTPARAM_STOP 0x03
#define CD_CMD_GETID 0x1A
#define CD_CMD_READS 0x1B
#define CD_CMD_RESET 0x1C
#define CD_CMD_READTOC 0x1E

#define FIFO_STAT_NOINTR_0 0x0
#define FIFO_STAT_DATAREADY_1 0x1
#define FIFO_STAT_ACK_2 0x2
#define FIFO_STAT_COMPLETE_3 0x3
#define FIFO_STAT_DATAEND_4 0x4
#define FIFO_STAT_ERROR_5 0x5

#define CD_IRQ_MASK 0x04

#define REG3_IDX1_RESET_PARAM_FIFO 0x40
#define REG3_IDX0_REQUEST_DATA 0x80

#define CDREG0_DATA_IN_RESPONSEFIFO 0x20
#define CDREG0_DATA_IN_DATAFIFO 0x40
#define CDREG0_DATA_BUSY 0x80


#define CD_CMD_COUNT 31
#define CD_RESPONSE_LENGTH 0x20

typedef struct COMMANDS
{

    char *displayName;
    unsigned char paramCount;
    unsigned char ackCount;
    unsigned char params[5]; // We can cache these per-command so it doesn't get tedious going back and forth

} COMMANDS;

extern COMMANDS commands[31];
extern unsigned char cmd_params[5];
extern ulong lastInt;
extern ulong lastResponse;
extern ulong lastResponseLength;
extern char cdResponseBuffer[CD_RESPONSE_LENGTH];

void CDInitForShell();

int CDReadSector( ulong inCount, ulong inSector, char* inBuffer );
int CDOpenAndRead( const char* inFile, char* inBuffer, ulong numSectors );

ulong CDCheckUnlocked();

int CDStop();
void CDWaitReady();
void InitCD();
void SendTheCommand(char cmd_index);

void CDStartCommand();
void CDWriteParam( unsigned char inParam );
void CDWriteCommand( unsigned char inCommand );

int CDAck();             // called 1-many times after each writecommand (one for each expected int)
ulong CDLastInt();       // read the last interrupt generated from CDAck();
ulong CDLastResponse();  // read the last response generated from CDAck();
int CDReadResponse();    // for e.g. GetLocP which requires multiple responses for one ack
int CDClearInts();

// Multiple responses?
// return: number of response bytes returned
// use CDLastInt to get the last interrupt type
int CDMultiAck();
char* GetCDResponseBuffer();

ulong Pad2k( ulong inSize );  // Rounds a size up to 2K for loading a sector.
