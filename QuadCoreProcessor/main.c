
#include "files.h"
#include "core.h"
#include "mainMemory.h"
#include "bus.h"
#include <string.h>


static Core_s cores[NUMBER_OF_CORES];


static bool ProcessorHalted();
static void AssignFiles(Core_s* cores);
static void CoresInit();




int main(int argc, char* argv[])
{
	if (FIles_TryToOpenFIles(argv, argc))
	{
		printf("Error in opening files.\n");
		return 1;
	}

	MainMemory_Init();
	CoresInit();

	while (!ProcessorHalted())
	{
		Bus_Iter();
		for (int core = 0; core < NUMBER_OF_CORES; core++)
		{
			Core_Iter(&cores[core]);
		}
	}

	for (int core = 0; core < NUMBER_OF_CORES; core++)
	{
		Core_Teaddown(&cores[core]);
	}

	MainMemory_PrintData();
	CloseFiles();
	return 0;
}


static bool ProcessorHalted()
{
	bool is_halted = true;
	for (int core = 0; core < NUMBER_OF_CORES; core++)
	{
		is_halted &= Core_Halted(&cores[core]);
	}

	return is_halted;
}


static void AssignFiles(Core_s* cores)
{
	for (int core = 0; core < NUMBER_OF_CORES; core++)
	{
		cores[core].core_files = CoresFilesArray[core];
	}
}


void CoresInit()
{
	memset(cores, 0, NUMBER_OF_CORES * sizeof(Core_s));
	AssignFiles(cores);
	for (int i = 0; i < NUMBER_OF_CORES; i++)
	{
		Core_Init(&cores[i], i);
	}
}