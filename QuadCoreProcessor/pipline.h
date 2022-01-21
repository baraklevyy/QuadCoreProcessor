#ifndef __PIPELINE_H__
#define __PIPELINE_H__

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include "Cache.h"
#include "helper.h"



typedef enum
{
	FETCH = 0,
	DECODE,
	EXECUTE,
	MEM,
	WRITE_BACK
} state_of_pipeline;

typedef struct
{
	state_of_pipeline state;
	uint32_t instruction;
	uint32_t execution_output;
	void (*operation)(parameters_to_command* arguments_to_cmd);
	uint16_t pc;
} current_pipeline_information;


typedef struct
{
	uint32_t* insturcionts_p;
	uint32_t* current_core_regs;
	cache_information current_data_from_cache;
	current_pipeline_information pipe_stages[PIPELINE_SIZE];
	parameters_to_command opcode_params;
	uint32_t current_pip_decode_stalls;
	uint32_t current_pip_memory_stalls;
	bool is_pip_halt;
	bool is_data_stall;
	bool is_mem_stall;
}Pipeline_s;

void initialize_pip(Pipeline_s* pipeline);
void Pipeline_Execute(Pipeline_s* pipeline);
void tracing_pip(Pipeline_s* pipeline, FILE* trace_file);
bool reg_compare_logic(uint32_t instruction, uint16_t reg, uint16_t op_code, uint16_t op_write_);
bool reg_compare_helper(uint16_t reg1, uint16_t reg2);
bool is_flush_require(Pipeline_s* pip, int phase);
void add_idle_slot(Pipeline_s* pipeline);
bool flush_the_pipe(Pipeline_s* pipeline);


#endif