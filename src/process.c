#include "process.h"
#include <tlhelp32.h>

bool get_all_processes(Process** out_proc, u32* out_num)
{
	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

	/* Count number of processes */
	u32 count = 0;
	PROCESSENTRY32 proc;
	bool result = Process32First(snapshot, &proc);

	while(result)
	{
		count++;
		result = Process32Next(snapshot, &proc);
	}

	/* Allocate and fetch */
	Process* proc_list = (Process*)malloc(sizeof(Process) * count);
	memzero(proc_list, sizeof(Process) * count);
}