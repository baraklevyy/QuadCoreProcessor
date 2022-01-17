#include "cache.h"
#include <string.h>
#include "bus.h"

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

typedef Cache_mesi_e(*snooping_state)(CacheData_s* data, Bus_packet_s* packet);

/************************************
*      static functions             *
************************************/
static void dirty_block_handling(CacheData_s* data, cache_addess_s addr);

// handles
static bool shared_signal_handle(CacheData_s* data, Bus_packet_s* packet, bool* is_modified);
static bool cache_snooping_handle(CacheData_s* data, Bus_packet_s* packet, uint8_t address_offset);
static bool cache_response_handle(CacheData_s* data, Bus_packet_s* packet, uint8_t* address_offset);

// state machine for snooping
static Cache_mesi_e mesi_snooping_invalid_state(CacheData_s* data, Bus_packet_s* packet);
static Cache_mesi_e mesi_snooping_shared_state(CacheData_s* data, Bus_packet_s* packet);
static Cache_mesi_e mesi_snooping_exlusive_state(CacheData_s* data, Bus_packet_s* packet);
static Cache_mesi_e mesi_snooping_modified_state(CacheData_s* data, Bus_packet_s* packet);

/************************************
*      variables                    *
************************************/
static snooping_state gSnoopingSM[4] = {
	mesi_snooping_invalid_state,
	mesi_snooping_shared_state,
	mesi_snooping_exlusive_state,
	mesi_snooping_modified_state
};

/************************************
*       API implementation          *
************************************/
void Cache_Init(CacheData_s* data, Cache_Id_e id)
{
	// set all cache memory to 0;
	memset((uint8_t*)data, 0, sizeof(data));
	data->id = id;

	// Register callback
	Bus_cache_interface_s cache_interface = { .core_id = id, .cache_data = data };
	cache_interface.cache_data = data;
	cache_interface.core_id = id;
	Bus_RegisterCache(cache_interface);
}

void Cache_RegisterBusHandles(void)
{
	Bus_RegisterCacheCallbacks(shared_signal_handle,
		cache_snooping_handle,
		cache_response_handle);
}

bool Cache_ReadData(CacheData_s* cache_data, uint32_t address, uint32_t* data)
{
	static bool miss_occurred = false;

	if (Bus_InTransaction(cache_data->id) || Bus_WaitForTransaction(cache_data->id))
		return false;

	cache_addess_s addr;
	addr.address = address;
	Tsram_s* tsram = &(cache_data->tsram[addr.as_bits.index]);

	// check if addresss tag is locate on block_number
	if (tsram->fields.tag == addr.as_bits.tag && tsram->fields.mesi != cache_mesi_invalid)
	{
		// hit on cache, getting the value 
		uint16_t index = addr.as_bits.index * BLOCK_SIZE + addr.as_bits.offset;
		*data = cache_data->dsram[index];

		if (!miss_occurred)
			cache_data->statistics.read_hits++;
		else
			miss_occurred = false;

		return true;
	}

	// we had a miss.
	miss_occurred = true;
	cache_data->statistics.read_misses++;

	// first, check if the required block is dirty
	// if so, we need first to send the block into the memory
	dirty_block_handling(cache_data, addr);

	// now, we need to take the new block from the main memory.
	Bus_packet_s packet = {
		.bus_origid = cache_data->id, .bus_cmd = bus_busRd, .bus_addr = addr.address, .bus_data = 0, .bus_shared = 0 };

	// add the read transaction into the bus queue
	Bus_AddTransaction(packet);

	return false;
}

bool Cache_WriteData(CacheData_s* cache_data, uint32_t address, uint32_t data)
{
	static bool miss_occurred = false;

	if (Bus_InTransaction(cache_data->id) || Bus_WaitForTransaction(cache_data->id))
		return false;

	cache_addess_s addr;
	addr.address = address;
	Tsram_s* tsram = &(cache_data->tsram[addr.as_bits.index]);

	// check if addresss tag is locate on block_number
	if (tsram->fields.tag == addr.as_bits.tag && tsram->fields.mesi != cache_mesi_invalid)
	{
		// if the block is shared, we need to go with RdX transaction and we have a miss.
		;		if (tsram->fields.mesi == cache_mesi_shared)
		{
			miss_occurred = true;
			cache_data->statistics.write_misses++;
			// Send Bus_Rdx to make sure this block is exclusive ours
			Bus_packet_s packet = {
				.bus_origid = cache_data->id, .bus_cmd = bus_busRdX, .bus_addr = addr.address, .bus_data = 0, .bus_shared = 0 };

			Bus_AddTransaction(packet);

			// add invalid packet for delay the in one cycle the next request
			Bus_packet_s invalid_packet = { .bus_origid = bus_invalid_originator };
			Bus_AddTransaction(invalid_packet);
			return false;
		}

		// hit on cache, write data
		// if the block is exclusive ours, we just change it locally
		if (!miss_occurred)
			cache_data->statistics.write_hits++;
		else
			miss_occurred = false;

		uint16_t index = addr.as_bits.index * BLOCK_SIZE + addr.as_bits.offset;
		cache_data->dsram[index] = data;
		// update modified flag.
		cache_data->tsram[addr.as_bits.index].fields.mesi = cache_mesi_modified;
		return true;
	}
	// we had a miss.
	miss_occurred = true;
	cache_data->statistics.write_misses++;

	// first, check if the required block is dirty
	// if so, we need first to send the block into the memory
	dirty_block_handling(cache_data, addr);

	// we need to take the data from the main memory.
	Bus_packet_s packet = {
		.bus_origid = cache_data->id, .bus_cmd = bus_busRdX, .bus_addr = addr.address, .bus_data = 0, .bus_shared = 0 };

	// add the read transaction into the bus queue
	Bus_AddTransaction(packet);

	return false;
}

