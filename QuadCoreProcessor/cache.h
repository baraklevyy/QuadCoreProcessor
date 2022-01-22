#ifndef __CHACHE_H_
#define __CHACHE_H_

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "helper.h"



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
	uint32_t tsram[TSRAM_NUMBER_OF_LINES];
	CacheStatistics_s statistics;
} cache_information;


void Cache_Init(cache_information* data, core_identifier id);
void Cache_RegisterBusHandles(void);
bool Cache_ReadData(cache_information* cache_data, uint32_t address, uint32_t* data);
bool Cache_WriteData(cache_information* cache_data, uint32_t address, uint32_t data);
void Cache_PrintData(cache_information* cache_data, FILE* dsram_file, FILE* tsram_file);


#endif 
