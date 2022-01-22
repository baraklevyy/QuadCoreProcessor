

#ifndef __HELPERS_H__
#define __HELPERS_H__
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#define MAX_INTEGER 0xffffui16
#define MAIN_MEMORY_SIZE 1048576 
#define BAD_EXIT_CODE 1
#define NUMBER_OF_REGISTERS 16
#define REG_ZERO 0
#define REG_IMM 1
#define NEXT_INSTRUCTION_ADDRESS_REGISTER 15
#define CORES_NUMBER 4
#define SIZE_OF_INST 1024
#define CACHE_SIZE	256
#define BLOCK_SIZE	4
#define TSRAM_NUMBER_OF_LINES 64
#define PIPELINE_SIZE 5

#define _CSECURE_NO_WARNINGS


typedef struct
{
	uint32_t address;
	uint16_t offset;
	uint16_t block;
} mem_address;



typedef struct
	{
		FILE* immediate_memory_file;
		FILE* register_out_files;
		FILE* core_trace_files;
		FILE* dsram_files;
		FILE* tsram_files;
		FILE* statistics_files;
	} current_core_data_files;



typedef struct
{
	bool* is_command_in_halt;
	uint32_t rs;
	uint16_t* pc;
	uint32_t* rd;
	uint32_t rt;
}parameters_to_command;









extern current_core_data_files files_of_cores[CORES_NUMBER];
FILE* MeminFile;
FILE* MemoutFile;
FILE* BusTraceFile;

enum core_E{core0=0, core1, core2, core3};
enum file_names_E{ imem0 = 1, imem1 , imem2, imem3,
	memin, memout,
	regout0, regout1, regout2, regout3,
	core0trace, core1trace, core2trace, core3trace, bustrace,
	dsram0, dsram1, dsram2, dsram3,
	tsram0, tsram1, tsram2, tsram3,
	stats0, stats1, stats2, stats3};

typedef enum {invalid=0, shared, exclusive, modified} mesi_state;
typedef enum {CORE0=0, CORE1, CORE2, CORE3,} core_identifier;
typedef enum {ADD = 0, SUB, AND, OR, XOR, MUL, SLL, SRA, SRL, BEQ, BNE, BLT, BGT, BLE, BGE, JAL, LW, SW, HALT = 20 }OpcodeOperations;
typedef enum {FETCH = 0, DECODE, EXECUTE, MEM, WRITE_BACK} phase_of_pipeline;
typedef enum {core0_on_bus = 0, core1_on_bus, core2_on_bus, core3_on_bus, main_memory_on_bus, err_originator_on_bus = 0xFFFF} orig_id_on_bus;
typedef enum {idle_cmd_on_bus, bus_read_cmd_on_bus, bus_read_exclusive_cmd_on_bus, bus_flush_cmd_on_bus} cmd_exec_on_bus;
typedef enum {success_op, failed_op}exit_func_code;



typedef struct
{
	phase_of_pipeline state;
	uint32_t instruction;
	uint32_t execution_output;
	void (*operation)(parameters_to_command* arguments_to_cmd);
	uint16_t pc;
} current_pipeline_information;
typedef struct
{
	orig_id_on_bus first_send_on_bus;
	orig_id_on_bus origid_on_bus;
	cmd_exec_on_bus command_on_bus;
	uint32_t address_on_bus;
	uint32_t data_on_bus;
	bool is_bus_shared;
} data_on_bus;
typedef struct
{
	int core_number;
	void* data_from_cache;
} communication_of_bus_cache_info;

/// <summary>
/// Queue for Round-Robin arbitration
/// </summary>
typedef struct _linked_list
{
	data_on_bus data;
	struct _linked_list* previous;
	struct _linked_list* next;
} arbitration_linked_list;



typedef struct {
	core_identifier id;
	bool memory_stall;
	uint32_t dsram[CACHE_SIZE];
	uint32_t number_of_read_hit;
	uint32_t number_of_write_hit;
	uint32_t number_of_read_miss;
	uint32_t number_of_write_miss;
	uint32_t tsram[TSRAM_NUMBER_OF_LINES];
} cache_information;




