#ifndef __BUS_H__
#define __BUS_H__

/************************************
*      include                      *
************************************/
#include <stdint.h>
#include <stdbool.h>

/************************************
*      definitions                 *
************************************/

/************************************
*       types                       *
************************************/
typedef enum
{
	bus_core0,
	bus_core1,
	bus_core2,
	bus_core3,
	bus_main_memory,
	bus_invalid_originator = 0xFFFF
} Bus_originator_e;

typedef enum
{
	bus_no_command,
	bus_busRd,
	bus_busRdX,
	bus_flush
} Bus_command_s;

typedef struct
{
	Bus_originator_e bus_original_sender;
	Bus_originator_e bus_origid;
	Bus_command_s bus_cmd;
	uint32_t bus_addr;
	uint32_t bus_data;
	bool bus_shared;
} Bus_packet_s;

typedef struct
{
	int core_id;
	void* cache_data;
} Bus_cache_interface_s;

typedef bool (*shared_signal_callback)(void* cache_data, Bus_packet_s* packet, bool* is_modified);
typedef bool (*cache_snooping_callback)(void* cache_data, Bus_packet_s* packet, uint8_t address_offset);
typedef bool (*cache_response_callback)(void* cache_data, Bus_packet_s* packet, uint8_t* address_offset);
typedef bool (*memory_callback_t)(Bus_packet_s* packet, bool direct_transaction);

/************************************
*       API                         *
************************************/
void Bus_RegisterCache(Bus_cache_interface_s cache_interface);
void Bus_RegisterCacheCallbacks(shared_signal_callback signal_callback,
	cache_snooping_callback snooping_callback,
	cache_response_callback response_callback);
void Bus_RegisterMemoryCallback(memory_callback_t callback);

/*!
******************************************************************************
\brief
 Create a new transaction on the bus.
\details
 this function add the transaction into the round-robin queue.
\param
 [in] counter_val - reset counter value
 [out] out_val    -
\return none
*****************************************************************************/
void Bus_AddTransaction(Bus_packet_s packet);

/*!
******************************************************************************
\brief
 Check if the bus is in transaction
\param
 [in] originator - the id of the core.
\return true if the bus is in transaction
*****************************************************************************/
bool Bus_InTransaction(Bus_originator_e originator);

/*!
******************************************************************************
\brief
 Check if the bus is waiting for transaction
\param
 [in] originator - the id of the core.
\return true if the bus is waiting
*****************************************************************************/
bool Bus_WaitForTransaction(Bus_originator_e originator);

/*!
******************************************************************************
\brief
 Bus iteration
\return none
*****************************************************************************/
void Bus_Iter(void);

#endif //__BUS_H__
