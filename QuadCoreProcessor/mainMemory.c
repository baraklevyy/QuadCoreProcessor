#define _CRT_SECURE_NO_WARNINGS
/*!
******************************************************************************
\file MainMemory.c
\date 17 October 2021
\author Rony Kositsky & Ofir Guthman & Yonatan Gartenberg
\brief
\details
\par Copyright
(c) Copyright 2021 Ofir & Rony & Yonatan
\par
ALL RIGHTS RESERVED
*****************************************************************************/

/************************************
*      include                      *
************************************/
#include "mainMemory.h"
#include "files.h"
#include "bus.h"
#include <string.h>

/************************************
*       types                       *
************************************/
typedef union
{
	uint32_t address;

	struct
	{
		uint16_t offset : 2;	// [0:1]
		uint32_t block : 18;	// [2:19]
	} fields;
} memory_addess_s;

/************************************
*      variables                    *
************************************/
static uint32_t gMemory[MAIN_MEMORY_SIZE];
static uint8_t counter;
static bool gMemoryTransaction;

/************************************
*      static functions             *
************************************/
static bool bus_transaction_handler(Bus_packet_s* packet, bool direct_transaction);
static uint32_t get_memory_length(void);

/************************************
*       API implementation          *
************************************/

/*!
******************************************************************************
\brief
Initialize main memory from input file.
\details
Must be called only once
\return none
*****************************************************************************/
void MainMemory_Init(void)
{
	memset((uint8_t*)gMemory, 0, sizeof(gMemory));
	counter = 0;
	gMemoryTransaction = false;

	uint16_t lineInProgram = 0;
	while (lineInProgram < MAIN_MEMORY_SIZE && fscanf(MeminFile, "%08x", (uint32_t*)&(gMemory[lineInProgram])) != EOF)
		lineInProgram++;

	Bus_RegisterMemoryCallback(bus_transaction_handler);
}

void MainMemory_PrintData(void)
{
	uint32_t length = get_memory_length();
	for (uint32_t i = 0; i < length; i++)
		fprintf(MemoutFile, "%08X\n", gMemory[i]);
}


/************************************
* static implementation             *
************************************/

/*!
******************************************************************************
\brief
Initialize main memory from input file.
[in] Bus_packet_s* packet	 - pointer to bus packet.
[in] bool direct_transaction - is direct transaction to the memory.
\return false if finished, true otherwise.
*****************************************************************************/
static bool bus_transaction_handler(Bus_packet_s* packet, bool direct_transaction)
{
	if (packet->bus_cmd == bus_no_command)
		return false;

	if (!gMemoryTransaction)
	{
		gMemoryTransaction = true;
		counter = !direct_transaction ? 0 : 16;
	}

	if (counter >= 16)
	{
		if (packet->bus_cmd == bus_busRd || packet->bus_cmd == bus_busRdX)
		{
			// send the memory value
			packet->bus_origid = bus_main_memory;
			packet->bus_cmd = bus_flush;
			packet->bus_data = gMemory[packet->bus_addr];
		}
		else if (packet->bus_cmd == bus_flush)
		{
			// write data to memory
			gMemory[packet->bus_addr] = packet->bus_data;
		}

		if (counter == 19)
			gMemoryTransaction = false;

		counter++;
		return true;
	}

	counter++;
	return false;
}

static uint32_t get_memory_length(void)
{
	uint32_t length = MAIN_MEMORY_SIZE - 1;

	for (; length > 0; length--)
	{
		if (gMemory[length])
			break;
	}

	return length + 1;
}