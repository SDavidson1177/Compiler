; express
; Takes as input an integer in register RDI. It will
; print the integer representation to the console
; using the sys_write system call.
; Registers used:
; RDI - Takes as input an integer
express:
	; Store registers
	sub rsp, 8
	mov [rsp], rcx
	sub rsp, 8
	mov [rsp], r8
	sub rsp, 8
	mov [rsp], r9
	sub rsp, 8
	mov [rsp], rbx
	sub rsp, 8
	mov [rsp], r11

	mov rcx, 4 ; 1 + 3 for newline, carriage return and null terminator
	mov r8, 10
	mov rax, rdi

	; count the number of digits in our number
    mov edx, 0
express_digit_count:
	div r8d
	cmp eax, 0
	je express_digit_count_exit
	inc rcx
	mov edx, 0
	jmp express_digit_count

express_digit_count_exit:

	; set r9 as base pointer to start at rsp
	mov r9, rsp
	
	; make enough room on the stack for each digit
	sub rsp, rcx

	; add null terminator, carriage return and newline
	; in reverse.
	sub r9, 1
	mov [r9], byte 0x0
	sub r9, 1
	mov [r9], byte 0xD
	sub r9, 1
	mov [r9], byte 0xA

	; add the rest of the digits in reverse order
	mov rax, rdi
	mov edx, 0
express_digit_place:
	div r8d
	cmp eax, 0
	je express_digit_place_exit
	sub r9, 1
	add dl, '0'
	mov [r9], dl
	mov edx, 0
	jmp express_digit_place

express_digit_place_exit:
	sub r9, 1
	add dl, '0'
	mov [r9], dl

	; fix back the stack pointer
	add rsp, rcx

	; write using sys_write system call
	mov rdi, 1 ; stdout
	mov rsi, r9 ; string buffer
	mov rdx, rcx ; string length
	mov rax, sys_write
	syscall

express_return:
	; Restore registers
	mov r11, [rsp]
	add rsp, 8
	mov rbx, [rsp]
	add rsp, 8
	mov r9, [rsp]
	add rsp, 8
	mov r8, [rsp]
	add rsp, 8
	mov rcx, [rsp]
	add rsp, 8

	ret
; end of express ------------------

; ascii_to_int
; Takes as input a pointer to a string
; Registers used:
; RDI - Pointer to the string
; EAX - Returns the integer value
ascii_to_int:
	sub rsp, 8
	mov [rsp], rcx
	sub rsp, 8
	mov [rsp], rdx

	mov eax, 0
	mov edx, 0
	mov cl, 10
ascii_to_int_loop:
	mov dl, byte [rdi]
	cmp edx, 0
	je ascii_to_int_end
	sub dl, '0'
	mul cl
	add rax, rdx
	add rdi, 1
	cmp eax, eax
	je ascii_to_int_loop

	ascii_to_int_end:
	mov rdx, [rsp]
	add rsp, 8
	mov rcx, [rsp]
	add rsp, 8
	ret
; end of ascii_to_int ------------