#pragma once

#include <cstdint>

namespace fasmcpp { class Assembly; }

struct FasmBinaryHeader
{
	void (*_assemble)(fasmcpp::Assembly* assembly, void* memory, size_t size, const char* predefinitions);
	uint64_t (*_run)(const void* memory);
	const char* _input_filename;
	const char* _output_filename;
	char* _fasm_memory;

	void (*fasmError)(fasmcpp::Assembly*, const char*);
	int (*fasmOpen)(fasmcpp::Assembly*, const char*);
	void (*fasmRead)(fasmcpp::Assembly*, uint32_t, char*, uint32_t);
	void (*fasmWrite)(fasmcpp::Assembly*, uint32_t, const char*, uint32_t);
	void (*fasmClose)(fasmcpp::Assembly*, uint32_t);
	uint32_t (*fasmLseek)(fasmcpp::Assembly*, uint32_t, int32_t, uint8_t);
	void (*fasmDisplayBlock)(fasmcpp::Assembly*, const char*, uint32_t);
};

#define _BINARY_HEADER reinterpret_cast<FasmBinaryHeader*>(WIN64_LOW_MEMORY_FIXED_ADDRESS)
#define _assemble (_BINARY_HEADER->_assemble)
#define _run (_BINARY_HEADER->_run)
#define _input_filename (_BINARY_HEADER->_input_filename)
#define _output_filename (_BINARY_HEADER->_output_filename)
