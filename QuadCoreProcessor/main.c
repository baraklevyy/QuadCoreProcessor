#define _CRT_SECURE_NO_WARNINGS
#include "core.h"
#include "mainMemory.h"
#include "bus.h"
#include <string.h>


static Core_s cores[CORES_NUMBER];
output_core_file files_of_cores[CORES_NUMBER];


static void AssignFiles(Core_s* cores);
static void CoresInit();
int open_files(char* argv[], int number_of_arguments);
void close_all_files();
enum core_E core_number;
enum file_names_E;


int main(int argc, char* argv[])
{
	/*
	uint32_t cmd = 0xbf2519f3;
	uint16_t res;
	res = get_command_opcode(cmd);
	*/



	if (!open_files(argv, argc))
	{
		printf("Cannot open files.\n");
		return BAD_EXIT_CODE;
	}

	MainMemory_Init();
	CoresInit();

	while(!cores[0].core_halted || !cores[1].core_halted || !cores[2].core_halted || !cores[3].core_halted)
	{
		Bus_Iter();
		for (int core = 0; core < CORES_NUMBER; core++)
		{
			Core_Iter(&cores[core]);
		}
	}

	for (int core = 0; core < CORES_NUMBER; core++)
	{
		Core_Teaddown(&cores[core]);
	}

	MainMemory_PrintData();
	void close_all_files();
	return 0;
}



static void AssignFiles(Core_s* cores)
{
	for (int core = 0; core < CORES_NUMBER; core++)
	{
		cores[core].core_files = files_of_cores[core];
	}
}


void CoresInit()
{
	memset(cores, 0, CORES_NUMBER * sizeof(Core_s));
	AssignFiles(cores);
	for (int i = 0; i < CORES_NUMBER; i++)
	{
		Core_Init(&cores[i], i);
	}
}

void close_all_files() {
	for (int core_index = 0; core_index < CORES_NUMBER; ++core_index){
		fclose(files_of_cores[core_index].dsram_F);
		fclose(files_of_cores[core_index].imem_F);
		fclose(files_of_cores[core_index].regout_F);
		fclose(files_of_cores[core_index].StatsFile);
		fclose(files_of_cores[core_index].core_trace_F);
		fclose(files_of_cores[core_index].TsRamFile);
	}
	fclose(MeminFile);
	fclose(MemoutFile);
	fclose(BusTraceFile);
}

