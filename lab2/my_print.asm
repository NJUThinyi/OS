global nasm_print

section .text
	;nasm_print(char * p, int length)
	
nasm_print:
	mov edx, [esp+8]	;获得length
	mov ecx, [esp+4]	;获得字符串指针p
	mov ebx, 1
	mov eax, 4
	int 80h
	ret
