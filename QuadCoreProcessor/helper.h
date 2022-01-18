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

#define MAIN_MEMORY_SIZE 1048576 
#define BAD_EXIT_CODE 1
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


typedef struct
{
	uint32_t address;
	uint16_t offset;
	uint16_t block;
} memory_addess_s;


/*
typedef struct
{
	uint32_t data;

	struct
	{
		uint16_t tag : 12;	// [0:11]
		uint16_t mesi : 2;	// [12:13]
	}fields;
} Tsram_s;
*/

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

enum core_E{core0, core1, core2, core3};
enum file_names_E{ imem0 = 1, imem1 , imem2, imem3,
	memin, memout,
	regout0, regout1, regout2, regout3,
	core0trace, core1trace, core2trace, core3trace, bustrace,
	dsram0, dsram1, dsram2, dsram3,
	tsram0, tsram1, tsram2, tsram3,
	stats0, stats1, stats2, stats3};

uint16_t get_address_offset(uint32_t address);
uint16_t get_address_block(uint32_t address);
void set_offset_to_address(uint32_t* address, uint8_t offset);
uint16_t get_tsram_tag(uint32_t tsram);
uint16_t get_tsram_mesi_state(uint32_t tsram);
void set_tag_to_tsram(uint32_t* tsram, uint16_t tag);
void set_mesi_state_to_tsram(uint32_t* tsram, uint16_t mesi_state);


#endif //__FILE_NAME_H__