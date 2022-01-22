#define _CRT_SECURE_NO_WARNINGS
#include "core.h"
#include <stdio.h>
#include <string.h>
static void fill_regs_files(data_of_core* core, uint32_t* regs_copy);
bool pipeline_is_legal(data_of_core* core);

void initialize_core(data_of_core* core, uint8_t id){
	int number_of_lines = 0;
	while (number_of_lines < SIZE_OF_INST && fscanf(core->core_files.immediate_memory_file, "%08x", (uint32_t*)&(core->data_of_instruction[number_of_lines])) != -1) number_of_lines = number_of_lines + 1;
	if (0 == number_of_lines){
		core->is_core_terminated = true;
		return;
	}
	core->index = id;
	core->pc_of_core = 0;
	core->is_core_terminated = false;
	core->number_of_cycles = core->number_of_cycles - 1;
	initialize_pip(&core->pipeline);
	initialize_the_cache(&core->pipeline.current_data_from_cache, id);
	set_cache_answer();
	set_cache_snoop_function();
	set_cache_shared_func();
	core->pipeline.pointer_to_instruction = core->data_of_instruction;
	core->pipeline.current_core_regs = core->data_register;
	core->pipeline.opcode_params.pc = &(core->pc_of_core);
}
bool pipeline_is_legal(data_of_core* core) {
	return (!core->pipeline.is_pip_halt && !core->pipeline.is_mem_stall && !core->pipeline.is_data_stall);
}
void operate_the_core(data_of_core* core){
	if (true == core->is_core_terminated) return;
	if (true == flush_the_pipe(&core->pipeline)){
		core->is_core_terminated = true;
		return;
	}
	uint32_t regs_copy[NUMBER_OF_REGISTERS];
	memcpy(regs_copy, core->data_register, sizeof(core->data_register));
	core->number_of_cycles = core->number_of_cycles + 1;
	if (pipeline_is_legal(core)) core->number_of_instructions = core->number_of_instructions + 1;
	set_pip(&core->pipeline, core->index);
	fprintf(core->core_files.core_trace_files, "%d ", core->number_of_cycles);
	tracing_pip(&core->pipeline, core->core_files.core_trace_files);
	fill_regs_files(core, regs_copy);
	fprintf(core->core_files.core_trace_files, "\n");
	add_idle_slot(&core->pipeline);
}
static void fill_regs_files(data_of_core* core, uint32_t* regs_copy){
	for (int i = 2; i < NUMBER_OF_REGISTERS; i++){
		fprintf(core->core_files.core_trace_files, "%08X ", *(regs_copy + i));
	}
}