typedef bool (*shared_function_pointer)(void* data_from_cache, data_on_bus* packet, bool* changed);
typedef bool (*snoop_function_pointer)(void* data_from_cache, data_on_bus* packet, uint8_t address_offset);
typedef bool (*cache_answer_function_pointer)(void* data_from_cache, data_on_bus* packet, uint8_t* address_offset);
typedef bool (*send_to_memory_function_pointer)(data_on_bus* packet, bool direct_transaction);
typedef enum {idle_status, ready_state, operate_state, final_state} operation_status;




uint16_t get_address_offset(uint32_t address);
uint16_t get_address_block(uint32_t address);
void set_offset_to_address(uint32_t* address, uint8_t offset);
uint16_t get_tsram_tag(uint32_t tsram);
uint16_t get_tsram_mesi_state(uint32_t tsram);
void set_tag_to_tsram(uint32_t* tsram, uint16_t tag);
void set_mesi_state_to_tsram(uint32_t* tsram, uint16_t mesi_state);
uint32_t get_cache_address_offset(uint32_t cache);
uint32_t get_cache_address_index(uint32_t cache);
uint32_t get_cache_address_tag(uint32_t cache);
void set_offset_to_cache_address(uint32_t* cache, uint32_t offset);
void set_index_to_cache_address(uint32_t* cache, uint32_t index);
void set_tag_to_cache_address(uint32_t* cache, uint32_t tag);
uint16_t get_command_immediate(uint32_t cmd);
uint16_t get_command_rt(uint32_t cmd);
uint16_t get_command_rs(uint32_t cmd);
uint16_t get_command_rd(uint32_t cmd);
uint16_t get_command_opcode(uint32_t cmd);
void add(parameters_to_command* arguments_to_cmd);
void sub(parameters_to_command* arguments_to_cmd);
void and (parameters_to_command* arguments_to_cmd);
void or (parameters_to_command * arguments_to_cmd);
void xor (parameters_to_command* arguments_to_cmd);
void mul(parameters_to_command* arguments_to_cmd);
void sll(parameters_to_command* arguments_to_cmd);
void sra(parameters_to_command* arguments_to_cmd);
void srl(parameters_to_command* arguments_to_cmd);
void beq(parameters_to_command* arguments_to_cmd);
void bne(parameters_to_command* arguments_to_cmd);
void blt(parameters_to_command* arguments_to_cmd);
void bgt(parameters_to_command* arguments_to_cmd);
void ble(parameters_to_command* arguments_to_cmd);
void bge(parameters_to_command* arguments_to_cmd);
void jal(parameters_to_command* arguments_to_cmd);

static void (*opcode_command_function_pointer[16])(parameters_to_command* arguments_to_cmd) = {
	 add, sub,and, or ,xor, mul, sll, sra,
	 srl, beq, bne, blt, bgt,
	 ble, bge, jal
};

void set_cache_bus_commu_func(communication_of_bus_cache_info cache_communication_bus);
void set_cache_shared_function(shared_function_pointer shared_func);
void set_bus_snoop_function(snoop_function_pointer snoop_func);
void set_cache_answer_function(cache_answer_function_pointer cache_ans_func);
void set_bus_memory_func(send_to_memory_function_pointer memory_operation);


/// <summary>
/// this func is pushing a new transaction to bus. Arbitration is Round-Robin
/// </summary>
/// <param name="bus_data"></param>
void push_new_bus_operation(data_on_bus packet);

/// <summary>
/// check if the bus is currently on transaction
/// </summary>
/// <param name="core_id"></param>
/// <returns>if the bus is on trnsation right now</returns>
bool is_bus_busy(orig_id_on_bus core_id);

/// <summary>
/// check if the bus is in idle mode and he can operate
/// </summary>
/// <param name="core_id"></param>
/// <returns>true if is idle</returns>
bool is_bus_waiting_for_operate(orig_id_on_bus core_id);

/// <summary>
/// operation the bus
/// </summary>
/// <param name=""></param>
void operate_bus(void);



void set_cache_shared_func(void);
void initialize_the_cache(cache_information* data, core_identifier id);
void set_cache_answer(void);
void set_cache_snoop_function(void);
exit_func_code read_from_cache(cache_information* cache_data, uint32_t address, uint32_t* data);
exit_func_code write_to_cache(cache_information* cache_data, uint32_t address, uint32_t data);
void cache_print_to_file(cache_information* cache_data, FILE* dsram_f, FILE* tsram_f);




#endif 