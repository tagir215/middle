#include "platform.h"
#include <cassert>
#include <windows.h>

void* platform_load_dynamic_library(char* dll)
{
    HMODULE result = LoadLibraryA(dll);
    assert(result);
    return result;
}

void* platform_load_dynamic_function(void* dll, char* fun_name)
{
    FARPROC proc = GetProcAddress((HMODULE)dll, fun_name);
    assert(proc);
    return (void*)proc;
}

bool platform_free_dynamic_library(void* dll)
{
    BOOL freeResult = FreeLibrary((HMODULE)dll);
    assert(freeResult);
    return (bool)freeResult;
}

void set_window_always_on_top(void* hwnd) {
    HWND realHwnd = reinterpret_cast<HWND>(hwnd);
    assert(realHwnd);
	SetWindowPos(realHwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
}

