/*
 * Copyright (c) 2021 Gscienty <gaoxiaochuan@hotmail.com>
 *
 * Distributed under the MIT software license, see the accompanying
 * file LICENSE or https://www.opensource.org/licenses/mit-license.php .
 *
 */

.text
.globl liteco_context_init
.type liteco_context_init,@function
.align 2
liteco_context_init:
    /* @param rdi: ctx */
    /* @param rsi: stack */
    /* @param rdx: size */
    /* @param rcx: func_addr */
    /* @param r8: arg */
    /* void liteco_context_init(ctx, stack, size, func_addr, arg) */

    movq %rcx, 0210(%rdi); /* func_addr */

    movq %rsi, %rax; /* %rax = stack */
    addq %rdx, %rax; /* %rax += size */ 

    subq $010, %rax; /* align */
    andq $0xfffffffffffffff0, %rax;

    movq %rax, 0140(%rdi); /* context[RBX] = base_addr */
    subq $010, %rax;
    movq %rax, 0200(%rdi); /* context[RSP] = calced_stack_top */

    movq %r8, 0110(%rdi); /* context[RDI] = arg */

    ret
.size liteco_context_init,.-liteco_context_init
