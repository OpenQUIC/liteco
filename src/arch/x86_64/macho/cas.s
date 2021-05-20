/*
 * Copyright (c) 2021 Gscienty <gaoxiaochuan@hotmail.com>
 *
 * Distributed under the MIT software license, see the accompanying
 * file LICENSE or https://www.opensource.org/licenses/mit-license.php .
 *
 */

	.section	__TEXT,__text,regular,pure_instructions
	.globl _liteco_cas
	.p2align 4, 0x90
_liteco_cas:
	.cfi_startproc
    /* @param rdi: ptr */
    /* @param rsi: exp */
    /* @param rdx: nex */
    /* void liteco_cas(*ptr, exp, nex) */

    mov 0(%rdi), %rbx
    movq %rsi, %rax     ; /* exp */
    movq %rdx, %rcx     ; /* nex */

    lock
    cmpxchg %cx, 0(%rdi)

    je seteq
    xorq %rax, %rax
    ret

seteq:
    movq $1, %rax
    ret

	.cfi_endproc

.subsections_via_symbols