int open_files(char* argv[], int number_of_arguments)
{
	if (!(1 == number_of_arguments)) {// arguments are passed
		MeminFile = fopen(argv[memin], "r");
		MemoutFile = fopen(argv[memout], "w");
		BusTraceFile =fopen(argv[bustrace], "w");

		files_of_cores[core0].imem_F =fopen(argv[imem0], "r");
		files_of_cores[core0].regout_F =fopen(argv[regout0], "w");
		files_of_cores[core0].core_trace_F =fopen(core0trace, "w");
		files_of_cores[core0].dsram_F =fopen(dsram0, "w");
		files_of_cores[core0].TsRamFile =fopen(tsram0, "w");
		files_of_cores[core0].StatsFile =fopen(stats0, "w");

		files_of_cores[core1].imem_F = fopen(imem1, "r");
		files_of_cores[core1].regout_F =fopen(regout1, "w");
		files_of_cores[core1].core_trace_F =fopen(core1trace, "w");
		files_of_cores[core1].dsram_F =fopen(dsram1, "w");
		files_of_cores[core1].TsRamFile =fopen(tsram1, "w");
		files_of_cores[core1].StatsFile =fopen(stats1, "w");

		files_of_cores[core2].imem_F =fopen(imem2, "r");
		files_of_cores[core2].regout_F = fopen(regout2, "w");
		files_of_cores[core2].core_trace_F =fopen(core2trace, "w");
		files_of_cores[core2].dsram_F =fopen(dsram2, "w");
		files_of_cores[core2].TsRamFile =fopen(tsram2, "w");
		files_of_cores[core2].StatsFile = fopen(stats2, "w");

		files_of_cores[core3].imem_F =fopen(imem3, "r");
		files_of_cores[core3].regout_F =fopen(regout3, "w");
		files_of_cores[core3].core_trace_F =fopen(core3trace, "w");
		files_of_cores[core3].dsram_F =fopen(dsram3, "w");
		files_of_cores[core3].TsRamFile =fopen(tsram3, "w");
		files_of_cores[core3].StatsFile =fopen(stats3, "w");
	}
	else { //default arguments
		MeminFile = fopen("memin.txt", "r");
		MemoutFile = fopen("memout.txt", "w");
		BusTraceFile = fopen("bustrace.txt", "w");

		files_of_cores[core0].imem_F = fopen("imem0.txt", "r");
		files_of_cores[core0].regout_F = fopen("regout0.txt", "w");
		files_of_cores[core0].core_trace_F = fopen("core0trace.txt", "w");
		files_of_cores[core0].dsram_F = fopen("dsram0.txt", "w");
		files_of_cores[core0].TsRamFile = fopen("tsram0.txt", "w");
		files_of_cores[core0].StatsFile = fopen("stats0.txt", "w");

		files_of_cores[core1].imem_F = fopen("imem1.txt", "r");
		files_of_cores[core1].regout_F = fopen("regout1.txt", "w");
		files_of_cores[core1].core_trace_F = fopen("core1trace.txt", "w");
		files_of_cores[core1].dsram_F = fopen("dsram1.txt", "w");
		files_of_cores[core1].TsRamFile = fopen("tsram1.txt", "w");
		files_of_cores[core1].StatsFile = fopen("stats1.txt", "w");

		files_of_cores[core2].imem_F = fopen("imem2.txt", "r");
		files_of_cores[core2].regout_F = fopen("regout2.txt", "w");
		files_of_cores[core2].core_trace_F = fopen("core2trace.txt", "w");
		files_of_cores[core2].dsram_F = fopen("dsram2.txt", "w");
		files_of_cores[core2].TsRamFile = fopen("tsram2.txt", "w");
		files_of_cores[core2].StatsFile = fopen("stats2.txt", "w");

		files_of_cores[core3].imem_F = fopen("imem3.txt", "r");
		files_of_cores[core3].regout_F = fopen("regout3.txt", "w");
		files_of_cores[core3].core_trace_F = fopen("core3trace.txt", "w");
		files_of_cores[core3].dsram_F = fopen("dsram3.txt", "w");
		files_of_cores[core3].TsRamFile = fopen("tsram3.txt", "w");
		files_of_cores[core3].StatsFile = fopen("stats3.txt", "w");

	}












/*

	MeminFile = is_default_parameters ? fopen("memin.txt", "r") : fopen(argv[5], "r");
	MemoutFile = is_default_parameters ? fopen("memout.txt", "w") : fopen(argv[6], "w");
	BusTraceFile = is_default_parameters ? fopen("bustrace.txt", "w") : fopen(argv[15], "w");

	files_of_cores[0].imem_F = is_default_parameters ? fopen("imem0.txt", "r") : fopen(argv[1], "r");
	files_of_cores[0].regout_F = is_default_parameters ? fopen("regout0.txt", "w") : fopen(argv[7], "w");
	files_of_cores[0].core_trace_F = is_default_parameters ? fopen("core0trace.txt", "w") : fopen(argv[11], "w");
	files_of_cores[0].dsram_F = is_default_parameters ? fopen("dsram0.txt", "w") : fopen(argv[16], "w");
	files_of_cores[0].TsRamFile = is_default_parameters ? fopen("tsram0.txt", "w") : fopen(argv[20], "w");
	files_of_cores[0].StatsFile = is_default_parameters ? fopen("stats0.txt", "w") : fopen(argv[24], "w");

	files_of_cores[1].imem_F = is_default_parameters ? fopen("imem1.txt", "r") : fopen(argv[2], "r");
	files_of_cores[1].regout_F = is_default_parameters ? fopen("regout1.txt", "w") : fopen(argv[8], "w");
	files_of_cores[1].core_trace_F = is_default_parameters ? fopen("core1trace.txt", "w") : fopen(argv[12], "w");
	files_of_cores[1].dsram_F = is_default_parameters ? fopen("dsram1.txt", "w") : fopen(argv[17], "w");
	files_of_cores[1].TsRamFile = is_default_parameters ? fopen("tsram1.txt", "w") : fopen(argv[21], "w");
	files_of_cores[1].StatsFile = is_default_parameters ? fopen("stats1.txt", "w") : fopen(argv[25], "w");

	files_of_cores[2].imem_F = is_default_parameters ? fopen("imem2.txt", "r") : fopen(argv[3], "r");
	files_of_cores[2].regout_F = is_default_parameters ? fopen("regout2.txt", "w") : fopen(argv[9], "w");
	files_of_cores[2].core_trace_F = is_default_parameters ? fopen("core2trace.txt", "w") : fopen(argv[13], "w");
	files_of_cores[2].dsram_F = is_default_parameters ? fopen("dsram2.txt", "w") : fopen(argv[18], "w");
	files_of_cores[2].TsRamFile = is_default_parameters ? fopen("tsram2.txt", "w") : fopen(argv[22], "w");
	files_of_cores[2].StatsFile = is_default_parameters ? fopen("stats2.txt", "w") : fopen(argv[26], "w");

	files_of_cores[3].imem_F = is_default_parameters ? fopen("imem3.txt", "r") : fopen(argv[4], "r");
	files_of_cores[3].regout_F = is_default_parameters ? fopen("regout3.txt", "w") : fopen(argv[10], "w");
	files_of_cores[3].core_trace_F = is_default_parameters ? fopen("core3trace.txt", "w") : fopen(argv[14], "w");
	files_of_cores[3].dsram_F = is_default_parameters ? fopen("dsram3.txt", "w") : fopen(argv[19], "w");
	files_of_cores[3].TsRamFile = is_default_parameters ? fopen("tsram3.txt", "w") : fopen(argv[23], "w");
	files_of_cores[3].StatsFile = is_default_parameters ? fopen("stats3.txt", "w") : fopen(argv[27], "w");
	
	//check all the core files for error in opening
	*/
	for (int i = 0; i < CORES_NUMBER; i++)
	{
		if (NULL == files_of_cores[i].core_trace_F || NULL == files_of_cores[i].dsram_F || NULL == files_of_cores[i].imem_F || NULL == files_of_cores[i].regout_F || NULL == files_of_cores[i].StatsFile || NULL == files_of_cores[i].TsRamFile)
			return false;
	}
	//check the rest of files
	if (NULL == MeminFile || NULL == MemoutFile || NULL == BusTraceFile) return false;
	return true;
}
