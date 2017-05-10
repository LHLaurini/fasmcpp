#include "fasmcpp.win64.h"

#include "fasmcpp.h"
#include "fasmFunctions.h"

#include <fstream>
#include <Windows.h>

fasmcpp::Assembler::Assembler()
{
	_assemblerMemory = nullptr;
	_assemblerMemorySize = 0;
}

fasmcpp::Assembler::Assembler(size_t memory, size_t predefinitionsMemory)
{
	std::ifstream file("fasmcpp.win64.bin", std::ios::binary);

	if (!file)
	{
		throw std::runtime_error("Fatal error: flat assembler binary not found.");
	}

	file.seekg(0, std::ios::end);
	size_t size = file.tellg();
	file.seekg(0);

	char* address = reinterpret_cast<char*>(WIN64_LOW_MEMORY_FIXED_ADDRESS);

	if (address + size + memory + predefinitionsMemory > reinterpret_cast<char*>(0xFFFFFFFFUi64))
	{
		throw std::length_error("Error: the requested amount of assembler memory is too large and "
		                        "cannot fit entirely in 32-bit memory.");
	}

	FasmBinaryHeader header;
	char* headerPtr = reinterpret_cast<char*>(&header);
	file.read(headerPtr, sizeof(header));

	header.fasmError = fasmError;
	header.fasmOpen = fasmOpen;
	header.fasmRead = fasmRead;
	header.fasmWrite = fasmWrite;
	header.fasmClose = fasmClose;
	header.fasmLseek = fasmLseek;
	header.fasmDisplayBlock = fasmDisplayBlock;

	size_t memoryQty = header._fasm_memory - address + memory + predefinitionsMemory;

	if (!VirtualAlloc(address, memoryQty, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE))
	{
		throw std::runtime_error("Fatal error: failed to allocate memory for Flat Assembler.");
	}

	std::copy(headerPtr, headerPtr + sizeof(FasmBinaryHeader), address);
	file.read(address + sizeof(FasmBinaryHeader), size - sizeof(FasmBinaryHeader));

	_assemblerMemory = header._fasm_memory;
	_assemblerMemorySize = static_cast<uint32_t>(memory);

	_predefinitionsMemory = header._fasm_memory + memory;
	_predefinitionsMemorySize = static_cast<uint32_t>(predefinitionsMemory);
}

fasmcpp::Assembler::~Assembler()
{
	if (_assemblerMemory)
	{
		VirtualFree(reinterpret_cast<void*>(WIN64_LOW_MEMORY_FIXED_ADDRESS), 0, MEM_RELEASE);
		_assemblerMemory = nullptr;
		_assemblerMemorySize = 0;
	}
}

void fasmcpp::Assembly::allocateExecutableMemory(size_t size)
{
	void* memory = VirtualAlloc(NULL, size, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	if (memory)
	{
		_executableMemory = memory;
		_executableMemorySize = size;
	}
	else
	{
		throw std::runtime_error("Fatal error: failed to allocate executable memory.");
	}
}

void fasmcpp::Assembly::freeExecutableMemory()
{
	if (_executableMemory)
	{
		VirtualFree(_executableMemory, 0, MEM_RELEASE);
		_executableMemory = nullptr;
	}
}

const char* fasmcpp::Assembly::rebuildPredefinitionsString()
{
	char* predefinitionsString = _assembler->getPredefinitionsMemory();

	if (_predefinitionsChanged)
	{
		char* end = predefinitionsString;
		ptrdiff_t memoryLeft = _assembler->getPredefinitionsMemorySize();

		for (auto& i : _predefinitions)
		{
			memoryLeft -= i.first.size() + i.second.size() + 2;
			if (memoryLeft < 1)
			{
				throw std::runtime_error("Error: out of predefinitions memory.");
			}

			*end++ = static_cast<char>(i.first.size());

			std::copy(i.first.begin(), i.first.end(), end);
			end += i.first.size();

			strcpy(end, i.second.c_str());
			end += i.second.size() + 1;
		}

		*end = 0;
	}

	return predefinitionsString;
}
