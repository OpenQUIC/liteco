	.section	__TEXT,__text,regular,pure_instructions
	.globl _liteco_cas
	.p2align 4, 0x90
_liteco_cas:
	.cfi_startproc
    /* @param rdi: ptr */
    /* @param rsi: old */
    /* @param rdx: nex */
    /* void liteco_cas(*ptr, old, next) */

    mov %rdi, %rbx
    movq %rsi, %rax
    movq %rdx, %rcx

    lock
    cmpxchg %cx, 0(%rbx)

    cmp 0(%rbx), %cx

    je seteq
    xorq %rax, %rax
    ret

seteq:
    movq $1, %rax
    ret

	.cfi_endproc

.subsections_via_symbols
