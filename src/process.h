#pragma once

typedef struct
{
	void* base_ptr;
	u32 size;

	const char* name;
} Modules;

typedef struct
{
	i32 id;
	const char* name;
} Process;

bool get_all_processes(Process** out_proc, u32* out_num);