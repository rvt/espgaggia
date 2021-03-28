
#pragma once
#include <gaggiaio.hpp>
#include <gaggiascriptcontext.hpp>

void gaggia_scripting_init(GaggiaIO* gaggiaIO);
void gaggia_scripting_load(const char*);
int8_t gaggia_scripting_handle();
GaggiaScriptContext* gaggia_scripting_context();
