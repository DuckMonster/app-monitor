#pragma once

typedef struct
{
	void* base_ptr;
	u32 size;

	const char* name;
} Modules;

typedef struct
{
	HANDLE handle;
	i32 id;

	const char* name;
} Process;