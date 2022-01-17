/*!
******************************************************************************
\file file_name.h
\date 17 October 2021
\author Rony Kosistky & Ofir Guthman & Yonatan Gartenberg
\brief
\details
\par Copyright
(c) Copyright 2021 Ofir & Rony & Yonatan Gartenberg
\par
ALL RIGHTS RESERVED
*****************************************************************************/

#ifndef __HELPERS_H__
#define __HELPERS_H__
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
	

#define NUMBER_OF_REGISTERS 16
#define REG_ZERO 0
#define REG_IMM 1
#define NEXT_INSTRUCTION_ADDRESS_REGISTER 15
#define CORES_NUMBER 4

/************************************
*       types                       *
************************************/
typedef union
{
	struct
	{
		uint16_t immediate : 12;
		uint16_t rt : 4;		
		uint16_t rs : 4;		
		uint16_t rd : 4;		
		uint16_t opcode : 8;	
	} bits;

	uint32_t cmd;
} inst;


/*File declarations*/
typedef struct
	{
		FILE* imem_F;
		FILE* regout_F;
		FILE* core_trace_F;
		FILE* dsram_F;
		FILE* TsRamFile;
		FILE* StatsFile;
	} output_core_file;

extern output_core_file files_of_cores[CORES_NUMBER];
FILE* MeminFile;
FILE* MemoutFile;
FILE* BusTraceFile;




#endif //__FILE_NAME_H__