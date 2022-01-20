/*!
******************************************************************************
\file Pipeline.c
\date 17 October 2021
\author Rony Kositsky & Ofir Guthman & Yonatan Gartenberg
\brief
\details
\par Copyright
(c) Copyright 2021 Ofir & Rony & Yonatan
\par
ALL RIGHTS RESERVED
*****************************************************************************/

/************************************
*      include                      *
************************************/
#include "pipline.h"
#include <string.h>
#include "cache.h"

/************************************
*      static functions             *
************************************/
static void fetch(Pipeline_s* pipeline);
static void decode(Pipeline_s* pipeline);
static void execute(Pipeline_s* pipeline);
static void mem(Pipeline_s* pipeline);
static void writeback(Pipeline_s* pipeline);
static void execute_stages(Pipeline_s* pipeline);
static void prepare_registers_params(Pipeline_s* pipeline, PipelineSM_e stage);
static bool pipeline_needs_data_hazard_stall(Pipeline_s* pipeline);
//static bool compare_register(Pipeline_s* pipeline, uint16_t reg, uint16_t stage);
static bool compare_register(Pipeline_s* pipeline, uint16_t reg);
static bool check_registers_hazrads(Pipeline_s* pipeline, PipelineSM_e stage);
static void (*pipe_functions[PIPELINE_SIZE])(Pipeline_s* pipeline) =
{
	fetch, decode, execute, mem, writeback
};
static void update_statistics(Pipeline_s* pipeline);

/************************************
*       API implementation          *
************************************/

/*!
******************************************************************************
\brief
Init the pipeline.
\param
 [in]  none
 [out] none
\return none
*****************************************************************************/
void Pipeline_Init(Pipeline_s* pipeline)
{
	pipeline->halted = false;
	pipeline->data_hazard_stall = false;
	pipeline->memory_stall = false;

	memset((uint8_t*)&pipeline->statistics, 0, sizeof(pipeline->statistics));
	memset((uint8_t*)&pipeline->opcode_params, 0, sizeof(pipeline->opcode_params));
	pipeline->opcode_params.halt = &pipeline->halted;

	memset((uint8_t*)pipeline->pipe_stages, 0, sizeof(pipeline->pipe_stages));
	for (int stage = FETCH; stage < PIPELINE_SIZE; stage++)
	{
		pipeline->pipe_stages[stage].state = stage;
		pipeline->pipe_stages[stage].pc = MAX_INTEGER;
	}

	pipeline->pipe_stages[FETCH].pc = 0;
}

/*!
******************************************************************************
\brief
One iteration of the pipeline. We will bubble the values inside the pipeline
based on it's condition.
\param
 [in]  none
 [out] none
\return none
*****************************************************************************/
void Pipeline_Execute(Pipeline_s* pipeline)
{
	pipeline->data_hazard_stall = pipeline_needs_data_hazard_stall(pipeline);
	execute_stages(pipeline);
	update_statistics(pipeline);
}

/*!
******************************************************************************
\brief
The pipeline flushed all the stages.
\param
 [in]  Pipeline_s *pipeline - Pointer to the relevant pipeline.
 [out] bool
\return true if flushed, false otherwise.
*****************************************************************************/
bool Pipeline_PipeFlushed(Pipeline_s* pipeline)
{
	bool flushed = pipeline->halted;
	for (int stage = FETCH; stage < PIPELINE_SIZE; stage++)
	{
		flushed &= (pipeline->pipe_stages[stage].pc == MAX_INTEGER);
	}

	return flushed;
}

/*!
******************************************************************************
\brief
Writing the pipeline to the trace file.
\param
 [in]  Pipeline_s *pipeline - Pointer to the relevant pipeline.
 [in]  FILE *trace_file		- Pointer to trace file.
 [out] none
\return none
*****************************************************************************/
void Pipeline_WriteToTrace(Pipeline_s* pipeline, FILE* trace_file)
{
	for (int stage = FETCH; stage < PIPELINE_SIZE; stage++)
	{
		if (pipeline->pipe_stages[stage].pc == MAX_INTEGER)
			fprintf(trace_file, "--- ");
		else
			fprintf(trace_file, "%03X ", pipeline->pipe_stages[stage].pc);
	}
}

