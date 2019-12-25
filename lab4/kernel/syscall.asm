
; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
;                               syscall.asm
; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
;                                                     Forrest Yu, 2005
; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

%include "sconst.inc"

INT_VECTOR_SYS_CALL equ 0x90
_NR_get_ticks       equ 0 ; 要跟 global.c 中 sys_call_table 的定义相对应！

_NR_process_sleep	equ 1
_NR_my_disp_str		equ 2
_NR_p       		equ 3
_NR_v				equ 4

; 导出符号
global	get_ticks

global process_sleep
global my_disp_str
global P
global V

bits 32
[section .text]

; ====================================================================
;                              get_ticks
; ====================================================================
get_ticks:
	mov	eax, _NR_get_ticks
	int	INT_VECTOR_SYS_CALL
	ret

process_sleep:
	mov eax, _NR_process_sleep
	mov edx, [esp+4]
	int INT_VECTOR_SYS_CALL
	ret

my_disp_str:
	mov eax, _NR_my_disp_str
	mov ebx, [esp+4]
	int INT_VECTOR_SYS_CALL
	ret

P:
	mov eax, _NR_p
	mov edx, [esp+4]
	int INT_VECTOR_SYS_CALL
	ret

V:
	mov eax, _NR_v
	mov edx, [esp+4]
	int INT_VECTOR_SYS_CALL
	ret

