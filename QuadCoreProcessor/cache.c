#include "helper.h"
#include <string.h>
mesi_state current_shared_state(cache_information* data, data_on_bus* data_container_from_bus);
bool check_existance_of_block(cache_information* data, data_on_bus* data_container_from_bus, uint8_t address_offset);
mesi_state current_modified_state(cache_information* data, data_on_bus* data_container_from_bus);
mesi_state current_exclusive_state(cache_information* data, data_on_bus* data_container_from_bus);
bool shared_block_check(cache_information* data, data_on_bus* data_container_from_bus, bool* changed);
bool cache_data_answer(cache_information* data, data_on_bus* data_container_from_bus, uint8_t* address_offset);
mesi_state current_invalid_state(cache_information* data, data_on_bus* data_container_from_bus);
typedef mesi_state(*get_mesi_block_state_from_bus_snoop)(cache_information* data, data_on_bus* data_container_from_bus);
void operate_dirty_block(cache_information* data, uint32_t addr);
get_mesi_block_state_from_bus_snoop from_what_mesi_state_operate[4] = {current_invalid_state, current_shared_state, current_exclusive_state, current_modified_state};


void initialize_the_cache(cache_information* data, core_identifier id){
	data->id = id;
	communication_of_bus_cache_info cache_communication_bus;
	cache_communication_bus.data_from_cache = data;
	cache_communication_bus.core_number = id;
	set_cache_bus_commu_func(cache_communication_bus);
}
exit_func_code read_from_cache(cache_information* cache_data, uint32_t address, uint32_t* data){
	static bool data_is_missing = false;
	if ((true == is_bus_busy(cache_data->id)) || (true == is_bus_waiting_for_operate(cache_data->id))) return failed_op;
	uint32_t addr;
	addr = address;
	uint32_t cache_tsram_memory;
	cache_tsram_memory  = cache_data->tsram[get_cache_address_index(addr)];
	if (get_tsram_tag(cache_tsram_memory) == get_cache_address_tag(addr) && get_tsram_mesi_state(cache_tsram_memory) != invalid){//hit
		uint16_t index;
		index = (BLOCK_SIZE) * (get_cache_address_index(addr)) + get_cache_address_offset(addr);
		*data = cache_data->dsram[index];
		if (false == data_is_missing) cache_data->number_of_read_hit = cache_data->number_of_read_hit + 1; //flag should stay false since we had hit
		else data_is_missing = false;//previous op was miss, now we change the flag since we had hit
		return success_op;
	}
	//this is not a hit, so we have miss on the data from cache
	data_is_missing = true;
	cache_data->number_of_read_miss = cache_data->number_of_read_miss + 1;
	operate_dirty_block(cache_data, addr); //we had miss, if the block is dirty we have to write it into memory
	data_on_bus data_container_from_bus;
	data_container_from_bus.is_bus_shared = false;
	data_container_from_bus.data_on_bus = (uint32_t*)0;
	data_container_from_bus.origid_on_bus = cache_data->id;
	data_container_from_bus.command_on_bus = bus_read_cmd_on_bus;
	data_container_from_bus.address_on_bus = addr;
	push_new_bus_operation(data_container_from_bus);
	return failed_op;
}
/// <summary>
/// writing into the cache and dealing with cache coherence using snoop and mesi algorithms
/// </summary>
/// <param name="cache_data"></param>
/// <param name="address"></param>
/// <param name="data"></param>
/// <returns>success_op is write to bus success</returns>
exit_func_code write_to_cache(cache_information* cache_data, uint32_t address, uint32_t data){
	static bool data_is_missing = false;
	if ((true == is_bus_busy(cache_data->id)) || (true == is_bus_waiting_for_operate(cache_data->id))) return failed_op;
	uint32_t addr;
	addr = address;
	uint32_t* tsram;
	tsram = &(cache_data->tsram[get_cache_address_index(addr)]);
	if (get_tsram_tag(*tsram) == get_cache_address_tag(addr) && get_tsram_mesi_state(*tsram) != invalid) { //check for equalitiy of tags and not invalid state - check for hit
		if (shared != get_tsram_mesi_state(*tsram)) { //hit and no shared data
			if (false == data_is_missing) cache_data->number_of_write_hit = cache_data->number_of_write_hit + 1;//we have write hit and the block is exclusive, so we can just change it
			else data_is_missing = false;
			uint16_t index = (BLOCK_SIZE) * (get_cache_address_index(addr)) + get_cache_address_offset(addr);
			cache_data->dsram[index] = data;
			uint32_t temp;
			temp = cache_data->tsram[get_cache_address_index(addr)];
			set_mesi_state_to_tsram(&temp, (uint16_t*)modified); //now this block is currently dirty so setting it to exclusive state
			cache_data->tsram[get_cache_address_index(addr)] = temp;
			return success_op;
		}
		if (shared == get_tsram_mesi_state(*tsram)) { //if the block is shared we need to implement bus_read transaction on the bus, also dealing it as a data miss
			data_is_missing = true;
			cache_data->number_of_write_miss = cache_data->number_of_write_miss + 1;
			//push bus_read_exclusive since we are writing new data
			data_on_bus data_container_from_bus;
			data_container_from_bus.is_bus_shared = false;
			data_container_from_bus.data_on_bus = (uint32_t*)0;
			data_container_from_bus.origid_on_bus = cache_data->id;
			data_container_from_bus.command_on_bus = bus_read_exclusive_cmd_on_bus;
			data_container_from_bus.address_on_bus = addr;
			push_new_bus_operation(data_container_from_bus);
			data_on_bus delaying_next_request_container;
			delaying_next_request_container.origid_on_bus = err_originator_on_bus; //we need to delay the next operation in one cycle so pushing no-operation container to the bus
			push_new_bus_operation(delaying_next_request_container);
			return failed_op;
		}
	}
	data_is_missing = true; //the tags arent equal so we have miss
	cache_data->number_of_write_miss = cache_data->number_of_write_miss + 1;
	operate_dirty_block(cache_data, addr); //if the block is dirty, we have to write it into main memory
	data_on_bus data_container_from_bus;
	data_container_from_bus.is_bus_shared = false;
	data_container_from_bus.data_on_bus = (uint32_t*)0;
	data_container_from_bus.origid_on_bus = cache_data->id;
	data_container_from_bus.command_on_bus = bus_read_exclusive_cmd_on_bus;
	data_container_from_bus.address_on_bus = addr;
	push_new_bus_operation(data_container_from_bus);
	return failed_op;
}

