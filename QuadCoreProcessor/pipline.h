#ifndef __PIPELINE_H__
#define __PIPELINE_H__

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "helper.h"



typedef struct
{
	uint32_t* current_core_regs;
	cache_information current_data_from_cache;
	uint32_t* pointer_to_instruction;
	current_pipeline_information pipe_stages[PIPELINE_SIZE];
	parameters_to_command opcode_params;
	bool is_pip_halt;
	uint32_t current_pip_decode_stalls;
	uint32_t current_pip_memory_stalls;
	bool is_data_stall;
	bool is_mem_stall;
}pipe_data;

void set_pip(pipe_data* pip);
void add_idle_slot(pipe_data* pip);
bool reg_compare_logic(uint32_t instruction, uint16_t reg, uint16_t op_code, uint16_t op_write_);
void initialize_pip(pipe_data* pip);
bool reg_compare_helper(uint16_t reg1, uint16_t reg2);
bool is_flush_require(pipe_data* pip, int phase);
void tracing_pip(pipe_data* pip, FILE* f);
bool flush_the_pipe(pipe_data* pip);


#endif