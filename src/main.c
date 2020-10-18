#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <tlhelp32.h>
#include <direct.h>
#include "console.h"

int main()
{
	_chdir("..\\..");

	console_open("app-monitor", 100, 100, 80, 40);
	while(console_is_open())
	{
		console_rect(rect(point(0, 0), point(50, 50)), 'A', CLR_RED_0, CLR_GRAY_2);
		console_update();
	}
	return 0;

	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (snapshot == INVALID_HANDLE_VALUE)
		return;

	PROCESSENTRY32 proc;
	memset(&proc, 0, sizeof(proc));
	proc.dwSize = sizeof(PROCESSENTRY32);

	bool result = Process32First(snapshot, &proc);
	i32 proc_id = 0;
	while(result)
	{
		if (strcmp(proc.szExeFile, "test-app.exe") == 0)
		{
			printf("%s, ID: %d\n", proc.szExeFile, proc.th32ProcessID);
			proc_id = proc.th32ProcessID;
			break;
		}

		result = Process32Next(snapshot, &proc);
	}

	if (proc_id == 0)
	{
		printf("Failed to find process\n");
		return 1;
	}

	if (!DebugActiveProcess(proc_id))
	{
		printf("Failed to debug process, %d\n", GetLastError());
		return 1;
	}

	HANDLE proc_hndl = OpenProcess(PROCESS_ALL_ACCESS, true, proc_id);
	HANDLE mem_snap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, proc_id);
	MODULEENTRY32 entry;
	entry.dwSize = sizeof(entry);

	result = Module32First(mem_snap, &entry);
	while(result)
	{
		printf("Name: %s\n", entry.szModule);
		printf("Base Ptr: %X\n", entry.modBaseAddr);
		printf("Base Size: %X\n", entry.modBaseSize);
		printf("\n");

		result = Module32Next(mem_snap, &entry);
	}

	system("pause");
	DebugActiveProcessStop(proc_id);
}