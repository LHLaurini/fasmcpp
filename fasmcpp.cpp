#include "fasmcpp.h"

#include "fasmFunctions.h"

#if defined _WIN64
#	include "fasmcpp.win64.h"
#elif defined _WIN32
#	include "fasmcpp.win32.h"
#endif

#include <algorithm>

fasmcpp::Assembler::Assembler(Assembler&& x)
{
	_assemblerMemory = x._assemblerMemory;
	_assemblerMemorySize = x._assemblerMemorySize;
	_predefinitionsMemory = x._predefinitionsMemory;
	_predefinitionsMemorySize = x._predefinitionsMemorySize;
	x._assemblerMemory = nullptr;
	x._assemblerMemorySize = 0;
}

fasmcpp::Assembler& fasmcpp::Assembler::operator=(Assembler&& x)
{
	std::swap(_assemblerMemory, x._assemblerMemory);
	std::swap(_assemblerMemorySize, x._assemblerMemorySize);
	std::swap(_predefinitionsMemory, x._predefinitionsMemory);
	std::swap(_predefinitionsMemorySize, x._predefinitionsMemorySize);
	return *this;
}

void* fasmcpp::Assembler::getMemory() const
{
	return _assemblerMemory;
}

uint32_t fasmcpp::Assembler::getMemorySize() const
{
	return _assemblerMemorySize;
}

char* fasmcpp::Assembler::getPredefinitionsMemory() const
{
	return _predefinitionsMemory;
}

uint32_t fasmcpp::Assembler::getPredefinitionsMemorySize() const
{
	return _predefinitionsMemorySize;
}

fasmcpp::Assembly::Assembly()
{
}

fasmcpp::Assembly::Assembly(const char* code)
	: _code(code)
{
}

fasmcpp::Assembly::Assembly(Assembly&& x)
{
	_assembler = std::move(x._assembler);
	_code = std::move(x._code);
	_bytecode = std::move(x._bytecode);
	_errors = std::move(x._errors);
	_messages = std::move(x._messages);
	_executableMemory = std::move(x._executableMemory);
	_executableMemorySize = std::move(x._executableMemorySize);
}

fasmcpp::Assembly& fasmcpp::Assembly::operator=(Assembly&& x)
{
	std::swap(_assembler, x._assembler);
	std::swap(_code, x._code);
	std::swap(_bytecode, x._bytecode);
	std::swap(_errors, x._errors);
	std::swap(_messages, x._messages);
	std::swap(_executableMemory, x._executableMemory);
	std::swap(_executableMemorySize, x._executableMemorySize);
	return *this;
}

void fasmcpp::Assembly::setAssembler(Assembler* assembler)
{
	_assembler = assembler;
}

fasmcpp::Assembler* fasmcpp::Assembly::getAssembler() const
{
	return _assembler;
}

bool fasmcpp::Assembly::assemble()
{
	if (!_assembler)
	{
		throw std::invalid_argument("Error: no Assembler bound to Assembly object.");
	}

	if (!_assembler->getMemory())
	{
		throw std::invalid_argument("Error: Assembler not initialized.");
	}

	const char* predefinitionsString;

#ifdef _WIN64
	predefinitionsString = rebuildPredefinitionsString();
#else
	if (_predefinitionsChanged)
	{
		_predefinitionsString.clear();
		for (auto& i : _predefinitions)
		{
			_predefinitionsString.emplace_back(static_cast<char>(i.first.size()));

			size_t stringEnd = _predefinitionsString.size();
			_predefinitionsString.resize(stringEnd + i.first.size() + i.second.size() + 1);

			std::copy(i.first.begin(), i.first.end(), _predefinitionsString.data() + stringEnd);
			stringEnd += i.first.size();

			strcpy(_predefinitionsString.data() + stringEnd, i.second.c_str());
		}
		_predefinitionsString.emplace_back(0);
	}
	predefinitionsString = _predefinitionsString.data();
#endif

	_errors.clear();
	_messages.clear();

	_assemble(this, _assembler->getMemory(), _assembler->getMemorySize(), predefinitionsString);

	if (!_errors.empty())
	{
		return false;
	}

	if (!_executableMemory)
	{
		allocateExecutableMemory(std::max<size_t>(100 * 1024, _bytecode.getSize()));
	}
	else if (_bytecode.getSize() > _executableMemorySize)
	{
		freeExecutableMemory();
		allocateExecutableMemory(_bytecode.getSize());
	}

	const char* begin = _bytecode.getData();
	std::copy(begin, begin + _bytecode.getSize(), reinterpret_cast<char*>(_executableMemory));

	return true;
}

int64_t fasmcpp::Assembly::run(bool preserveRegisters) const
{
	if (!_errors.empty())
	{
		throw std::runtime_error("Error: trying to run code despite unsuccessful assembling.");
	}

	if (!_executableMemory)
	{
		throw std::logic_error("Error: trying to run code before assembling.");
	}

	if (preserveRegisters)
	{
		return _run(_executableMemory);
	}
	else
	{
		return reinterpret_cast<uint64_t(*)()>(_executableMemory)();
	}
}

const char* fasmcpp::Assembly::getErrors() const
{
	return _errors.c_str();
}

const char* fasmcpp::Assembly::getMessages() const
{
	return _messages.c_str();
}

void fasmcpp::Assembly::addError(const char* error, AssemblyKey)
{
	_errors.append(error);
}

void fasmcpp::Assembly::addMessage(const char* message, AssemblyKey)
{
	_messages.append(message);
}

_fasmcpp::AssemblerInput& fasmcpp::Assembly::getInput(AssemblyKey)
{
	return _code;
}

_fasmcpp::AssemblerOutput& fasmcpp::Assembly::getOutput(AssemblyKey)
{
	return _bytecode;
}

void fasmcpp::Assembly::addPredefinition(const char* name, const char* value)
{
	if (strlen(name) > std::numeric_limits<unsigned char>::max())
	{
		throw std::invalid_argument("Error: predefinition name is too long.");
	}
	_predefinitionsChanged = true;
	_predefinitions[name] = value;
}

void fasmcpp::Assembly::addPredefinition(const char* name, const void* value)
{
	addPredefinition(name, std::to_string(reinterpret_cast<uintptr_t>(value)).c_str());
}

void fasmcpp::Assembly::clearPredefinitions()
{
	_predefinitionsChanged = true;
	_predefinitions.clear();
}

const char* fasmcpp::Assembly::getOutput()
{
	return _bytecode.getData();
}

size_t fasmcpp::Assembly::getOutputSize()
{
	return _bytecode.getSize();
}
