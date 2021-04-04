/*
 * Copyright (c) 2021 Gscienty <gaoxiaochuan@hotmail.com>
 *
 * Distributed under the MIT software license, see the accompanying
 * file LICENSE or https://www.opensource.org/licenses/mit-license.php .
 *
 */

	.section	__TEXT,__text,regular,pure_instructions
	.globl _liteco_cas_yield
	.p2align 4, 0x90
_liteco_cas_yield:
	.cfi_startproc
    /* @param rdi: n */
    /* void liteco_cas_yield(n) */
    
    movq %rdi, %rax

    cas_yield_loop:
    pause
    subq $1, %rax
    JNZ cas_yield_loop

    ret

	.cfi_endproc

.subsections_via_symbols
