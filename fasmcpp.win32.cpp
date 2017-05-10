#include "fasmcpp.win32.h"

#include "fasmcpp.h"

#include <Windows.h>

fasmcpp::Assembler::Assembler()
{
	_assemblerMemory = nullptr;
	_assemblerMemorySize = 0;
}

fasmcpp::Assembler::Assembler(size_t memory, size_t)
{
	try
	{
		_assemblerMemory = new char[memory];
		_assemblerMemorySize = static_cast<uint32_t>(memory);
	}
	catch (const std::bad_alloc&)
	{
		throw std::runtime_error("Fatal error: failed to allocate memory for Flat Assembler.");
	}
}

fasmcpp::Assembler::~Assembler()
{
	if (_assemblerMemory)
	{
		delete[] _assemblerMemory;
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
