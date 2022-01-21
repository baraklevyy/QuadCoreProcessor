#define _CRT_SECURE_NO_WARNINGS
#include "core.h"
#include <stdio.h>
#include <string.h>
#include "helper.h"



void add (parameters_to_command* arguments_to_cmd)
{
	*(arguments_to_cmd->rd) = ((arguments_to_cmd->rs)) + ((arguments_to_cmd->rt));
}

void sub (parameters_to_command* arguments_to_cmd)
{
	*(arguments_to_cmd->rd) = ((arguments_to_cmd->rs)) - ((arguments_to_cmd->rt));
}

void and (parameters_to_command* arguments_to_cmd)
{
	*(arguments_to_cmd->rd) = (arguments_to_cmd->rs) & (arguments_to_cmd->rt);
}


void or (parameters_to_command * arguments_to_cmd)
{
	*(arguments_to_cmd->rd) = (arguments_to_cmd->rs) | (arguments_to_cmd->rt);
}

void xor (parameters_to_command* arguments_to_cmd)
{
	*(arguments_to_cmd->rd) = (arguments_to_cmd->rs) ^ (arguments_to_cmd->rt);
}


void mul(parameters_to_command* arguments_to_cmd)
{
	*(arguments_to_cmd->rd) = (arguments_to_cmd->rs) * (arguments_to_cmd->rt);
}

void sll (parameters_to_command* arguments_to_cmd)
{
	*(arguments_to_cmd->rd) = (arguments_to_cmd->rs) << (arguments_to_cmd->rt);
}


void sra (parameters_to_command* arguments_to_cmd)
{
	*(arguments_to_cmd->rd) = (int)(arguments_to_cmd->rs) >> (int)(arguments_to_cmd->rt);
}


void srl (parameters_to_command* arguments_to_cmd)
{
	*(arguments_to_cmd->rd) = (arguments_to_cmd->rs) >> (arguments_to_cmd->rt);
}

void beq (parameters_to_command* arguments_to_cmd)
{
	if ((arguments_to_cmd->rs) == (arguments_to_cmd->rt))
	{
		*(arguments_to_cmd->pc) = (uint16_t)(*(arguments_to_cmd->rd) & 0x1FF);
	}
}

void bne (parameters_to_command* arguments_to_cmd)
{
	if ((arguments_to_cmd->rs) != (arguments_to_cmd->rt))
	{
		*(arguments_to_cmd->pc) = (uint16_t)(*(arguments_to_cmd->rd) & 0x1FF);
	}
}


void blt (parameters_to_command* arguments_to_cmd)
{
	if ((arguments_to_cmd->rs) < (arguments_to_cmd->rt))
	{
		*(arguments_to_cmd->pc) = (uint16_t)(*(arguments_to_cmd->rd) & 0x1FF);
	}
}

void bgt (parameters_to_command* arguments_to_cmd)
{
	if ((arguments_to_cmd->rs) > (arguments_to_cmd->rt))
	{
		*(arguments_to_cmd->pc) = (uint16_t)(*(arguments_to_cmd->rd) & 0x1FF);
	}
}

void ble (parameters_to_command* arguments_to_cmd)
{
	if ((arguments_to_cmd->rs) <= (arguments_to_cmd->rt))
	{
		*(arguments_to_cmd->pc) = (uint16_t)(*(arguments_to_cmd->rd) & 0x1FF);
	}
}

void bge (parameters_to_command* arguments_to_cmd)
{
	if ((arguments_to_cmd->rs) >= (arguments_to_cmd->rt))
	{
		*(arguments_to_cmd->pc) = (uint16_t)(*(arguments_to_cmd->rd) & 0x1FF);
	}
}

void jal (parameters_to_command* arguments_to_cmd)
{
	*(arguments_to_cmd->rd) = *(arguments_to_cmd->pc);
	*(arguments_to_cmd->pc) = (uint16_t)(*(arguments_to_cmd->rd) & 0x1FF);
}

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


/*
typedef union
{
	uint32_t address;

	struct
	{
		uint32_t offset : 2;	// [0:1]
		uint32_t index : 6;	// [2:7]
		uint32_t tag : 12;	// [8:19]
	} as_bits;
} cache_addess_s;
*/


uint32_t get_cache_address_offset(uint32_t cache) {
	uint32_t mask = 0x00000003;
	uint32_t offset = cache & mask;
	return offset;
}

uint32_t get_cache_address_index(uint32_t cache) {
	uint32_t mask = 0x000000fc; // setting ones just in the index location [2:7]
	uint32_t index = cache & mask;
	index = (index >> 2); //need to shift the bits back to the lsb in order to isolate the index
	return index;
}

uint32_t get_cache_address_tag(uint32_t cache) {
	uint32_t mask = 0x000fff00; // setting ones just in the tag location [8:19]
	uint32_t tag = cache & mask;
	tag = (tag >> 8); //need to shift the bits back to the lsb in order to isolate the index
	return tag;
}
void set_offset_to_cache_address(uint32_t* cache, uint32_t offset) {
	*cache = (*cache) & (0xfffffffc); //make 2 LSB into 0 and keep the rest the same;
	uint32_t mask = 0x00000003; // this mask is 2LSB as 0
	mask = mask & offset; //take the 2 LSB of offset
	*cache = (*cache) | mask;
}

void set_index_to_cache_address(uint32_t* cache, uint32_t index) {
	*cache = (*cache) & (0xffffff03); //make bit [2:7] into 0 and all the rest to 1
	uint32_t mask = 0x0000003f; // this mask is 6LSB of index
	mask = mask & index; //take the 2 LSB of offset
	mask = (mask << 2);
	*cache = (*cache) | mask;
}
void set_tag_to_cache_address(uint32_t* cache, uint32_t tag) {
	*cache = (*cache) & (0xfff000ff); //make bit [8:19] into 0 and all the rest to 1
	uint32_t mask = 0x00000fff; // this mask is 12LSB of tag
	mask = mask & tag; //take the 2 LSB of offset
	mask = (mask << 8);
	*cache = (*cache) | mask;
}



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
} inst;*/
uint16_t get_command_immediate(uint32_t cmd) {
	uint32_t mask = 0x00000fff; //// setting ones just in the index location [0:11]
	uint16_t immediate = cmd & mask;
	return immediate;
}
uint16_t get_command_rt(uint32_t cmd) {
	uint32_t mask = 0x0000f000; // setting ones just in the index location [12:15]
	uint16_t rt= cmd & mask;
	rt = (rt >> 12); //need to shift the bits back to the lsb in order to isolate rt
	return rt;
}
uint16_t get_command_rs(uint32_t cmd) {
	uint32_t mask = 0x000f0000; // setting ones just in the index location [16:19]
	uint32_t rs = cmd & mask;
	rs = (rs >> 16); //need to shift the bits back to the lsb in order to isolate rs
	return (uint16_t*)rs;
}
uint16_t get_command_rd(uint32_t cmd) {
	uint32_t mask = 0x00f00000; // setting ones just in the index location [20:23]
	uint32_t rd = cmd & mask;
	rd = (rd >> 20); //need to shift the bits back to the lsb in order to isolate rd
	return (uint16_t*)rd;
}
uint16_t get_command_opcode(uint32_t cmd) {
	uint32_t mask = 0xff000000; // setting ones just in the index location [24:31]
	uint32_t opcode = cmd & mask;
	opcode = (opcode >> 24); //need to shift the bits back to the lsb in order to isolate opcode
	return (uint16_t*)opcode;
}