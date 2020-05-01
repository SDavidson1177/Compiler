; initialize_heap:
; Allocates memory for the entire heap. The heap will be a fixed size
; determined by the constant 'heapsize'. The binary buddy system
; will be used to manage heap memory
; Registers used:
; RAX - Return the address of where the heap begins. Since the binary
;	    buddy system is used, the value at this address is the total
;       size of the heap in bytes.
initialize_heap:
	; Store used registers
	sub rsp, 8
	mov [rsp], rdi
	sub rsp, 8
	mov [rsp], r11

	; error checks
	mov rdi, heapsize
	cmp rdi, 0
	jg initialize_heap_good_size
	mov rax, 60
	mov rdi, -1
	syscall ; exit with exit code -1
	initialize_heap_good_size:

	; get current end of data segment
	mov rax, 12
	mov rdi, 0
	syscall

	; store the starting address of the heap
	sub rsp, 8
	mov [rsp], rax

	; change end of data segment to allocate
	; memory for the heap. return value in rax
	add rax, heapsize
	mov rdi, rax
	mov rax, 12
	syscall

	; additional error checks
	mov rdi, [rsp]
	cmp rax, rdi
	jne initialize_heap_good_alloc
	mov rax, 60
	mov rdi, -1
	syscall ; exit with exit code -1
	initialize_heap_good_alloc:

	mov eax, heapsize
	mov [rdi], eax
	mov rax, rdi

	add rsp, 8

	; restore register
	mov r11, [rsp]
	add rsp, 8
	mov rdi, [rsp]
	add rsp, 8
	ret

; end of initialize_heap ----------------------------

; malloc:
; allocates a number of bytes in the heap for use.
; Register use:
; RDI - Number of bytes to allocate in the heap.
; RCX - base address of the location in the heap we are
;	    looking at.
; RAX - Returns the address of the allocated memory. if
;		memory could not be allocated, zero is returned.
; Requires:
; R11 - Holds the 
malloc:
	; save used registers
	sub rsp, 8
	mov [rsp], rcx
	sub rsp, 8
	mov [rsp], r8
	sub rsp, 8
	mov [rsp], r9
	sub rsp, 8
	mov [rsp], rdx

	mov r8, 0
	mov r8b, 2 ; store 2 in r9


	; get base pointer
	mov rcx, r11

malloc_main_loop:
	mov eax, [rcx] ; get size of memory segment
	
	; check if this slot is taken
	mov edx, 0
	div r8d
	cmp edx, 0
	je malloc_check_size ; check if it fits

	; we need to find a new index to check
	mul r8 ; get back proper size

malloc_new_slot:
	add rcx, rax

	; if we are out of space, exit
	mov rax, r11
	add rax, heapsize
	cmp rax, rcx
	jg malloc_main_loop

	; exit on error
	mov rax, 0
	jmp malloc_return
malloc_check_size:
	mov eax, [rcx] ; retrieve the proper size of this segment
	sub eax, 4 ; bytes needed to store size
	cmp eax, edi
	jge malloc_found_slot ; we found a good spot we can use
	add eax, 4

	; This spot is too small. Look for another one
	jmp malloc_new_slot

malloc_found_slot:
	add eax, 4
	mov edx, 0
	div r8d
	sub eax, 4
	cmp eax, edi
	jl malloc_finish ; this slot is fine. reserve it

	; the slot is too large, make partition it in half
	add eax, 4 ; new size
	mov [rcx], eax
	mov r9, rcx
	add r9, rax
	mov [r9], eax
	sub eax, 4
	jmp malloc_found_slot

malloc_finish:
	add eax, 4
	mul r8
	add eax, 1
	mov [rcx], eax
	mov rax, rcx
	add rax, 4 ; skip over first 4 bytes for memory size

