/*
 * Copyright (c) 2021 Gscienty <gaoxiaochuan@hotmail.com>
 *
 * Distributed under the MIT software license, see the accompanying
 * file LICENSE or https://www.opensource.org/licenses/mit-license.php .
 *
 */

.text
.globl liteco_cas
.type liteco_cas,@function
.align 2
liteco_cas:
    /* @param rdi: ptr */
    /* @param rsi: exp */
    /* @param rdx: nex */
    /* void liteco_cas(*ptr, exp, nex) */

    movq 0(%rdi), %rbx 
    movq %rsi, %rax /* exp */
    movq %rdx, %rcx /* nex */

    lock
    cmpxchg %cx, 0(%rdi)

    je seteq
    xorq %rax, %rax
    ret

seteq:
    movq $1, %rax
    ret
.size liteco_cas,.-liteco_cas
