#include "fasmFunctions.h"

#include <iostream>
#include <string>
#include <cassert>

#include "fasmcpp.h"

#if defined _WIN64
#	include "fasmcpp.win64.h"
#elif defined _WIN32
#	include "fasmcpp.win32.h"
extern "C" {
#endif

void ASM fasmError(fasmcpp::Assembly* assembly, const char* error)
{
	assembly->addError(error, AssemblyKey());
}

int ASM fasmOpen(fasmcpp::Assembly* assembly, const char* filename)
{
	std::string filenameStr = filename;
	if (filenameStr == _input_filename)
	{
		assembly->getInput(AssemblyKey()).open();
		return 1;
	}
	else if (filenameStr == _output_filename)
	{
		assembly->getOutput(AssemblyKey()).open();
		return 2;
	}
	else
	{
		return 0;
	}
}

void ASM fasmRead(fasmcpp::Assembly* assembly, uint32_t handle, char* buffer, uint32_t size)
{
	assert(handle == 1);
	assembly->getInput(AssemblyKey()).read(buffer, size);
}

void ASM fasmWrite(fasmcpp::Assembly* assembly, uint32_t handle, const char* buffer, uint32_t size)
{
	assert(handle == 2);
	assembly->getOutput(AssemblyKey()).write(buffer, size);
}

void ASM fasmClose(fasmcpp::Assembly* assembly, uint32_t handle)
{
	if (handle == 1)
	{
		assembly->getInput(AssemblyKey()).close();
	}
	else if (handle == 2)
	{
		assembly->getOutput(AssemblyKey()).close();
	}
	else
	{
		assert(0);
	}
}

uint32_t ASM fasmLseek(fasmcpp::Assembly* assembly, uint32_t handle, int32_t offset, uint8_t mode)
{
	if (handle == 1)
	{
		_fasmcpp::AssemblerInput& input = assembly->getInput(AssemblyKey());
		input.setPosition(static_cast<int>(offset),
		                  static_cast<_fasmcpp::AssemblerIO::OffsetMode>(mode));

		assert(input.getPosition() <= std::numeric_limits<uint32_t>::max());
		return static_cast<uint32_t>(input.getPosition());
	}
	else if (handle == 2)
	{
		_fasmcpp::AssemblerOutput& output = assembly->getOutput(AssemblyKey());
		output.setPosition(static_cast<int>(offset),
		                  static_cast<_fasmcpp::AssemblerIO::OffsetMode>(mode));

		assert(output.getPosition() <= std::numeric_limits<uint32_t>::max());
		return static_cast<uint32_t>(output.getPosition());
	}
	else
	{
		assert(0);
		return 0;
	}
}

void ASM fasmDisplayBlock(fasmcpp::Assembly* assembly, const char* message, uint32_t size)
{
	assembly->addMessage(std::string(message, size).c_str(), AssemblyKey());
}

#if defined _WIN32 && !defined _WIN64
}
#endif
