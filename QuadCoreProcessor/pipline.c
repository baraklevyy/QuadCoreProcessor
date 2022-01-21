
#include "pipline.h"
#include <string.h>
#include "cache.h"

static void fetch(Pipeline_s* pip);
static void decode(Pipeline_s* pip);
static void execute(Pipeline_s* pip);
static void mem(Pipeline_s* pip);
static void writeback(Pipeline_s* pip);
static void start_pip_phases(Pipeline_s* pip);
static void set_arguments_to_regs(Pipeline_s* pip, state_of_pipeline stage);
static bool pipeline_needs_data_hazard_stall(Pipeline_s* pip);
static bool compare_register(Pipeline_s* pip, uint16_t reg);
static bool check_registers_hazrads(Pipeline_s* pip, state_of_pipeline stage);
static void (*pipeline_functions_pointer[PIPELINE_SIZE])(Pipeline_s* pip) ={fetch, decode, execute, mem, writeback};
static void update_statistics(Pipeline_s* pip);

void initialize_pip(Pipeline_s* pip)
{
	pip->is_pip_halt = false;
	pip->is_data_stall = false;
	pip->is_mem_stall = false;
	pip->opcode_params.is_command_in_halt = &pip->is_pip_halt;

	memset((uint8_t*)pip->pipe_stages, 0, sizeof(pip->pipe_stages));
	for (int phase = FETCH; phase < PIPELINE_SIZE; phase++)
	{
		pip->pipe_stages[phase].state = phase;
		pip->pipe_stages[phase].pc = MAX_INTEGER;
	}

	pip->pipe_stages[FETCH].pc = 0;
}


void Pipeline_Execute(Pipeline_s* pip)
{
	pip->is_data_stall = pipeline_needs_data_hazard_stall(pip);
	start_pip_phases(pip);
	update_statistics(pip);
}

bool is_flush_require(Pipeline_s* pip, int phase) {
	return (pip->pipe_stages[phase].pc == MAX_INTEGER);
}
bool flush_the_pipe(Pipeline_s* pip)
{
	bool res;
	res = pip->is_pip_halt;
	for (int phase = FETCH; phase < 5; phase++)
	{
		res = res & (is_flush_require(pip, phase));
	}
	return res;
}


void tracing_pip(Pipeline_s* pip, FILE* trace_file)
{
	for (int phase = FETCH; phase < 5; phase++)
	{
		if (MAX_INTEGER == pip->pipe_stages[phase].pc)
			fprintf(trace_file, "--- "); //this is the signs we asked to add
		else
			fprintf(trace_file, "%03X ", pip->pipe_stages[phase].pc);
	}
}


void add_idle_slot(Pipeline_s* ppl)
{
	
	int current_stage = 4; // pipleline length - 1 
	while (current_stage > 0) {
		if (EXECUTE == current_stage && ppl->is_data_stall)
		{
			ppl->pipe_stages[EXECUTE].pc = MAX_INTEGER;
			break;
		}
		else if (ppl->is_mem_stall)
		{
			ppl->pipe_stages[WRITE_BACK].pc = MAX_INTEGER;
			break;
		}
		else if (ppl->pipe_stages[current_stage - 1].pc == MAX_INTEGER)
		{
			ppl->pipe_stages[current_stage].pc = MAX_INTEGER;
			current_stage -= 1;
		}
		else
		{
			ppl->pipe_stages[current_stage].pc = ppl->pipe_stages[current_stage - 1].pc;
			ppl->pipe_stages[current_stage].instruction = ppl->pipe_stages[current_stage - 1].instruction;
			ppl->pipe_stages[current_stage].operation = *ppl->pipe_stages[current_stage - 1].operation;
			ppl->pipe_stages[current_stage].execution_output = ppl->pipe_stages[current_stage - 1].execution_output;
			current_stage -= 1;
		}
	}
	if (true == ppl->is_pip_halt)
	{
		ppl->pipe_stages[FETCH].pc = MAX_INTEGER;
		ppl->pipe_stages[DECODE].pc = MAX_INTEGER;
	}
}