malloc_return:
	; Reload registers
	mov rdx, [rsp]
	add rsp, 8
	mov r9, [rsp]
	add rsp, 8
	mov r8, [rsp]
	add rsp, 8
	mov rcx, [rsp]
	add rsp, 8

	ret 

; end of malloc --------------------

; free
; Free reclaims allocated memory to the heap
; Registers used:
; RDI - Memory address to reclaim memory for
; RAX - Returns 0 if memory could be freed. returns
;		-1 otherwise.

free:
	; save used registers
	sub rsp, 8
	mov [rsp], rcx
	sub rsp, 8
	mov [rsp], r8
	sub rsp, 8
	mov [rsp], r9
	sub rsp, 8
	mov [rsp], rdx
	sub rsp, 8
	mov [rsp], r10

	mov r8, 0
	mov r8b, 2 ; store 2 in r9
	sub rdi, 4 ; move back to get address of first 4 bytes

	; Check if this memory address exists within the heap
	cmp rdi, r11
	jl free_return
	mov rdx, r11
	add rdx, heapsize
	cmp rdi, rdx
	jge free_return

	; check if that memory was in use. if not
	; don't do anything
	mov eax, [rdi]
	mov edx, 0
	div r8d
	cmp edx, 0
	je free_return ; We don't have memory to free here

	mul r8 ; get back the size of the block
	mov [rdi], eax ; set block to be available

free_consolidate:
	; Now we try to consolidate binary buddies
	mov rcx, rdi

	; first check upper neighbor
	add rcx, rax

	; check if we are out of bounds
	mov r10, r11
	add r10, heapsize
	cmp rcx, r10
	jge free_check_lower ; we are out of bounds

	mov edx, [rcx]
	mov r10d, [rdi]
	cmp r10d, edx
	jne free_check_lower

	; upper bound is correct size. Check if they are buddies
	mul r8
	mov r10d, eax ; r10 holds divisor

	mov rax, 0
	mov rax, rcx
	sub rax, r11
	mov edx, 0
	div r10d
	mov r9d, eax ; first modulus

	mov rax, 0
	mov rax, rdi
	sub rax, r11
	mov edx, 0
	div r10d ; second modulus in eax

	cmp eax, r9d ; check if they are buddies
	jne free_check_lower_prep ; check lower address

	; they are buddies. consolidate
	mov [rdi], r10d ; set the size
	mov eax, r10d ; reset the size of eax
	jmp free_consolidate ; consolidate again

free_check_lower_prep:

	mov eax, r10d
	mov edx, 0
	div r8d
	mov r10d, eax
free_check_lower:
	mov rcx, rdi
	mov eax, r10d
	sub rcx, rax ; Get the proper offsets in rcx

	; Check bounds
	cmp rcx, r11
	jl free_return ; we are out of bounds

	; Check if these are the same size
	mov edx, [rcx]
	mov r10d, [rdi]
	cmp r10d, edx
	jne free_return ; We cannot conslidate. Return

	; Check if they are buddies
	mul r8
	mov r10d, eax ; r10 holds divisor

	mov rax, 0
	mov rax, rcx
	sub rax, r11
	mov edx, 0
	div r10d
	mov r9d, eax ; first modulus

	mov rax, 0
	mov rax, rdi
	sub rax, r11
	mov edx, 0
	div r10d ; second modulus in eax

	cmp eax, r9d ; check if they are buddies
	jne free_return ; They are not buddies. Do not consolidate

	; they are buddies. consolidate
	mov [rcx], r10d ; set the size
	mov rdi, rcx
	mov eax, r10d ; reset the size of eax
	jmp free_consolidate ; consolidate again

free_return:
	; Reload registers
	mov r10, [rsp]
	add rsp, 8
	mov rdx, [rsp]
	add rsp, 8
	mov r9, [rsp]
	add rsp, 8
	mov r8, [rsp]
	add rsp, 8
	mov rcx, [rsp]
	add rsp, 8

	ret

; end of free ----------------------
