
format MS COFF

a=1
public a as "@feat.00"

section '.text' code readable executable

	public assemble as "__assemble@16"
	public run as "__run@4"
	public input_filename as "__input_filename"
	public output_filename as "__output_filename"
	
	extrn '_fasmError@8' as fasmError:DWORD
	extrn '_fasmOpen@8' as fasmOpen:DWORD
	extrn '_fasmRead@16' as fasmRead:DWORD
	extrn '_fasmWrite@16' as fasmWrite:DWORD
	extrn '_fasmClose@8' as fasmClose:DWORD
	extrn '_fasmLseek@16' as fasmLseek:DWORD
	extrn '_fasmDisplayBlock@12' as fasmDisplayBlock:DWORD

assemble: ; Assembly* assembly, void* memory, uint32_t size, const char* predefinitions
	mov eax, [esp+4]
	mov [assembly], eax
	mov eax, [esp+8]
	mov [memory_start], eax
	mov ecx, [esp+12]
	mov eax, [esp+16]
	mov [initial_definitions], eax

	push ebp
	push ebx
	push edi
	push esi

	; We split memory just like the Win32 assembler interface: 25% memory, 75% additional memory
	mov ebx, ecx
	shr ebx, 2
	sub ecx, ebx
	add ebx, [memory_start]
	mov [memory_end], ebx
	mov [additional_memory], ebx
	add ebx, ecx
	mov [additional_memory_end], ebx

	mov eax, esp
	and eax, not 0FFFh
	add eax, 1000h-10000h
	mov [stack_limit], eax

	mov [input_file], input_filename
	mov [output_file], output_filename
	mov [symbols_file], 0
	mov [passes_limit], 100

	mov [esp_backup], esp

	call preprocessor
	call parser
	call assembler
	call formatter

exit:
	call show_display_buffer
	mov esp, [esp_backup]

	pop esi
	pop edi
	pop ebx
	pop ebp

	ret 16

run:
	mov eax, [esp + 4]
	push ebp
	push ebx
	push edi
	push esi
	mov [esp_backup], esp
	call eax
	mov esp, [esp_backup]
	pop esi
	pop edi
	pop ebx
	pop ebp
	ret 4

macro pusha
{
	push ecx
	push edx
}

macro popa
{
	pop edx
	pop ecx
}

;void fasmError(const char* error);
;int fasmOpen(const char* filename);
;void fasmRead(uint32_t handle, char* buffer, uint32_t size);
;void fasmWrite(uint32_t handle, const char* buffer, uint32_t size);
;void fasmClose(uint32_t handle);
;uint32_t fasmLseek(uint32_t handle, int32_t offset, uint8_t mode);
;void fasmDisplayBlock(const char* message, uint32_t size);

fatal_error:
assembler_error:
	push [assembly]
	call fasmError
	jmp exit

create: ; in: edx = filename - out: ebx = handle, cf = error
open: ; in: edx = filename - out: ebx = handle, cf = error
	pusha
	push edx
	push [assembly]
	call fasmOpen
	popa
	or eax, eax
	jz open_error
	mov ebx, eax
	clc
	ret
  open_error:
	stc
	ret

read: ; in: ebx = handle, ecx = size, edx = buffer - out: cf = error
	pusha
	push ecx
	push edx
	push ebx
	push [assembly]
	call fasmRead
	popa
	ret

write: ; in: ebx = handle, ecx = size, edx = buffer - out: cf = error
	pusha
	push ecx
	push edx
	push ebx
	push [assembly]
	call fasmWrite
	popa
	ret

close: ; in: ebx = handle
	pusha
	push ebx
	push [assembly]
	call fasmClose
	popa
	ret

lseek: ; in: ebx = handle, edx = offset, al = mode (0: begin, 1: current, 2: end) out: eax
	pusha
	push eax
	push edx
	push ebx
	push [assembly]
	call fasmLseek
	popa
	ret

get_environment_variable: ; in: esi = name, edi = buffer, ecx = size
	ret

display_block: ; in: esi = message, ecx = size
	pusha
	push ecx
	push esi
	push [assembly]
	call fasmDisplayBlock
	popa
	ret

make_timestamp:
	jmp code_cannot_be_generated

	input_filename db "input", 0
	input_filename_end:
	output_filename db "output", 0
	output_filename_end:

purge pusha
purge popa

include '../source/version.inc'
include '../source/errors.inc'
include '../source/symbdump.inc'
include '../source/preproce.inc'
include '../source/parser.inc'
include '../source/exprpars.inc'
include '../source/assemble.inc'
include '../source/exprcalc.inc'
include '../source/formats.inc'
include '../source/avx.inc'
include '../source/x86_64.inc'
include '../source/tables.inc'
include '../source/messages.inc'

section '.data' data readable writeable

	assembly dd 0
	esp_backup dd 0

include '../source/variable.inc'
