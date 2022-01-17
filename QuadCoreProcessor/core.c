/*!
******************************************************************************
\file Core.c
\date 26 October 2021
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
#define _CRT_SECURE_NO_WARNINGS
#include "core.h"
#include <stdio.h>
#include <string.h>

/************************************
*      definitions                 *
************************************/
#define INSTRUCTION_COUNT 0

/************************************
*      variables                    *
************************************/

/************************************
*      static functions             *
************************************/
static int init_memory(Core_s* core);
static void write_trace(Core_s* core, uint32_t* regs_copy);
static void write_regs_to_file(Core_s* core, uint32_t* regs_copy);
static void update_statistics(Core_s* core);
static void print_register_file(Core_s* core);
static void print_statistics(Core_s* core);

/************************************
*       API implementation          *
************************************/

/*!
******************************************************************************
\brief
Init the core.
\details
Called at the start of the run.
\param
 [in] core - the operating core
\return none
*****************************************************************************/
void Core_Init(Core_s* core, uint8_t id)
{
	memset(&core->register_array, 0, sizeof(NUMBER_OF_REGISTERS));
	int number_of_lines = init_memory(core);
	if (!number_of_lines)
	{
		core->core_halted = true;
		return;
	}

	core->program_counter = 0;
	core->index = id;
	core->core_halted = false;

	memset(&core->statistics, 0, sizeof(Statistics_s));
	core->statistics.cycles = -1; // To start the count from 0.

	memset(&core->pipeline, 0, sizeof(Pipeline_s));
	Pipeline_Init(&core->pipeline);

	memset(&core->pipeline.cache_data, 0, sizeof(CacheData_s));
	Cache_Init(&core->pipeline.cache_data, id);

	//TODO: maybe move it to different location.
	Cache_RegisterBusHandles();

	core->pipeline.core_registers_p = core->register_array;
	core->pipeline.insturcionts_p = core->instructions_memory_image;
	core->pipeline.opcode_params.pc = &(core->program_counter);
}

/*!
******************************************************************************
\brief
Run core iteration
\details
core is running with pipeline.
\param
 [in] core - the operating core
\return none
*****************************************************************************/
void Core_Iter(Core_s* core)
{
	if (core->core_halted)
	{
		return;
	}

	if (Pipeline_PipeFlushed(&core->pipeline))
	{
		core->core_halted = true;
		return;
	}

	uint32_t regs_copy[NUMBER_OF_REGISTERS];
	memcpy(regs_copy, core->register_array, sizeof(core->register_array));

	update_statistics(core);
	Pipeline_Execute(&core->pipeline, core->index);
	write_trace(core, regs_copy);
	Pipeline_BubbleCommands(&core->pipeline);
}

/*!
******************************************************************************
\brief
Teardown of the code.
\param
 [in] core - the operating core
\return none
*****************************************************************************/
void Core_Teaddown(Core_s* core)
{
	print_register_file(core);
	Cache_PrintData(&core->pipeline.cache_data,
		core->core_files.dsram_F, core->core_files.TsRamFile);
	print_statistics(core);
}

/*!
******************************************************************************
\brief
If the core is halted.
\param
 [in] core - the operating core
 [out] bool
\return true if core is halted, fale otherwise.
*****************************************************************************/
bool Core_Halted(Core_s* core)
{
	return core->core_halted;
}


/************************************
* static implementation             *
************************************/

/*!
******************************************************************************
\brief
Init the core instructions memory.
\param
 [in] core - the operating core
\return number of memory lines
*****************************************************************************/
static int init_memory(Core_s* core)
{
	int number_of_lines = 0;
	while (number_of_lines < INSTRUCTIONS_MEMORY_SIZE && fscanf(core->core_files.imem_F,
		"%08x", (uint32_t*)&(core->instructions_memory_image[number_of_lines])) != EOF)
	{
		number_of_lines++;
	}

	return number_of_lines;
}

/*!
******************************************************************************
\brief
Writing the trace of the core.
\param
 [in] core				  - the operating core
 [in] uint32_t *regs_copy - pointer to the regs copied values.
\return none
*****************************************************************************/
static void write_trace(Core_s* core, uint32_t* regs_copy)
{
	fprintf(core->core_files.core_trace_F, "%d ", core->statistics.cycles);
	Pipeline_WriteToTrace(&core->pipeline, core->core_files.core_trace_F);
	write_regs_to_file(core, regs_copy);
	fprintf(core->core_files.core_trace_F, "\n");

}

/*!
******************************************************************************
\brief
Writing the registers to file.
\param
 [in] core				  - the operating core.
 [in] uint32_t *regs_copy - pointer to the regs copied values.
\return none
*****************************************************************************/
static void write_regs_to_file(Core_s* core, uint32_t* regs_copy)
{
	//first two registers are not for write
	for (int i = 2; i < NUMBER_OF_REGISTERS; i++)
	{
		fprintf(core->core_files.core_trace_F, "%08X ", regs_copy[i]);
	}
}

/*!
******************************************************************************
\brief
Updating staistics struct.
\param
 [in] core - the operating core.
\return none
*****************************************************************************/
static void update_statistics(Core_s* core)
{
	core->statistics.cycles++;

	if (!core->pipeline.halted && !core->pipeline.memory_stall && !core->pipeline.data_hazard_stall)
		core->statistics.instructions++;
}

/*!
******************************************************************************
\brief
Printing the registers file.
\param
 [in] core - the operating core.
\return none
*****************************************************************************/
static void print_register_file(Core_s* core)
{
	//first two registers arent for printing
	for (int i = 2; i < NUMBER_OF_REGISTERS; i++)
	{
		fprintf(core->core_files.regout_F, "%08X\n", core->register_array[i]);
	}
}

static void print_statistics(Core_s* core)
{
	fprintf(core->core_files.StatsFile, "cycles %d\n", core->statistics.cycles + 1);
	fprintf(core->core_files.StatsFile, "instructions %d\n", core->statistics.instructions - 1);
	fprintf(core->core_files.StatsFile, "read_hit %d\n", core->pipeline.cache_data.statistics.read_hits);
	fprintf(core->core_files.StatsFile, "write_hit %d\n", core->pipeline.cache_data.statistics.write_hits);
	fprintf(core->core_files.StatsFile, "read_miss %d\n", core->pipeline.cache_data.statistics.read_misses);
	fprintf(core->core_files.StatsFile, "write_miss %d\n", core->pipeline.cache_data.statistics.write_misses);
	fprintf(core->core_files.StatsFile, "decode_stall %d\n", core->pipeline.statistics.decode_stalls);
	fprintf(core->core_files.StatsFile, "mem_stall %d\n", core->pipeline.statistics.mem_stalls);
}
