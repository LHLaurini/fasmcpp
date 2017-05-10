#pragma once

#include <cstdint>

namespace fasmcpp { class Assembly; }

extern "C" void __stdcall _assemble(fasmcpp::Assembly* assembly, void* memory, size_t size, const char* predefinitions);
extern "C" uint64_t __stdcall _run(const void* memory);

extern "C" const char _input_filename[];
extern "C" const char _output_filename[];
