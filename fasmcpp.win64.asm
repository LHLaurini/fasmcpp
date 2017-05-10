
use64
org WIN64_LOW_MEMORY_FIXED_ADDRESS

; symbols to export
	dq _assemble
	dq _run
	dq _input_filename
	dq _output_filename
	dq _fasm_memory

; symbols to import
	fasmError dq ?
	fasmOpen dq ?
	fasmRead dq ?
	fasmWrite dq ?
	fasmClose dq ?
	fasmLseek dq ?
	fasmDisplayBlock dq ?

; code

_assemble: ; Assembly* assembly, void* memory, uint32_t size, const char* predefinitions
	mov [rsp+8], rcx
	mov [rsp+16], rdx
	mov [rsp+24], r8
	mov [rsp+32], r9

	push rbp
	push rbx
	push rdi
	push rsi

	mov rax, [rsp+4*8+8]
	mov [assembly], rax

	; We split memory just like the Win32 assembler interface: 25% memory, 75% additional memory
	mov eax, [rsp+4*8+16]
	mov [memory_start], eax
	mov ebx, [rsp+4*8+24]
	mov ecx, ebx
	shr ebx, 2
	sub ecx, ebx
	add ebx, eax
	mov [memory_end], ebx
	mov [additional_memory], ebx
	add ebx, ecx
	mov [additional_memory_end], ebx

	mov eax, esp
	and eax, not 0FFFh
	add eax, 1000h-10000h
	mov [stack_limit], eax

	mov eax, [rsp+4*8+32]
	mov [initial_definitions], eax
	mov [input_file], _input_filename
	mov [output_file], _output_filename
	mov [symbols_file], 0
	mov [passes_limit], 100

	mov [rsp_backup], rsp

	call preprocessor
	call parser
	call assembler
	call formatter

exit:
	call show_display_buffer

	mov rsp, [rsp_backup]
	pop rsi
	pop rdi
	pop rbx
	pop rbp

	ret

_run:
	push rbx
	push rbp
	push rdi
	push rsi
	push r12
	push r13
	push r14
	push r15
	mov [rsp_backup], rsp
	call rcx
	mov rsp, [rsp_backup]
	pop r15
	pop r14
	pop r13
	pop r12
	pop rsi
	pop rdi
	pop rbp
	pop rbx
	ret

macro pusha
{
	push rcx
	push rdx
	push r8
	push r9
	push r10
	push r11
}

macro popa
{
	pop r11
	pop r10
	pop r9
	pop r8
	pop rdx
	pop rcx
}

macro fix_stack
{
	mov r12, rsp
	sub rsp, 8*4
	and rsp, not 0xF
}

macro restore_stack
{
	mov rsp, r12
}

fatal_error:
assembler_error:
	mov rcx, [assembly]
	mov edx, [rsp]
	fix_stack
	call [fasmError]
	jmp exit

create: ; in: edx = filename - out: ebx = handle, cf = error
open: ; in: edx = filename - out: ebx = handle, cf = error
	pusha
	fix_stack
	mov rcx, [assembly]
	call [fasmOpen]
	restore_stack
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
	fix_stack
	mov r9d, ecx
	mov r8d, edx
	mov edx, ebx
	mov rcx, [assembly]
	call [fasmRead]
	restore_stack
	popa
	ret

write: ; in: ebx = handle, ecx = size, edx = buffer - out: cf = error
	pusha
	fix_stack
	mov r9d, ecx
	mov r8d, edx
	mov edx, ebx
	mov rcx, [assembly]
	call [fasmWrite]
	restore_stack
	popa
	ret

close: ; in: ebx = handle
	pusha
	fix_stack
	mov rcx, [assembly]
	mov edx, ebx
	call [fasmClose]
	restore_stack
	popa
	ret

lseek: ; in: ebx = handle, edx = offset, al = mode (0: begin, 1: current, 2: end) out: eax
	pusha
	fix_stack
	movzx r9d, al
	mov r8d, edx
	mov edx, ebx
	mov rcx, [assembly]
	call [fasmLseek]
	restore_stack
	popa
	ret

get_environment_variable: ; in: esi = name, edi = buffer, ecx = size
	ret

display_block: ; in: esi = message, ecx = size
	pusha
	fix_stack
	mov r8d, ecx
	mov edx, esi
	mov rcx, [assembly]
	call [fasmDisplayBlock]
	restore_stack
	popa
	ret

make_timestamp:
	jmp code_cannot_be_generated

	initial_defs db 0
	_input_filename db "input", 0
	input_filename_end:
	_output_filename db "output", 0
	output_filename_end:

purge pusha, popa

include 'linux/x64/modes.inc'

include 'version.inc'
include 'errors.inc'
include 'symbdump.inc'
include 'preproce.inc'
include 'parser.inc'
include 'exprpars.inc'
include 'assemble.inc'
include 'exprcalc.inc'
include 'formats.inc'
include 'avx.inc'
include 'x86_64.inc'
include 'tables.inc'
include 'messages.inc'

; data
	assembly dq ?
	rsp_backup dq ?

include 'variable.inc'

_fasm_memory:
