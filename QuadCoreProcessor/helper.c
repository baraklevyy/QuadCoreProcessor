#define _CRT_SECURE_NO_WARNINGS
#include "core.h"
#include <stdio.h>
#include <string.h>
#include "helper.h"





uint16_t get_address_offset(uint32_t address) {
	uint32_t mask = 0x00000003;
	uint16_t offset = address & mask;
	return offset;
}

uint16_t get_address_block(uint32_t address) {
	uint32_t mask = 0x000ffffc;
	uint16_t block = address & mask;
	return block;
}

void set_offset_to_address(uint32_t* address, uint8_t offset) {
	*address = (*address) & (0xfffffffc); //make 8 LSB into 1 and keep the rest the same;
	uint8_t mask = 0x03;
	mask = mask & offset;
	*address = (*address) | mask;
}