static void fetch(Pipeline_s* ppl)
{

	if (false == ppl->is_mem_stall) {
		ppl->pipe_stages[FETCH].pc = *(ppl->opcode_params.pc);
		ppl->pipe_stages[FETCH].instruction = ppl->insturcionts_p[*(ppl->opcode_params.pc)];
		if (!ppl->is_data_stall) {
			uint16_t pc_tmp = *(ppl->opcode_params.pc) + 1;
			*(ppl->opcode_params.pc) = pc_tmp;
		}
	}
}

static void decode(Pipeline_s* ppl)
{
	uint16_t code = get_command_opcode(ppl->pipe_stages[DECODE].instruction);
	if (HALT == code)
	{
		ppl->is_pip_halt = true;
		return;
	}
	ppl->pipe_stages[DECODE].operation = *(opcode_command_function_pointer + code);
	bool is_branch_taken; 
	is_branch_taken = (BEQ <= code && LW > code);
	if (is_branch_taken)
	{
		set_arguments_to_regs(ppl, DECODE);
		ppl->pipe_stages[DECODE].operation(&ppl->opcode_params);
	}
}

static void execute(Pipeline_s* ppl)
{
	uint16_t code = get_command_opcode(ppl->pipe_stages[EXECUTE].instruction);
	if (!(BEQ <= code && LW > code) && !(SW == code || LW == code) && HALT != code) 
	{
		set_arguments_to_regs(ppl, EXECUTE);
		ppl->pipe_stages[EXECUTE].operation(&ppl->opcode_params);
	}
}

static void mem(Pipeline_s* ppl)
{
	uint16_t code;
	code = get_command_opcode(ppl->pipe_stages[MEM].instruction);
	if (code == LW || code == SW) //if this is a memory phase
	{
		bool is_data_in_cache;
		set_arguments_to_regs(ppl, MEM);
		uint32_t registers_address;
		registers_address = ppl->opcode_params.rs + ppl->opcode_params.rt;
		uint32_t* data_for_mem;
		data_for_mem  = ppl->opcode_params.rd;
		if (SW == code) is_data_in_cache = Cache_WriteData(&ppl->current_data_from_cache, registers_address, *data_for_mem);
		else is_data_in_cache = Cache_ReadData(&ppl->current_data_from_cache, registers_address, data_for_mem);
		bool add_stall = !is_data_in_cache;
		ppl->is_mem_stall = add_stall;
	}
}

static void writeback(Pipeline_s* ppl)
{
	uint32_t instruction;
	instruction = ppl->pipe_stages[WRITE_BACK].instruction;
	int index_for_jump;
	if (JAL == get_command_opcode(instruction)) index_for_jump = NEXT_INSTRUCTION_ADDRESS_REGISTER;
	else index_for_jump = get_command_rd(instruction);
	*(ppl->current_core_regs + index_for_jump) = ppl->pipe_stages[WRITE_BACK].execution_output;
}


static void set_arguments_to_regs(Pipeline_s* ppl, state_of_pipeline ppl_phase)
{
	uint32_t instruction;
	instruction = ppl->pipe_stages[ppl_phase].instruction;
	*(ppl->current_core_regs +REG_IMM) = get_command_immediate(instruction);
	current_pipeline_information* current_info;
	current_info = ppl->pipe_stages;
	(current_info + ppl_phase)->execution_output = ppl->current_core_regs[get_command_rd(instruction)];
	ppl->opcode_params.rs = ppl->current_core_regs[get_command_rs(instruction)];
	ppl->opcode_params.rt = ppl->current_core_regs[get_command_rt(instruction)];
	ppl->opcode_params.rd = &ppl->pipe_stages[ppl_phase].execution_output;
}