void cache_print_to_file(cache_information* cache_data, FILE* dsram_f, FILE* tsram_f){
	for (uint32_t i = 0; i < CACHE_SIZE; i++) {
		uint32_t* tmp = cache_data->dsram;
		fprintf(dsram_f, "%08X\n", *(tmp + i));
	}
	for (uint32_t i = 0; i < TSRAM_NUMBER_OF_LINES; i++) {
		uint32_t* tmp = cache_data->tsram;
		fprintf(tsram_f, "%08X\n", *(tmp + i));
	}
}
mesi_state current_invalid_state(cache_information* data, data_on_bus* data_container_from_bus){
	return invalid;
}
mesi_state current_shared_state(cache_information* data, data_on_bus* data_container_from_bus){
	if (data_container_from_bus->command_on_bus == bus_read_exclusive_cmd_on_bus) return invalid;
	return shared;
}
mesi_state current_exclusive_state(cache_information* data, data_on_bus* data_container_from_bus){
	
	if (data_container_from_bus->command_on_bus == bus_read_cmd_on_bus) return shared;
	if (data_container_from_bus->command_on_bus == bus_read_exclusive_cmd_on_bus) return invalid;
	return exclusive;
}
mesi_state current_modified_state(cache_information* data, data_on_bus* data_container_from_bus) {
	uint32_t address;
	address = data_container_from_bus->address_on_bus;
	uint16_t ind;
	ind = (uint16_t*)(get_cache_address_index(address) * BLOCK_SIZE + get_cache_address_offset(address));
	if (data_container_from_bus->command_on_bus == bus_read_cmd_on_bus){
		data_container_from_bus->data_on_bus = data->dsram[ind];
		data_container_from_bus->command_on_bus = bus_flush_cmd_on_bus;
		data_container_from_bus->origid_on_bus = data->id;
		return shared;
	}
	else if (data_container_from_bus->command_on_bus == bus_read_exclusive_cmd_on_bus){
		data_container_from_bus->origid_on_bus = data->id;
		data_container_from_bus->command_on_bus = bus_flush_cmd_on_bus;
		data_container_from_bus->data_on_bus = data->dsram[ind];
		return invalid;
	}
	else if (data_container_from_bus->command_on_bus == bus_flush_cmd_on_bus){
		data_container_from_bus->data_on_bus = data->dsram[ind];
		data_container_from_bus->command_on_bus = bus_flush_cmd_on_bus;
		data_container_from_bus->origid_on_bus = data->id;
		return modified;
	}
}