/*!
******************************************************************************
\brief
Bubble the commands through the pipeline. Entering bubble where such thing
is necessary.
\param
 [in]  Pipeline_s *pipeline - Pointer to the relevant pipeline.
 [out] none
\return none
*****************************************************************************/
void Pipeline_BubbleCommands(Pipeline_s* ppl)
{
	/*
	for (int stage = PIPELINE_SIZE - 1; stage > FETCH; stage--)
	{
		if (ppl->memory_stall)
		{
			pipeline->pipe_stages[WRITE_BACK].pc = UINT16_MAX;
			break;
		}
		else if (pipeline->data_hazard_stall && stage == EXECUTE)
		{
			pipeline->pipe_stages[EXECUTE].pc = UINT16_MAX;
			break;
		}
		else if (pipeline->pipe_stages[stage - 1].pc == UINT16_MAX)
		{
			pipeline->pipe_stages[stage].pc = UINT16_MAX;
		}
		else
		{
			pipeline->pipe_stages[stage].pc = pipeline->pipe_stages[stage - 1].pc;
			pipeline->pipe_stages[stage].instruction.cmd =
				pipeline->pipe_stages[stage - 1].instruction.cmd;
			pipeline->pipe_stages[stage].operation = *pipeline->pipe_stages[stage - 1].operation;
			pipeline->pipe_stages[stage].execute_result = pipeline->pipe_stages[stage - 1].execute_result;
		}
	}

	if (pipeline->halted)
	{
		pipeline->pipe_stages[FETCH].pc = UINT16_MAX;
		pipeline->pipe_stages[DECODE].pc = UINT16_MAX;
	}
	*/


	int current_stage = 4; // pipleline length - 1 
	while (current_stage > 0) {
		if (EXECUTE == current_stage && ppl->data_hazard_stall)
		{
			ppl->pipe_stages[EXECUTE].pc = MAX_INTEGER;
			break;
		}
		else if (ppl->memory_stall)
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
			ppl->pipe_stages[current_stage].execute_result = ppl->pipe_stages[current_stage - 1].execute_result;
			current_stage -= 1;
		}




	}

	/*
	for (int current_stage = 4; current_stage > 0; current_stage--)
	{
		if (EXECUTE == current_stage && ppl->data_hazard_stall)
		{
			ppl->pipe_stages[EXECUTE].pc = UINT16_MAX;
			break;
		}
		else if (ppl->memory_stall)
		{
			ppl->pipe_stages[WRITE_BACK].pc = UINT16_MAX;
			break;
		}
		else if (ppl->pipe_stages[current_stage - 1].pc == UINT16_MAX)
		{
			ppl->pipe_stages[current_stage].pc = UINT16_MAX;
		}
		else
		{
			ppl->pipe_stages[current_stage].pc = ppl->pipe_stages[current_stage - 1].pc;
			ppl->pipe_stages[current_stage].instruction = ppl->pipe_stages[current_stage - 1].instruction;
			ppl->pipe_stages[current_stage].operation = *ppl->pipe_stages[current_stage - 1].operation;
			ppl->pipe_stages[current_stage].execute_result = ppl->pipe_stages[current_stage - 1].execute_result;
		}
	}
	*/
	if (true == ppl->halted)
	{
		ppl->pipe_stages[FETCH].pc = MAX_INTEGER;
		ppl->pipe_stages[DECODE].pc = MAX_INTEGER;
	}
}



/************************************
* static implementation             *
************************************/

/*!
******************************************************************************
\brief
Fetch stage of the pipeline.
\param
 [in]  Pipeline_s *pipeline - Pointer to the relevant pipeline.
 [out] none
\return none
*****************************************************************************/
static void fetch(Pipeline_s* ppl)
{
	/*
	if (pipeline->memory_stall)
	{
		return;
	}
	*/
	if (false == ppl->memory_stall) {
		ppl->pipe_stages[FETCH].pc = *(ppl->opcode_params.pc);
		ppl->pipe_stages[FETCH].instruction = ppl->insturcionts_p[*(ppl->opcode_params.pc)];
		if (!ppl->data_hazard_stall) {
			uint16_t pc_tmp = *(ppl->opcode_params.pc) + 1;
			*(ppl->opcode_params.pc) = pc_tmp;
		}
	}
	/*
	pipeline->pipe_stages[FETCH].pc = *(pipeline->opcode_params.pc);
	pipeline->pipe_stages[FETCH].instruction = pipeline->insturcionts_p[*(pipeline->opcode_params.pc)];
	if (!pipeline->data_hazard_stall) // Not in stall
	{
		*(pipeline->opcode_params.pc) += 1;
	}
	*/
}

