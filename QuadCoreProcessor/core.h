#ifndef CORE_H
#define CORE_H

#include <stdint.h>
#include "pipline.h"
#include "helper.h"



#define INSTRUCTIONS_MEMORY_SIZE 1024

typedef struct
{
	uint32_t cycles;
	uint32_t instructions;
}Statistics_s;

typedef struct
{
	bool core_halted;
	uint8_t index;
	uint16_t program_counter;	// pc is 10bit
	uint32_t register_array[NUMBER_OF_REGISTERS];
	uint32_t instructions_memory_image[INSTRUCTIONS_MEMORY_SIZE];
	output_core_file core_files;
	Pipeline_s pipeline;
	Statistics_s statistics;
}Core_s;

void Core_Init(Core_s* core, uint8_t id);
void Core_Iter(Core_s* core);
void Core_Teaddown(Core_s* core);
bool Core_Halted(Core_s* core);


#endif

