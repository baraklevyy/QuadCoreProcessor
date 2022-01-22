#include "helper.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

//variables and functions declaration
static snoop_function_pointer snoop_func_ptr;
static send_to_memory_function_pointer memory_func_ptr;
static uint8_t offset_of_address;
static shared_function_pointer shared_func_ptr;
static communication_of_bus_cache_info cache_data_for_each_core[CORES_NUMBER];
static cache_answer_function_pointer cache_answer_func_ptr;
static exit_func_code is_bus_operating;
static operation_status core_operating_state[CORES_NUMBER] = { 0, 0, 0, 0 };
static arbitration_linked_list* arbitration_last_element;
static data_on_bus current_data_on_bus;
static uint32_t total_number_of_iterations = 0;
static bool is_cache_snooping(data_on_bus* data);
static void bus_information_print(data_on_bus data);
bool is_linked_list_empty(void);
static arbitration_linked_list* arbitration_first_element;
static bool is_bus_shared_line(data_on_bus* data, bool* changed);
bool push_to_arbitration_line(data_on_bus data);
bool pop_from_arbitration_line(data_on_bus* data);
/// <summary>
/// registering the funtions into cache of different cores
/// </summary>
/// <param name="cache_communication_bus"></param>
void set_cache_bus_commu_func(communication_of_bus_cache_info cache_communication_bus)
{
	cache_data_for_each_core[cache_communication_bus.core_number] = cache_communication_bus;
}
/// <summary>
/// registering functions
/// </summary>
/// <param name="memory_operation"></param>
void set_bus_memory_func(send_to_memory_function_pointer memory_operation)
{
	memory_func_ptr = memory_operation;
}
/// <summary>
/// checking if the linked list is empty
/// </summary>
/// <param name=""></param>
/// <returns>true if the list is empty</returns>
bool is_linked_list_empty(void){
	if (NULL == arbitration_first_element) return true;
	return false;
}
/// <summary>
/// pushing new transaction to the arbitration list - implementing Round Robin
/// </summary>
/// <param name="data"></param>
/// <returns> true on succedd to push</returns>
bool push_to_arbitration_line(data_on_bus data){
	arbitration_linked_list* list_node = malloc(sizeof(arbitration_linked_list));
	if (NULL == list_node) return false;
	list_node->next = NULL;
	list_node->previous = NULL;
	list_node->data = data;
	if (true == is_linked_list_empty()){
		arbitration_first_element = list_node;
		arbitration_last_element = list_node;
	}
	else{
		arbitration_first_element->previous = list_node;
		list_node->next = arbitration_first_element;
		arbitration_first_element = list_node;
	}
	return true;
}
/// <summary>
/// pop out a transaction that going to execute on the bus
/// </summary>
/// <param name="data"></param>
/// <returns></returns>
bool pop_from_arbitration_line(data_on_bus* data){
	if (NULL ==arbitration_last_element) return false;
	arbitration_linked_list* list_node = arbitration_last_element;
	arbitration_last_element = list_node->previous;
	if (NULL == arbitration_last_element) arbitration_first_element = NULL;
	else arbitration_last_element->next = NULL;
	*data = list_node->data;
	list_node->previous = NULL;
	free(list_node);
	return true;
}

