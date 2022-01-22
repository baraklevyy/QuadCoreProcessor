#define _CRT_SECURE_NO_WARNINGS
#include "core.h"
#include <string.h>
#include "helper.h"


static data_of_core cores[CORES_NUMBER];
current_core_data_files files_of_cores[CORES_NUMBER];


int open_files(char* argv[], int number_of_arguments);
void close_all_files();
enum core_E core_number;
enum file_names_E;


static void print_register_file(data_of_core* core);
static void print_statistics(data_of_core* core);

static uint32_t main_memory[MAIN_MEMORY_SIZE];
static uint8_t main_memory_line_counter;
static bool is_memory_operation;
void print_main_memory();
static uint32_t get_memory_length();
static bool bus_memory_exec_function(data_on_bus* bus_container, bool direct_transaction);
void memory_initalize();

int main(int argc, char* argv[]){
	if (false == open_files(argv, argc)){
		printf("Cannot open files.\n");
		return failed_op;
	}
	memory_initalize();
	for (int core = 0; core < CORES_NUMBER; core++) {
		cores[core].core_files = files_of_cores[core];
	}
	for(int i = 0; i < CORES_NUMBER; i++){
		initialize_core(&cores[i], i);
	}
	while(!cores[0].is_core_terminated || !cores[1].is_core_terminated || !cores[2].is_core_terminated || !cores[3].is_core_terminated){//processor finished
		operate_bus();
		operate_the_core(&cores[core0]);
		operate_the_core(&cores[core1]);
		operate_the_core(&cores[core2]);
		operate_the_core(&cores[core3]);
	}

	for (int core = 0; core < CORES_NUMBER; core++){
		data_of_core* current_core;
		current_core = &(*(cores + core));
		print_register_file(current_core);
		cache_print_to_file(&current_core->pipeline.current_data_from_cache, current_core->core_files.dsram_files, current_core->core_files.tsram_files);
		print_statistics(current_core);
	}

	print_main_memory();
	void close_all_files();
	return success_op;
}






void close_all_files() {
	for (int core_index = 0; core_index < CORES_NUMBER; ++core_index){
		fclose(files_of_cores[core_index].dsram_files);
		fclose(files_of_cores[core_index].immediate_memory_file);
		fclose(files_of_cores[core_index].register_out_files);
		fclose(files_of_cores[core_index].statistics_files);
		fclose(files_of_cores[core_index].core_trace_files);
		fclose(files_of_cores[core_index].tsram_files);
	}
	fclose(memory_input_file);
	fclose(memory_output_file);
	fclose(bus_trace_file);
}

