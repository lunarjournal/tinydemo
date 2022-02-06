.intel_syntax noprefix
.text
    .globl _start, syscall
    _start:
        xor rbp,rbp   /* xoring a value with itself = 0 */
        pop rdi       /* rdi = argc */
        	          /* the pop instruction already added 8 to rsp */
        mov rsi,rsp   /* rest of the stack as an array of char ptr */
        and rsp,-16
        call main   
	    mov rdi,rax   /* syscall param 1 = rax (ret value of main) */
        mov rax,60    /* SYS_exit */
        syscall
	    ret

    syscall:
        mov rax,rdi
        mov rdi,rsi
        mov rsi,rdx
        mov rdx,rcx
        mov r10,r8
        mov r8,r9
        mov r9,0
        syscall
        ret
    .global mod_start

    .section .rodata
    mod_start:
    .incbin "B.gz"
    mod_end:
    .global mod_size
    mod_size:
    .int    mod_end - mod_start
