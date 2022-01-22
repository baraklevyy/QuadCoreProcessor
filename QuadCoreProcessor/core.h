#ifndef CORE_H
#define CORE_H
#include <stdint.h>
#include "pipline.h"
#include "helper.h"
/// <summary>
/// core_struct that store all the necessary data for single core
/// </summary>
typedef struct{
	uint32_t number_of_instructions;
	uint32_t number_of_cycles;
	bool is_core_terminated;
	uint8_t index;
	uint16_t pc_of_core;
	uint32_t data_register[NUMBER_OF_REGISTERS];
	pipe_data pipeline;
	uint32_t data_of_instruction[SIZE_OF_INST];
	current_core_data_files core_files;
}data_of_core;
//functions declarations
void initialize_core(data_of_core* core, uint8_t id);
void operate_the_core(data_of_core* core);

#endif