int open_files(char* argv[], int number_of_arguments)
{
	if (!(1 == number_of_arguments)) {// arguments are passed
		memory_input_file = fopen(argv[memin], "r");
		memory_output_file = fopen(argv[memout], "w");
		bus_trace_file =fopen(argv[bustrace], "w");

		files_of_cores[core0].immediate_memory_file =fopen(argv[imem0], "r");
		files_of_cores[core0].register_out_files =fopen(argv[regout0], "w");
		files_of_cores[core0].core_trace_files =fopen(core0trace, "w");
		files_of_cores[core0].dsram_files =fopen(dsram0, "w");
		files_of_cores[core0].tsram_files =fopen(tsram0, "w");
		files_of_cores[core0].statistics_files =fopen(stats0, "w");

		files_of_cores[core1].immediate_memory_file = fopen(imem1, "r");
		files_of_cores[core1].register_out_files =fopen(regout1, "w");
		files_of_cores[core1].core_trace_files =fopen(core1trace, "w");
		files_of_cores[core1].dsram_files =fopen(dsram1, "w");
		files_of_cores[core1].tsram_files =fopen(tsram1, "w");
		files_of_cores[core1].statistics_files =fopen(stats1, "w");

		files_of_cores[core2].immediate_memory_file =fopen(imem2, "r");
		files_of_cores[core2].register_out_files = fopen(regout2, "w");
		files_of_cores[core2].core_trace_files =fopen(core2trace, "w");
		files_of_cores[core2].dsram_files =fopen(dsram2, "w");
		files_of_cores[core2].tsram_files =fopen(tsram2, "w");
		files_of_cores[core2].statistics_files = fopen(stats2, "w");

		files_of_cores[core3].immediate_memory_file =fopen(imem3, "r");
		files_of_cores[core3].register_out_files =fopen(regout3, "w");
		files_of_cores[core3].core_trace_files =fopen(core3trace, "w");
		files_of_cores[core3].dsram_files =fopen(dsram3, "w");
		files_of_cores[core3].tsram_files =fopen(tsram3, "w");
		files_of_cores[core3].statistics_files =fopen(stats3, "w");
	}
	else { //default arguments
		memory_input_file = fopen("memin.txt", "r");
		memory_output_file = fopen("memout.txt", "w");
		bus_trace_file = fopen("bustrace.txt", "w");

		files_of_cores[core0].immediate_memory_file = fopen("imem0.txt", "r");
		files_of_cores[core0].register_out_files = fopen("regout0.txt", "w");
		files_of_cores[core0].core_trace_files = fopen("core0trace.txt", "w");
		files_of_cores[core0].dsram_files = fopen("dsram0.txt", "w");
		files_of_cores[core0].tsram_files = fopen("tsram0.txt", "w");
		files_of_cores[core0].statistics_files = fopen("stats0.txt", "w");

		files_of_cores[core1].immediate_memory_file = fopen("imem1.txt", "r");
		files_of_cores[core1].register_out_files = fopen("regout1.txt", "w");
		files_of_cores[core1].core_trace_files = fopen("core1trace.txt", "w");
		files_of_cores[core1].dsram_files = fopen("dsram1.txt", "w");
		files_of_cores[core1].tsram_files = fopen("tsram1.txt", "w");
		files_of_cores[core1].statistics_files = fopen("stats1.txt", "w");

		files_of_cores[core2].immediate_memory_file = fopen("imem2.txt", "r");
		files_of_cores[core2].register_out_files = fopen("regout2.txt", "w");
		files_of_cores[core2].core_trace_files = fopen("core2trace.txt", "w");
		files_of_cores[core2].dsram_files = fopen("dsram2.txt", "w");
		files_of_cores[core2].tsram_files = fopen("tsram2.txt", "w");
		files_of_cores[core2].statistics_files = fopen("stats2.txt", "w");

		files_of_cores[core3].immediate_memory_file = fopen("imem3.txt", "r");
		files_of_cores[core3].register_out_files = fopen("regout3.txt", "w");
		files_of_cores[core3].core_trace_files = fopen("core3trace.txt", "w");
		files_of_cores[core3].dsram_files = fopen("dsram3.txt", "w");
		files_of_cores[core3].tsram_files = fopen("tsram3.txt", "w");
		files_of_cores[core3].statistics_files = fopen("stats3.txt", "w");

	}












/*

	memory_input_file = is_default_parameters ? fopen("memin.txt", "r") : fopen(argv[5], "r");
	memory_output_file = is_default_parameters ? fopen("memout.txt", "w") : fopen(argv[6], "w");
	bus_trace_file = is_default_parameters ? fopen("bustrace.txt", "w") : fopen(argv[15], "w");

	files_of_cores[0].immediate_memory_file = is_default_parameters ? fopen("imem0.txt", "r") : fopen(argv[1], "r");
	files_of_cores[0].register_out_files = is_default_parameters ? fopen("regout0.txt", "w") : fopen(argv[7], "w");
	files_of_cores[0].core_trace_files = is_default_parameters ? fopen("core0trace.txt", "w") : fopen(argv[11], "w");
	files_of_cores[0].dsram_files = is_default_parameters ? fopen("dsram0.txt", "w") : fopen(argv[16], "w");
	files_of_cores[0].tsram_files = is_default_parameters ? fopen("tsram0.txt", "w") : fopen(argv[20], "w");
	files_of_cores[0].statistics_files = is_default_parameters ? fopen("stats0.txt", "w") : fopen(argv[24], "w");

	files_of_cores[1].immediate_memory_file = is_default_parameters ? fopen("imem1.txt", "r") : fopen(argv[2], "r");
	files_of_cores[1].register_out_files = is_default_parameters ? fopen("regout1.txt", "w") : fopen(argv[8], "w");
	files_of_cores[1].core_trace_files = is_default_parameters ? fopen("core1trace.txt", "w") : fopen(argv[12], "w");
	files_of_cores[1].dsram_files = is_default_parameters ? fopen("dsram1.txt", "w") : fopen(argv[17], "w");
	files_of_cores[1].tsram_files = is_default_parameters ? fopen("tsram1.txt", "w") : fopen(argv[21], "w");
	files_of_cores[1].statistics_files = is_default_parameters ? fopen("stats1.txt", "w") : fopen(argv[25], "w");

	files_of_cores[2].immediate_memory_file = is_default_parameters ? fopen("imem2.txt", "r") : fopen(argv[3], "r");
	files_of_cores[2].register_out_files = is_default_parameters ? fopen("regout2.txt", "w") : fopen(argv[9], "w");
	files_of_cores[2].core_trace_files = is_default_parameters ? fopen("core2trace.txt", "w") : fopen(argv[13], "w");
	files_of_cores[2].dsram_files = is_default_parameters ? fopen("dsram2.txt", "w") : fopen(argv[18], "w");
	files_of_cores[2].tsram_files = is_default_parameters ? fopen("tsram2.txt", "w") : fopen(argv[22], "w");
	files_of_cores[2].statistics_files = is_default_parameters ? fopen("stats2.txt", "w") : fopen(argv[26], "w");

	files_of_cores[3].immediate_memory_file = is_default_parameters ? fopen("imem3.txt", "r") : fopen(argv[4], "r");
	files_of_cores[3].register_out_files = is_default_parameters ? fopen("regout3.txt", "w") : fopen(argv[10], "w");
	files_of_cores[3].core_trace_files = is_default_parameters ? fopen("core3trace.txt", "w") : fopen(argv[14], "w");
	files_of_cores[3].dsram_files = is_default_parameters ? fopen("dsram3.txt", "w") : fopen(argv[19], "w");
	files_of_cores[3].tsram_files = is_default_parameters ? fopen("tsram3.txt", "w") : fopen(argv[23], "w");
	files_of_cores[3].statistics_files = is_default_parameters ? fopen("stats3.txt", "w") : fopen(argv[27], "w");
	
	//check all the core files for error in opening
	*/
	for (int i = 0; i < CORES_NUMBER; i++)
	{
		if (NULL == files_of_cores[i].core_trace_files || NULL == files_of_cores[i].dsram_files || NULL == files_of_cores[i].immediate_memory_file || NULL == files_of_cores[i].register_out_files || NULL == files_of_cores[i].statistics_files || NULL == files_of_cores[i].tsram_files)
			return false;
	}
	//check the rest of files
	if (NULL == memory_input_file || NULL == memory_output_file || NULL == bus_trace_file) return false;
	return true;
}
static void print_statistics(data_of_core* core) {
	fprintf(core->core_files.statistics_files, "cycles %d\n", core->number_of_cycles + 1);
	fprintf(core->core_files.statistics_files, "instructions %d\n", core->number_of_instructions - 1);
	fprintf(core->core_files.statistics_files, "read_hit %d\n", core->pipeline.current_data_from_cache.number_of_read_hit);
	fprintf(core->core_files.statistics_files, "write_hit %d\n", core->pipeline.current_data_from_cache.number_of_write_hit);
	fprintf(core->core_files.statistics_files, "read_miss %d\n", core->pipeline.current_data_from_cache.number_of_read_miss);
	fprintf(core->core_files.statistics_files, "write_miss %d\n", core->pipeline.current_data_from_cache.number_of_write_miss);
	fprintf(core->core_files.statistics_files, "decode_stall %d\n", core->pipeline.current_pip_decode_stalls);
	fprintf(core->core_files.statistics_files, "mem_stall %d\n", core->pipeline.current_pip_memory_stalls);
}
static void print_register_file(data_of_core* core) {
	//reg1, reg2 are fixed and don't have to be printed
	for (int i = 2; i < 16; i++) {
		uint32_t* tmp;
		tmp = core->data_register;
		fprintf(core->core_files.register_out_files, "%08X\n", *(tmp + i));
	}
}
void memory_initalize() {

	main_memory_line_counter = 0;
	is_memory_operation = false;

	uint16_t lineInProgram = 0;
	while (lineInProgram < MAIN_MEMORY_SIZE && fscanf(memory_input_file, "%08x", (uint32_t*)&(main_memory[lineInProgram])) != -1)
		lineInProgram = lineInProgram + 1;
	set_bus_memory_func(bus_memory_exec_function);
}
static bool bus_memory_exec_function(data_on_bus* bus_container, bool direct_transaction)
{
	if (bus_container->command_on_bus == idle_cmd_on_bus)
		return false;

	if (!is_memory_operation)
	{
		is_memory_operation = true;
		main_memory_line_counter = !direct_transaction ? 0 : 16;
	}

	if (main_memory_line_counter >= 16)
	{
		if (bus_container->command_on_bus == bus_read_cmd_on_bus || bus_container->command_on_bus == bus_read_exclusive_cmd_on_bus)
		{
			// send the memory value
			bus_container->origid_on_bus = main_memory_on_bus;
			bus_container->command_on_bus = bus_flush_cmd_on_bus;
			bus_container->data_on_bus = main_memory[bus_container->address_on_bus];
		}
		else if (bus_container->command_on_bus == bus_flush_cmd_on_bus)
		{
			// write data to memory
			main_memory[bus_container->address_on_bus] = bus_container->data_on_bus;
		}

		if (main_memory_line_counter == 19)
			is_memory_operation = false;

		main_memory_line_counter++;
		return true;
	}

	main_memory_line_counter++;
	return false;
}

static uint32_t get_memory_length(void)
{
	uint32_t length = MAIN_MEMORY_SIZE - 1;

	for (; length > 0; length--)
	{
		if (main_memory[length])
			break;
	}

	return length + 1;
}
void print_main_memory()
{
	uint32_t length = get_memory_length();
	for (uint32_t i = 0; i < length; i++)
		fprintf(memory_output_file, "%08X\n", main_memory[i]);
}
