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


uint16_t get_tsram_tag(uint32_t tsram) {
	uint32_t mask = 0x00000fff;
	uint16_t tag = tsram & mask;
	return tag;
}

uint16_t get_tsram_mesi_state(uint32_t tsram) {
	uint32_t mask = 0x00003000; //all zeros instead the 12,13 bits locations
	uint16_t mesi_state = tsram & mask;
	mesi_state = (mesi_state >> 12); //need to shift the bits back to the lsb in order to isolate the mesi state
	return mesi_state;
}

void set_tag_to_tsram(uint32_t* tsram, uint16_t tag) {
	*tsram = (*tsram) & (0xfffff000); //make 12 LSB into 0 and keep the rest the same;
	uint16_t mask = 0x0fff; // this mask is 12LSB as 0
	mask = mask & tag; //take the 12 LSB of tag
	*tsram = (*tsram) | mask;
}

void set_mesi_state_to_tsram(uint32_t* tsram, uint16_t mesi_state) {
	*tsram = (*tsram) & (0xffffcfff); //set 0 the bits in location 12, 13 - corresponding the the mesi satate in the tsram memory
	uint16_t mask = 0x00000003; // set 1 in 12, 13 places
	mask = mask & mesi_state;
	mask = (mask << 12);
	*tsram = (*tsram) | mask;
}