/// <summary>
/// pushing new wlwmwnt to arbitration queue
/// </summary>
/// <param name="bus_data"></param>
void push_new_bus_operation(data_on_bus bus_data){
	push_to_arbitration_line(bus_data);
	if (err_originator_on_bus == bus_data.origid_on_bus) return;
	core_operating_state[bus_data.origid_on_bus] = ready_state;
}
/// <summary>
/// checking if the bus is busy
/// </summary>
/// <param name="originator"></param>
/// <returns>true is busy</returns>
bool is_bus_busy(orig_id_on_bus originator){
	if (true == idle_cmd_on_bus != *(core_operating_state + originator)) return true;
	return false;
}
/// <summary>
/// checking if some core is waiting to execute on the bus
/// </summary>
/// <param name="originator"></param>
/// <returns>true if the core is waiting</returns>
bool is_bus_waiting_for_operate(orig_id_on_bus originator){
	if (ready_state == *(core_operating_state + originator)) return true;
	return false;
}
/// <summary>
/// actucal bus operation
/// </summary>
/// <param name=""></param>
void operate_bus(void){
	static bool flag_for_initial_shared = true; //set to true for the first time
	total_number_of_iterations += 1;
	if (final_state == core_operating_state[current_data_on_bus.origid_on_bus]) *(core_operating_state + current_data_on_bus.origid_on_bus) = idle_status;
	if (true == is_linked_list_empty() && false == is_bus_operating){
		current_data_on_bus.origid_on_bus = err_originator_on_bus;
		return;
	}
	if (false == is_bus_operating){
		flag_for_initial_shared = true;
		if (false == pop_from_arbitration_line(&current_data_on_bus) || err_originator_on_bus == current_data_on_bus.origid_on_bus) return;
		current_data_on_bus.first_send_on_bus = current_data_on_bus.origid_on_bus;
		is_bus_operating = !is_bus_operating;
		*(core_operating_state + current_data_on_bus.origid_on_bus) = operate_state;
		offset_of_address = 0;
		bus_information_print(current_data_on_bus);
	}
	data_on_bus data;
	data = current_data_on_bus;
	mem_address address;
	address.address = current_data_on_bus.address_on_bus;
	set_offset_to_address(&address, offset_of_address);
	data.address_on_bus = address.address;
	bool changed;
	changed = false;
	data.is_bus_shared = is_bus_shared_line(&current_data_on_bus, &changed);
	if (true == flag_for_initial_shared && true == changed){
		flag_for_initial_shared = !flag_for_initial_shared;
		return;
	}
	is_cache_snooping(&data);
	if (memory_func_ptr(&data, changed)){
		bus_information_print(data);
		if (true == (cache_answer_func_ptr(cache_data_for_each_core[current_data_on_bus.origid_on_bus].data_from_cache, &data, &offset_of_address))){
			*(core_operating_state + current_data_on_bus.origid_on_bus) = final_state;
			is_bus_operating = false;
		}
	}
}
/// <summary>
/// this is the function helps for determined is the block from cache will be titled as shared
/// </summary>
/// <param name="data"></param>
/// <param name="changed"></param>
/// <returns>true if other cores has this block</returns>
static bool is_bus_shared_line(data_on_bus* data, bool* changed){
	bool shared = false;
	for (int i = 0; i < CORES_NUMBER; i++) {
		shared = shared | shared_func_ptr(cache_data_for_each_core[i].data_from_cache, data, changed);
		if (true == shared) return true;
	}
	return false;
}

/// <summary>
/// cache snoopin function
/// </summary>
/// <param name="data"></param>
/// <returns>true if there is answer from cache.</returns>
static bool is_cache_snooping(data_on_bus* data){
	bool answer_from_cache = false;
	for (int i = 0; i < CORES_NUMBER; i++) {
		answer_from_cache = answer_from_cache | snoop_func_ptr(cache_data_for_each_core[i].data_from_cache, data, offset_of_address);
		if (true == answer_from_cache) return true;
	}
	return false;
}
/// <summary>
/// printing the bus information to the bustrace file
/// </summary>
/// <param name="data"></param>
static void bus_information_print(data_on_bus data){
	fprintf(bus_trace_file, "%d %d %d %05X %08X %d\n", total_number_of_iterations, data.origid_on_bus, data.command_on_bus, data.address_on_bus, data.data_on_bus, data.is_bus_shared);
}

void set_cache_shared_function(shared_function_pointer shared) {
	shared_func_ptr = shared;
}
void set_bus_snoop_function(snoop_function_pointer snoop) {
	snoop_func_ptr = snoop;
}
void set_cache_answer_function(cache_answer_function_pointer answer) {
	cache_answer_func_ptr = answer;
}