static void start_pip_phases(Pipeline_s* pip)
{
	uint16_t phase;
	if (pip->is_mem_stall) phase = MEM;
	else if (pip->is_data_stall) phase = EXECUTE;
	else phase = DECODE;
	if (!pip->is_pip_halt)
		pipeline_functions_pointer[FETCH](pip);

	for (phase; phase < PIPELINE_SIZE; phase++)
	{
		if (!(is_flush_require(pip, phase))) pipeline_functions_pointer[phase](pip);
	}
}

/*!
******************************************************************************
\brief
Comparing between the registers of certain stage and the rd register.
\param
 [in]  Pipeline_s *pipeline - Pointer to the relevant pipeline.
 [in]  uint16_t reg			- The rd register we are comparing to.
 [in]  uint16_t stage		- The pipeline stage we are comparing to.
 [out] bool
\return true if found two identical registed index, false otherwise.
*****************************************************************************/
//static bool compare_register(Pipeline_s* pipeline, uint16_t reg, uint16_t stage)
static bool compare_register(Pipeline_s* pip, uint16_t reg)
{
	bool ret = false;
	uint32_t decode_ins = pip->pipe_stages[DECODE].instruction;
	uint16_t op_write_back = get_command_opcode(pip->pipe_stages[WRITE_BACK].instruction);
	uint16_t rs, rd, rt, opcode;
	rs = get_command_rs(decode_ins);
	rt = get_command_rt(decode_ins);
	rd = get_command_rd(decode_ins);
	opcode = get_command_opcode(decode_ins);
	return(reg_compare_logic(decode_ins, reg, opcode, op_write_back));
}

bool reg_compare_logic(uint32_t instruction, uint16_t reg, uint16_t op_code, uint16_t op_write_back) {
	if (reg == REG_IMM || reg == REG_ZERO) return false;
	if (get_command_opcode(instruction) == LW || get_command_opcode(instruction) <= SRL || ((get_command_opcode(instruction) == SW && SW == op_write_back)))
		return (reg_compare_helper(reg, get_command_rs(instruction)) || reg_compare_helper(reg, get_command_rt(instruction)));

	
	//otherwise we have to return
	return (reg == get_command_rd(instruction) || reg == get_command_rs(instruction) || reg == get_command_rt(instruction));
}

bool reg_compare_helper(uint16_t reg1, uint16_t reg2) {
	return reg1 == reg2;
}


/*!
******************************************************************************
\brief
Checking registers data hazards.
\param
 [in]  Pipeline_s *pipeline - Pointer to the relevant pipeline.
 [in]  uint16_t stage		- The pipeline stage we are comparing to.
 [out] bool
\return true if found data hazard, false otherwise.
*****************************************************************************/
static bool check_registers_hazrads(Pipeline_s* pip, state_of_pipeline stage)
{
	if (pip->pipe_stages[stage].pc == MAX_INTEGER)
	{
		return false;
	}
	//return compare_register(pipeline, pipeline->pipe_stages[stage].instruction.bits.rd, stage);
	//return compare_register(pipeline, pipeline->pipe_stages[stage].instruction.bits.rd);
	return compare_register(pip, get_command_rd(pip->pipe_stages[stage].instruction));
}

/*!
******************************************************************************
\brief
Checking if pipeline needs data hazard.
\param
 [in]  Pipeline_s *pipeline - Pointer to the relevant pipeline.
 [out] bool
\return true pipeline in data hazard, false otherwise.
*****************************************************************************/
static bool pipeline_needs_data_hazard_stall(Pipeline_s* pip)
{
	return check_registers_hazrads(pip, EXECUTE) || check_registers_hazrads(pip, WRITE_BACK) ||
		check_registers_hazrads(pip, MEM);
}

/*!
******************************************************************************
\brief
Updating staistics struct.
\param
 [in] core - the operating core.
\return none
*****************************************************************************/
static void update_statistics(Pipeline_s* pip)
{
	if (pip->is_data_stall && !pip->is_mem_stall)
		pip->current_pip_decode_stalls++;

	if (pip->is_mem_stall)
		pip->current_pip_memory_stalls++;
}