void Cache_PrintData(CacheData_s* cache_data, FILE* dsram_file, FILE* tsram_file)
{
	for (uint32_t i = 0; i < CACHE_SIZE; i++)
		fprintf(dsram_file, "%08X\n", cache_data->dsram[i]);

	for (uint32_t i = 0; i < FRAME_SIZE; i++)
		fprintf(tsram_file, "%08X\n", cache_data->tsram[i].data);
}

/************************************
* static implementation             *
************************************/
static void dirty_block_handling(CacheData_s* data, cache_addess_s addr)
{
	if (data->tsram[addr.as_bits.index].fields.mesi == cache_mesi_modified)
	{
		// get stored block address
		cache_addess_s block_addr = {
			.as_bits.index = addr.as_bits.index,
			.as_bits.tag = data->tsram[addr.as_bits.index].fields.tag,
			.as_bits.offset = 0
		};

		// send bus flush
		Bus_packet_s packet = {
			.bus_origid = data->id, .bus_cmd = bus_flush, .bus_addr = block_addr.address, .bus_shared = 0 };

		uint16_t index = addr.as_bits.index * BLOCK_SIZE + addr.as_bits.offset;
		packet.bus_data = data->dsram[index];

		// add the flush transaction into the bus queue
		Bus_AddTransaction(packet);
	}
}


static bool shared_signal_handle(CacheData_s* data, Bus_packet_s* packet, bool* is_modified)
{
	// check if this is my packet
	if (data->id == packet->bus_origid)
		return false;

	cache_addess_s address = { .address = packet->bus_addr };
	Tsram_s* tsram = &(data->tsram[address.as_bits.index]);
	//
	*is_modified |= tsram->fields.mesi == cache_mesi_modified;
	return tsram->fields.tag == address.as_bits.tag && tsram->fields.mesi != cache_mesi_invalid;
}

static bool cache_snooping_handle(CacheData_s* data, Bus_packet_s* packet, uint8_t address_offset)
{
	// check if this is my packet
	if (data->id == packet->bus_original_sender && packet->bus_cmd != bus_flush)
		return false;

	cache_addess_s address = { .address = packet->bus_addr };
	Tsram_s* tsram = &(data->tsram[address.as_bits.index]);

	// check if the block is in the cache, if not, do nothing.
	if (tsram->fields.tag != address.as_bits.tag || tsram->fields.mesi == cache_mesi_invalid)
		return false;

	// execute block state machine
	Cache_mesi_e next_state = gSnoopingSM[tsram->fields.mesi](data, packet);

	if (address_offset == (BLOCK_SIZE - 1) || tsram->fields.mesi != cache_mesi_modified)
	{
		tsram->fields.mesi = next_state;
	}

	return true;
}

static bool cache_response_handle(CacheData_s* data, Bus_packet_s* packet, uint8_t* address_offset)
{
	// check if this is my packet
	if (data->id == packet->bus_origid && packet->bus_cmd != bus_flush)
		return false;
	else if (data->id == packet->bus_origid && packet->bus_cmd == bus_flush)
	{
		if (*address_offset == (BLOCK_SIZE - 1))
			return true;

		*address_offset += 1;
		return false;
	}

	cache_addess_s address = { .address = packet->bus_addr };
	Tsram_s* tsram = &(data->tsram[address.as_bits.index]);

	// update the new tag
	tsram->fields.tag = address.as_bits.tag;

	// execute block state machine
	if (packet->bus_cmd == bus_flush)
	{
		uint16_t index = address.as_bits.index * BLOCK_SIZE + address.as_bits.offset;
		data->dsram[index] = packet->bus_data;
	}

	if (*address_offset == (BLOCK_SIZE - 1))
	{
		tsram->fields.mesi = packet->bus_shared ? cache_mesi_shared : cache_mesi_exclusive;
		return true;
	}

	*address_offset += 1;
	return false;
}

// state machine for snooping
static Cache_mesi_e mesi_snooping_invalid_state(CacheData_s* data, Bus_packet_s* packet)
{
	return cache_mesi_invalid;
}

static Cache_mesi_e mesi_snooping_shared_state(CacheData_s* data, Bus_packet_s* packet)
{
	if (packet->bus_cmd == bus_busRdX)
		return cache_mesi_invalid;

	return cache_mesi_shared;
}

static Cache_mesi_e mesi_snooping_exlusive_state(CacheData_s* data, Bus_packet_s* packet)
{
	if (packet->bus_cmd == bus_busRd)
		return cache_mesi_shared;

	if (packet->bus_cmd == bus_busRdX)
		return cache_mesi_invalid;

	return cache_mesi_exclusive;
}

static Cache_mesi_e mesi_snooping_modified_state(CacheData_s* data, Bus_packet_s* packet)
{
	cache_addess_s address = { .address = packet->bus_addr };
	uint16_t index = address.as_bits.index * BLOCK_SIZE + address.as_bits.offset;

	if (packet->bus_cmd == bus_busRd)
	{
		// send back the modified data
		packet->bus_cmd = bus_flush;
		packet->bus_data = data->dsram[index];
		packet->bus_origid = data->id;

		return cache_mesi_shared;
	}

	else if (packet->bus_cmd == bus_busRdX)
	{
		// send back the modified data
		packet->bus_cmd = bus_flush;
		packet->bus_data = data->dsram[index];
		packet->bus_origid = data->id;

		return cache_mesi_invalid;
	}

	else if (packet->bus_cmd == bus_flush)
	{
		// send back the modified data
		packet->bus_cmd = bus_flush;
		packet->bus_data = data->dsram[index];
		packet->bus_origid = data->id;

		return cache_mesi_modified;
	}

	return cache_mesi_modified;
}