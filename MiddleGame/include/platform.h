#pragma once
#include <string>

void* platform_load_dynamic_library(char* dll);
void* platform_load_dynamic_function(void* dll, char* fun_name);
bool platform_free_dynamic_library(void* dll);
void set_window_always_on_top(void* hwnd);
