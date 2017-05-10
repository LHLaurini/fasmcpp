
#include <chrono>
#include <iostream>
#include "fasmcpp.h"

using namespace std::chrono;
using namespace fasmcpp::size_multipliers;

int main()
{
	fasmcpp::Assembler assembler;

	try
	{
		assembler = 100_KB;
	}
	catch (const std::exception& e)
	{
		std::cout << e.what() << std::endl;
		std::cin.get();
		return 1;
	}

	fasmcpp::Assembly helloWorld;

#if defined _WIN64
	helloWorld = "\
		use64 \n\
		sub rsp, 5*8 \n\
		lea rcx, [string] \n\
		call [_printf] \n\
		add rsp, 5*8 \n\
		ret \n\
		_printf dq printf \n\
		string db 'Hello world (from assembly)!', 13, 10, 0 \n\
	";
#elif defined _WIN32
	helloWorld = "\
		use32 \n\
		call $+5 \n\
		mov eax, [esp] \n\
		add dword[esp], string-5 \n\
		call [eax+_printf-5] \n\
		add esp, 4 \n\
		ret \n\
		_printf dd printf \n\
		string db 'Hello world (from assembly)!', 13, 10, 0 \n\
	";
#endif

	helloWorld.setAssembler(&assembler);
	helloWorld.addPredefinition("printf", printf);

	high_resolution_clock::time_point before = high_resolution_clock::now();
	bool success = helloWorld.assemble();
	high_resolution_clock::time_point after = high_resolution_clock::now();

	if (success)
	{
		std::cout << helloWorld.getMessages();
		std::cout << "Successfully built code in " <<
			duration_cast<duration<double, std::micro>>(after - before).count() <<
			" microseconds.\n";
		helloWorld.run(false);
	}
	else
	{
		std::cout << helloWorld.getErrors() << std::endl;
	}

	std::cin.get();
}
