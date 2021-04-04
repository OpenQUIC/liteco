/*
 * Copyright (c) 2021 Gscienty <gaoxiaochuan@hotmail.com>
 *
 * Distributed under the MIT software license, see the accompanying
 * file LICENSE or https://www.opensource.org/licenses/mit-license.php .
 *
 */

.text
.globl liteco_context_yield
.type liteco_context_yield,@function
.align 2
liteco_context_yield:
    /* @param rdi: n */
    /* void liteco_cas_yield(n) */
    
    movq %rdi, %rax

    cas_yield_loop:
    pause
    subq $1, %rax
    JNZ cas_yield_loop

    ret
.size liteco_context_yield,.-liteco_context_yield
