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

#define MAX_INTEGER 0xffffui16
#define MAIN_MEMORY_SIZE 1048576 
#define BAD_EXIT_CODE 1
#define NUMBER_OF_REGISTERS 16
#define REG_ZERO 0
#define REG_IMM 1
#define NEXT_INSTRUCTION_ADDRESS_REGISTER 15
#define CORES_NUMBER 4
#define CACHE_SIZE	256
#define BLOCK_SIZE	4
#define TSRAM_NUMBER_OF_LINES 64
#define PIPELINE_SIZE 5
#define _CSECURE_NO_WARNINGS

/************************************
*       types                       *
************************************/
/*
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
*/

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
/*
typedef struct
{
	uint32_t read_hits;
	uint32_t write_hits;
	uint32_t read_misses;
	uint32_t write_misses;
} CacheStatistics_s;
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

/*
typedef struct
{
	core_identifier id;
	bool memory_stall;
	uint32_t dsram[CACHE_SIZE];
	//Tsram_s tsram[TSRAM_NUMBER_OF_LINES];
	uint32_t tsram[TSRAM_NUMBER_OF_LINES];
	CacheStatistics_s statistics;
} cache_information;
*/
/*
typedef struct
{
	uint32_t read_hits;
	uint32_t write_hits;
	uint32_t read_misses;
	uint32_t write_misses;
} CacheStatistics_s;


typedef struct
{
	core_identifier id;
	bool memory_stall;
	uint32_t dsram[CACHE_SIZE];
	//Tsram_s tsram[TSRAM_NUMBER_OF_LINES];
	uint32_t tsram[TSRAM_NUMBER_OF_LINES];
	CacheStatistics_s statistics;
} cache_information;
*/

typedef struct
{
	bool* is_command_in_halt;
	uint32_t rs;
	uint16_t* pc;
	uint32_t* rd;
	uint32_t rt;
}parameters_to_command;








extern output_core_file files_of_cores[CORES_NUMBER];
FILE* MeminFile;
FILE* MemoutFile;
FILE* BusTraceFile;

enum core_E{core0=0, core1, core2, core3};
enum file_names_E{ imem0 = 1, imem1 , imem2, imem3,
	memin, memout,
	regout0, regout1, regout2, regout3,
	core0trace, core1trace, core2trace, core3trace, bustrace,
	dsram0, dsram1, dsram2, dsram3,
	tsram0, tsram1, tsram2, tsram3,
	stats0, stats1, stats2, stats3};

typedef enum{invalid=0, shared, exclusive, modified} mesi_state;
typedef enum{CORE0=0, CORE1, CORE2, CORE3,} core_identifier;
typedef enum { ADD = 0, SUB, AND, OR, XOR, MUL, SLL, SRA, SRL, BEQ, BNE, BLT, BGT, BLE, BGE, JAL, LW, SW, HALT = 20 }OpcodeOperations;


uint16_t get_address_offset(uint32_t address);
uint16_t get_address_block(uint32_t address);
void set_offset_to_address(uint32_t* address, uint8_t offset);
uint16_t get_tsram_tag(uint32_t tsram);
uint16_t get_tsram_mesi_state(uint32_t tsram);
void set_tag_to_tsram(uint32_t* tsram, uint16_t tag);
void set_mesi_state_to_tsram(uint32_t* tsram, uint16_t mesi_state);
uint32_t get_cache_address_offset(uint32_t cache);
uint32_t get_cache_address_index(uint32_t cache);
uint32_t get_cache_address_tag(uint32_t cache);
void set_offset_to_cache_address(uint32_t* cache, uint32_t offset);
void set_index_to_cache_address(uint32_t* cache, uint32_t index);
void set_tag_to_cache_address(uint32_t* cache, uint32_t tag);
uint16_t get_command_immediate(uint32_t cmd);
uint16_t get_command_rt(uint32_t cmd);
uint16_t get_command_rs(uint32_t cmd);
uint16_t get_command_rd(uint32_t cmd);
uint16_t get_command_opcode(uint32_t cmd);
void add(parameters_to_command* arguments_to_cmd);
void sub(parameters_to_command* arguments_to_cmd);
void and (parameters_to_command* arguments_to_cmd);
void or (parameters_to_command * arguments_to_cmd);
void xor (parameters_to_command* arguments_to_cmd);
void mul(parameters_to_command* arguments_to_cmd);
void sll(parameters_to_command* arguments_to_cmd);
void sra(parameters_to_command* arguments_to_cmd);
void srl(parameters_to_command* arguments_to_cmd);
void beq(parameters_to_command* arguments_to_cmd);
void bne(parameters_to_command* arguments_to_cmd);
void blt(parameters_to_command* arguments_to_cmd);
void bgt(parameters_to_command* arguments_to_cmd);
void ble(parameters_to_command* arguments_to_cmd);
void bge(parameters_to_command* arguments_to_cmd);
void jal(parameters_to_command* arguments_to_cmd);

static void (*opcode_command_function_pointer[16])(parameters_to_command* arguments_to_cmd) = {
	 add, sub, and, or, xor, mul, sll, sra,
	 srl, beq, bne, blt, bgt,
	 ble, bge, jal
};







#endif //__FILE_NAME_H__