/*!
******************************************************************************
\brief
Decode stage of the pipeline.
\param
 [in]  Pipeline_s *pipeline - Pointer to the relevant pipeline.
 [out] none
\return none
*****************************************************************************/
static void decode(Pipeline_s* ppl)
{
	//uint16_t opcode = ppl->pipe_stages[DECODE].instruction.bits.opcode;
	uint16_t code = get_command_opcode(ppl->pipe_stages[DECODE].instruction);
	if (HALT == code)
	{
		ppl->halted = true;
		return;
	}

	ppl->pipe_stages[DECODE].operation = OpcodeMapping[code];

	//if (Opcode_IsBranchResulotion(ppl->pipe_stages[DECODE].instruction.bits.code))
	bool is_branch_taken; 
	is_branch_taken = (BEQ <= code && LW > code);
	if (is_branch_taken)
	{
		prepare_registers_params(ppl, DECODE);
		ppl->pipe_stages[DECODE].operation(&ppl->opcode_params);
	}
}

/*!
******************************************************************************
\brief
Execute stage of the pipeline.
\param
 [in]  Pipeline_s *pipeline - Pointer to the relevant pipeline.
 [out] none
\return none
*****************************************************************************/
static void execute(Pipeline_s* ppl)
{
	//uint16_t code = ppl->pipe_stages[EXECUTE].instruction.bits.code;
	uint16_t code = get_command_opcode(ppl->pipe_stages[EXECUTE].instruction);
	//if (!Opcode_IsBranchResulotion(code) && !Opcode_IsMemoryCommand(code) && code != HALT)
	if (!(BEQ <= code && LW > code) && !(SW == code || LW == code) && HALT != code) //not branch not memory and not halt command
	{
		prepare_registers_params(ppl, EXECUTE);
		ppl->pipe_stages[EXECUTE].operation(&ppl->opcode_params);
	}
}

/*!
******************************************************************************
\brief
Memory stage of the pipeline.
\param
 [in]  Pipeline_s *pipeline - Pointer to the relevant pipeline.
 [out] none
\return none
*****************************************************************************/
static void mem(Pipeline_s* ppl)
{
	//uint16_t opcode = ppl->pipe_stages[MEM].instruction.bits.opcode;
	uint16_t code;
	code = get_command_opcode(ppl->pipe_stages[MEM].instruction);
	//if (Opcode_IsMemoryCommand(code))
	if (code == LW || code == SW) //if this is a memory phase
	{
		bool is_data_in_cache;
		prepare_registers_params(ppl, MEM);
		uint32_t registers_address;
		registers_address = ppl->opcode_params.rs + ppl->opcode_params.rt;

		uint32_t* data_for_mem;
		data_for_mem  = ppl->opcode_params.rd;
		if (SW == code) is_data_in_cache = Cache_WriteData(&ppl->cache_data, registers_address, *data_for_mem);
		else is_data_in_cache = Cache_ReadData(&ppl->cache_data, registers_address, data_for_mem);
		bool add_stall = !is_data_in_cache;
		ppl->memory_stall = add_stall;
	}
}

/*!
******************************************************************************
\brief
Write back stage of the pipeline.
\param
 [in]  Pipeline_s *pipeline - Pointer to the relevant pipeline.
 [out] none
\return none
*****************************************************************************/
static void writeback(Pipeline_s* ppl)
{
	//inst instuction = { .cmd = pipeline->pipe_stages[WRITE_BACK].instruction.cmd };
	uint32_t instruction = ppl->pipe_stages[WRITE_BACK].instruction;
	int index_for_jump;
	if (JAL == get_command_opcode(instruction)) index_for_jump = NEXT_INSTRUCTION_ADDRESS_REGISTER;
	else index_for_jump = get_command_rd(instruction);
	ppl->core_registers_p[index_for_jump] = ppl->pipe_stages[WRITE_BACK].execute_result;
}

