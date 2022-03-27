// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef HWREGS_H
#define HWREGS_H

// Shared by the CD and IRQ handlers
#define CDREG0 0xBF801800
#define pCDREG0 *(volatile uchar*)CDREG0

#define CDREG1 0xBF801801
#define pCDREG1 *(volatile uchar*)CDREG1

#define CDREG2 0xBF801802
#define pCDREG2 *(volatile uchar*)CDREG2

#define CDREG3 0xBF801803
#define pCDREG3 *(volatile uchar*)CDREG3

#define CDISTAT 0xBF801070
#define pCDISTAT *(volatile ushort*)CDISTAT

#define ISTAT 0xBF801070
#define	pISTAT *(volatile ulong*)ISTAT

#define IMASK 0xBF801074
#define pIMASK *(volatile ulong*)IMASK

// 0x10 bytes into the scratch pad
// anywhere's good, really, but it's 0x10 long
#define PAD1BUFFER 0x1F800010
#define pPAD1BUFFER (unsigned char*)PAD1BUFFER
#define PAD2BUFFER 0x1F800020
#define pPAD2BUFFER (unsigned char*)PAD2BUFFER

#define SPUVOICE0 0x1F801C00
#define pSPUVOICE0 *(volatile ushort*)SPUVOICE0

#define DMA_DPCR 0x1F8010F0
#define pDMA_DPCR *(volatile ulong*)DMA_DPCR

#define SPU_DELAY 0x1F801014
#define pSPU_DELAY *(volatile ulong*)SPU_DELAY

#define SOUND_RAM_DATA_TRANSFER_ADDR 0x1F801DA6
#define pSOUND_RAM_DATA_TRANSFER_ADDR *(volatile ushort*)SOUND_RAM_DATA_TRANSFER_ADDR

#define SPU_CTRL_REG_CPUCNT 0x1F801DAA
#define pSPU_CTRL_REG_CPUCNT *(volatile ushort*)SPU_CTRL_REG_CPUCNT

#define DMA_SPU_MADR 0x1F8010C0
#define pDMA_SPU_MADR *(volatile ulong*)DMA_SPU_MADR

#define DMA_SPU_BCR 0x1F8010C4
#define pDMA_SPU_BCR *(volatile ulong*)DMA_SPU_BCR

#define DMA_SPU_CHCR 0x1F8010C8
#define pDMA_SPU_CHCR *(volatile ulong*)DMA_SPU_CHCR


#endif