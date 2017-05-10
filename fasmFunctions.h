#pragma once

#include <cstdint>

class AssemblyKey {};
namespace fasmcpp { class Assembly; }

#if defined _WIN32 && !defined _WIN64
#	define ASM __stdcall
extern "C" {
#else
#	define ASM
#endif

void ASM fasmError(fasmcpp::Assembly*, const char*);
int ASM fasmOpen(fasmcpp::Assembly*, const char*);
void ASM fasmRead(fasmcpp::Assembly*, uint32_t, char*, uint32_t);
void ASM fasmWrite(fasmcpp::Assembly*, uint32_t, const char*, uint32_t);
void ASM fasmClose(fasmcpp::Assembly*, uint32_t);
uint32_t ASM fasmLseek(fasmcpp::Assembly*, uint32_t, int32_t, uint8_t);
void ASM fasmDisplayBlock(fasmcpp::Assembly*, const char*, uint32_t);

#if defined _WIN32 && !defined _WIN64
}
#endif