bool shared_block_check(cache_information* data, data_on_bus* data_container_from_bus, bool* changed){
	if (data->id == data_container_from_bus->origid_on_bus) return false; //if this is a self-data
	uint32_t address = data_container_from_bus->address_on_bus;
	uint32_t* tsram = &(data->tsram[get_cache_address_index(address)]);
	*changed = *changed | (modified == get_tsram_mesi_state(*tsram));
	return get_tsram_tag(*tsram) == get_cache_address_tag(address) && get_tsram_mesi_state(*tsram) != invalid;
}
void operate_dirty_block(cache_information* data, uint32_t addr){
	if (modified == get_tsram_mesi_state(data->tsram[get_cache_address_index(addr)])){
		uint32_t block_addr = 0x00000000;
		set_index_to_cache_address(&block_addr, get_cache_address_index(addr));
		uint32_t tmp = 0x00000000;
		tmp = get_tsram_tag(data->tsram[get_cache_address_index(addr)]);
		set_tag_to_cache_address(&block_addr, tmp);
		set_offset_to_cache_address(&block_addr, 0x00000000);
		data_on_bus data_container_from_bus;
		data_container_from_bus.is_bus_shared = false;
		data_container_from_bus.origid_on_bus = data->id;
		data_container_from_bus.command_on_bus = bus_flush_cmd_on_bus;
		data_container_from_bus.address_on_bus = block_addr;
		uint16_t index = get_cache_address_index(addr) * BLOCK_SIZE + get_cache_address_offset(addr);
		data_container_from_bus.data_on_bus = data->dsram[index];
		push_new_bus_operation(data_container_from_bus); //push the flush operation into arbitration linked list
	}
}
bool check_existance_of_block(cache_information* data, data_on_bus* data_container_from_bus, uint8_t address_offset){
	if (data->id == data_container_from_bus->first_send_on_bus && data_container_from_bus->command_on_bus != bus_flush_cmd_on_bus) return false; //sef-data
	uint32_t address = data_container_from_bus->address_on_bus;
	uint32_t* tsram = &(data->tsram[get_cache_address_index(address)]);
	if (get_tsram_tag(*tsram) != get_cache_address_tag(address) || get_tsram_mesi_state(*tsram) == invalid) return false; //the block is not in cache
	mesi_state next_state = from_what_mesi_state_operate[get_tsram_mesi_state(*tsram)](data, data_container_from_bus); //changing the state of block regarding the MESI algorithm
	if (address_offset == (BLOCK_SIZE - 1) || get_tsram_mesi_state(*tsram) != modified) set_mesi_state_to_tsram(tsram, (uint16_t*)next_state);
	return true;
}
static bool cache_data_answer(cache_information* data, data_on_bus* data_container_from_bus, uint8_t* address_offset){
	if (data->id == data_container_from_bus->origid_on_bus && data_container_from_bus->command_on_bus != bus_flush_cmd_on_bus) return false; //self data
	else if (data->id == data_container_from_bus->origid_on_bus && data_container_from_bus->command_on_bus == bus_flush_cmd_on_bus){
		if (*address_offset == (BLOCK_SIZE - 1)) return true;
		*address_offset += 1;
		return false;
	}
	uint32_t address = data_container_from_bus->address_on_bus;
	uint32_t* tsram = &(data->tsram[get_cache_address_index(address)]);
	set_tag_to_tsram(tsram, (uint16_t*)get_cache_address_tag(address));//new tag
	if (bus_flush_cmd_on_bus == data_container_from_bus->command_on_bus){
		uint16_t index = (uint16_t*)(get_cache_address_index(address) * BLOCK_SIZE + get_cache_address_offset(address));
		data->dsram[index] = data_container_from_bus->data_on_bus;
	}
	if (*address_offset == (BLOCK_SIZE - 1)) {
		if (true == data_container_from_bus->is_bus_shared) set_mesi_state_to_tsram(tsram, (uint16_t*)shared);
		else set_mesi_state_to_tsram(tsram, (uint16_t*)exclusive);
		return true;
	}
	*address_offset += 1;
	return false;
}
//setting up functions to operate that communicate with bus
void set_cache_answer(void) {
	set_cache_answer_function(cache_data_answer);
}
void set_cache_shared_func(void) {
	set_cache_shared_function(shared_block_check);
}
void set_cache_snoop_function(void) {
	set_bus_snoop_function(check_existance_of_block);
}



