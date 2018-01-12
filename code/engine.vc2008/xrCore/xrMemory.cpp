#include "stdafx.h"
#pragma hdrstop

#include	"xrsharedmem.h"
#include	<malloc.h>

xrMemory Memory;
bool mem_initialized	= false;
bool shared_str_initialized	= false;

//fake fix of memory corruptions in multiplayer game :(
XRCORE_API bool g_allow_heap_min = true;

void xrMemory::_initialize()
{
	stat_calls				= 0;
	stat_counter			= 0;

	ConditionalInitPureAlloc();

	if (!use_pure_alloc)
	{
		// initialize POOLs
		u32	element = mem_pools_ebase;
		u32 sector = mem_pools_ebase * 1024;
		for (u32 pid = 0; pid < mem_pools_count; pid++)
		{
			mem_pools[pid]._initialize(element, sector, 0x1);
			element += mem_pools_ebase;
		}
	}

	mem_initialized				= true;
	g_pStringContainer			= new str_container();
	shared_str_initialized		= true;
	g_pSharedMemoryContainer	= new smem_container();
}

void xrMemory::_destroy()
{
	xr_delete(g_pSharedMemoryContainer);
	xr_delete(g_pStringContainer);

	mem_initialized = false;
}

inline const size_t external_size = -1;

void xrMemory::mem_compact()
{
	RegFlushKey(HKEY_CLASSES_ROOT);
	RegFlushKey(HKEY_CURRENT_USER);

	if (g_allow_heap_min)
		_heapmin();

	HeapCompact(GetProcessHeap(), 0);

	if (g_pStringContainer)			g_pStringContainer->clean();
	if (g_pSharedMemoryContainer)	g_pSharedMemoryContainer->clean();

	if (strstr(Core.Params, "-swap_on_compact"))
	{
		SetProcessWorkingSetSize(GetCurrentProcess(), external_size, external_size);
	}
}

void xrMemory::ConditionalInitPureAlloc()
{
	if (!use_pure_alloc_initialized)
	{
		use_pure_alloc_initialized = true;
 		use_pure_alloc =
 #	ifdef XRCORE_STATIC
 			true
 #	else // XRCORE_STATIC
 			!!strstr(GetCommandLine(), "-pure_alloc")
 #	endif // XRCORE_STATIC
 			;
	}
}

// xr_strdup
char* xr_strdup(const char* string)
{
	VERIFY(string);
	size_t len = xr_strlen(string) + 1;
	char *	memory = (char*)Memory.mem_alloc(len);
	std::memcpy(memory, string, len);
	return	memory;
}

XRCORE_API bool is_stack_ptr( void* _ptr)
{
	int			local_value		= 0;
	void*		ptr_refsound	= _ptr;
	void*		ptr_local		= &local_value;
	ptrdiff_t	difference		= (ptrdiff_t)_abs(s64(ptrdiff_t(ptr_local) - ptrdiff_t(ptr_refsound)));
	return		(difference < (512*1024));
}