/*!
******************************************************************************
\brief
Preparing the parans struct for the operations functions.
\param
 [in]  Pipeline_s *pipeline - Pointer to the relevant pipeline.
 [in]  PipelineSM_e stage - The pipeline stage this function is called.
 [out] none
\return none
*****************************************************************************/
static void prepare_registers_params(Pipeline_s* ppl, PipelineSM_e ppl_phase)
{
	//inst instuction = { .cmd = pipeline->pipe_stages[stage].instruction.cmd };
	uint32_t instruction;
	instruction = ppl->pipe_stages[ppl_phase].instruction;
	ppl->core_registers_p[REG_IMM] = get_command_immediate(instruction);
	ppl->pipe_stages[ppl_phase].execute_result = ppl->core_registers_p[get_command_rd(instruction)];
	ppl->opcode_params.rs = ppl->core_registers_p[get_command_rs(instruction)];
	ppl->opcode_params.rt = ppl->core_registers_p[get_command_rt(instruction)];
	ppl->opcode_params.rd = &ppl->pipe_stages[ppl_phase].execute_result;
}

/*!
******************************************************************************
\brief
Executing the pipeline.
\param
 [in]  Pipeline_s *pipeline - Pointer to the relevant pipeline.
 [out] none
\return none
*****************************************************************************/
static void execute_stages(Pipeline_s* pipeline)
{
	uint8_t stage = pipeline->memory_stall ? MEM : pipeline->data_hazard_stall ? EXECUTE : DECODE;
	if (!pipeline->halted)
		pipe_functions[FETCH](pipeline);

	for (; stage < PIPELINE_SIZE; stage++)
	{
		if (!(pipeline->pipe_stages[stage].pc == MAX_INTEGER))
		{
			pipe_functions[stage](pipeline);
		}
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
static bool compare_register(Pipeline_s* pipeline, uint16_t reg, uint16_t stage)
{
	bool ret = false;
	uint32_t decode_ins = pipeline->pipe_stages[DECODE].instruction;
	uint16_t op = get_command_opcode(pipeline->pipe_stages[WRITE_BACK].instruction);
	uint16_t rs, rd, rt, opcode;
	rs = get_command_rs(decode_ins);
	rt = get_command_rt(decode_ins);
	rd = get_command_rd(decode_ins);
	opcode = get_command_opcode(decode_ins);


	if (reg == REG_IMM || reg == REG_ZERO)
	{
		ret = false;
	}
	else if (opcode <= SRL || opcode == LW ||
		(opcode == SW && op == SW))
	{
		ret = (reg == rs || reg == rt);
	}
	else
	{
		ret = (reg == rd || reg == rs || reg == rt);
	}

	return ret;
}
bool reg_compare_logic(uint32_t instruction, uint16_t reg, uint16_t op_code) {
	if (get_command_opcode(instruction) == LW || get_command_opcode(instruction) <= SRL || (get_command_opcode(instruction) == SW && SW == op_code))

		return (reg_compare_helper(reg, get_command_rs(instruction)) || reg_compare_helper(reg, get_command_rt(instruction)));

	if (reg == REG_IMM || reg == REG_ZERO) return false;
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
static bool check_registers_hazrads(Pipeline_s* pipeline, PipelineSM_e stage)
{
	if (pipeline->pipe_stages[stage].pc == MAX_INTEGER)
	{
		return false;
	}
	//return compare_register(pipeline, pipeline->pipe_stages[stage].instruction.bits.rd, stage);
	//return compare_register(pipeline, pipeline->pipe_stages[stage].instruction.bits.rd);
	return compare_register(pipeline, get_command_rd(pipeline->pipe_stages[stage].instruction), stage);
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
static bool pipeline_needs_data_hazard_stall(Pipeline_s* pipeline)
{
	return check_registers_hazrads(pipeline, EXECUTE) || check_registers_hazrads(pipeline, WRITE_BACK) ||
		check_registers_hazrads(pipeline, MEM);
}

/*!
******************************************************************************
\brief
Updating staistics struct.
\param
 [in] core - the operating core.
\return none
*****************************************************************************/
static void update_statistics(Pipeline_s* pipeline)
{
	if (pipeline->data_hazard_stall && !pipeline->memory_stall)
		pipeline->statistics.decode_stalls++;

	if (pipeline->memory_stall)
		pipeline->statistics.mem_stalls++;
}