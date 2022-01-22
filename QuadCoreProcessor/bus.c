#include "helper.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>


static communication_of_bus_cache_info gCacheInterface[CORES_NUMBER];
static shared_function_pointer shared_func_ptr;
static snoop_function_pointer snoop_func_ptr;
static cache_answer_function_pointer cache_answer_func_ptr;
static send_to_memory_function_pointer memory_func_ptr;
static bool is_bus_operating;
static operation_status core_operating_state[CORES_NUMBER] = { 0, 0, 0, 0 };
static data_on_bus current_data_on_bus;
static uint8_t offset_of_address;
static uint32_t total_number_of_iterations = 0;
static arbitration_linked_list* arbitration_first_element;
static arbitration_linked_list* arbitration_last_element;


static bool is_bus_shared_line(data_on_bus* data, bool* changed);
static bool is_cache_snooping(data_on_bus* data);
static void bus_information_print(data_on_bus data);

// Fifo functions
bool is_linked_list_empty(void);
bool push_to_arbitration_line(data_on_bus data);
bool pop_from_arbitration_line(data_on_bus* data);

/************************************
*       API implementation          *
************************************/
void set_cache_bus_commu_func(communication_of_bus_cache_info cache_communication_bus)
{
	gCacheInterface[cache_communication_bus.core_number] = cache_communication_bus;
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

void set_bus_memory_func(send_to_memory_function_pointer memory_operation)
{
	memory_func_ptr = memory_operation;
}

void push_new_bus_operation(data_on_bus bus_data)
{
	push_to_arbitration_line(bus_data);
	if (err_originator_on_bus == bus_data.origid_on_bus) return;
	core_operating_state[bus_data.origid_on_bus] = ready_state;
}

bool is_bus_busy(orig_id_on_bus originator)
{
	return core_operating_state[originator] != idle_status;
}

bool is_bus_waiting_for_operate(orig_id_on_bus originator)
{
	return core_operating_state[originator] == ready_state;
}

void operate_bus(void)
{
	static bool is_first_shared = true;
	total_number_of_iterations++;

	if (core_operating_state[current_data_on_bus.origid_on_bus] == final_state)
		core_operating_state[current_data_on_bus.origid_on_bus] = idle_status;

	if (is_linked_list_empty() && !is_bus_operating)
	{
		current_data_on_bus.origid_on_bus = err_originator_on_bus;
		return;
	}

	if (!is_bus_operating)
	{
		is_first_shared = true;
		int prev_origid = current_data_on_bus.origid_on_bus;

		if (!pop_from_arbitration_line(&current_data_on_bus) || current_data_on_bus.origid_on_bus == err_originator_on_bus)
			return;

		current_data_on_bus.first_send_on_bus = current_data_on_bus.origid_on_bus;

		is_bus_operating = true;
		core_operating_state[current_data_on_bus.origid_on_bus] = operate_state;
		offset_of_address = 0;
		// print bus trace
		printf("bus trace - (#%d) dequeue next cmd\n", total_number_of_iterations);
		bus_information_print(current_data_on_bus);
	}

	data_on_bus data;
	memcpy(&data, &current_data_on_bus, sizeof(current_data_on_bus));
	memory_addess_s address;
	address.address = current_data_on_bus.address_on_bus;
	//address.offset = offset_of_address;
	set_offset_to_address(&address, offset_of_address);
	data.address_on_bus = address.address;

	bool changed = false;
	data.is_bus_shared = is_bus_shared_line(&current_data_on_bus, &changed);
	if (changed && is_first_shared)
	{
		is_first_shared = false;
		return;
	}

	bool cache_response = is_cache_snooping(&data);
	bool memory_response = memory_func_ptr(&data, changed);

	if (memory_response)
	{
		// print response trace.
		printf("bus trace - (#%d) response to sender\n", total_number_of_iterations);
		bus_information_print(data);

		// send the response data back to the sender
		if (cache_answer_func_ptr(gCacheInterface[current_data_on_bus.origid_on_bus].data_from_cache, &data, &offset_of_address))
		{
			core_operating_state[current_data_on_bus.origid_on_bus] = final_state;
			is_bus_operating = false;
		}
	}
}


/************************************
* static implementation             *
************************************/
static bool is_bus_shared_line(data_on_bus* data, bool* changed)
{
	bool shared = false;

	for (int i = 0; i < CORES_NUMBER; i++)
		shared |= shared_func_ptr(gCacheInterface[i].data_from_cache, data, changed);

	return shared;
}

static bool is_cache_snooping(data_on_bus* data)
{
	bool cache_response = false;
	for (int i = 0; i < CORES_NUMBER; i++)
		cache_response |= snoop_func_ptr(gCacheInterface[i].data_from_cache, data, offset_of_address);

	return cache_response;
}

static void bus_information_print(data_on_bus data)
{
	fprintf(BusTraceFile, "%d %d %d %05X %08X %d\n", total_number_of_iterations, data.origid_on_bus, data.command_on_bus,
		data.address_on_bus, data.data_on_bus, data.is_bus_shared);
}


// Fifo implemantation
bool is_linked_list_empty(void)
{
	return arbitration_first_element == NULL;
}

bool push_to_arbitration_line(data_on_bus data)
{
	arbitration_linked_list* queue_item = malloc(sizeof(arbitration_linked_list));
	if (queue_item == NULL)
		return false;

	// initiate data
	queue_item->data = data;
	queue_item->next = NULL;
	queue_item->previous = NULL;

	if (is_linked_list_empty())
	{
		arbitration_first_element = queue_item;
		arbitration_last_element = queue_item;
	}
	else
	{
		arbitration_first_element->previous = queue_item;
		queue_item->next = arbitration_first_element;
		arbitration_first_element = queue_item;
	}
	return true;
}

bool pop_from_arbitration_line(data_on_bus* data)
{
	if (arbitration_last_element == NULL)
		return false;

	arbitration_linked_list* queue_item = arbitration_last_element;
	arbitration_last_element = queue_item->previous;

	if (arbitration_last_element == NULL)
		arbitration_first_element = NULL;
	else
		arbitration_last_element->next = NULL;

	queue_item->previous = NULL;
	*data = queue_item->data;

	free(queue_item);
	return true;
}