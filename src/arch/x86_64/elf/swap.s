/*
 * Copyright (c) 2021 Gscienty <gaoxiaochuan@hotmail.com>
 *
 * Distributed under the MIT software license, see the accompanying
 * file LICENSE or https://www.opensource.org/licenses/mit-license.php .
 *
 */

.text
.globl liteco_context_swap
.type liteco_context_swap,@function
.align 2
liteco_context_swap:
    /* @param rdi: from */
    /* @param rsi: to */
    /* void liteco_context_swap(from, to) */

    /* store */
    movq %r8,  0010(%rdi); /* R8 */
    movq %r9,  0020(%rdi); /* R9 */
    movq %r10, 0030(%rdi); /* R10 */
    movq %r11, 0040(%rdi); /* R11 */
    movq %r12, 0050(%rdi); /* R12 */
    movq %r13, 0060(%rdi); /* R13 */
    movq %r14, 0070(%rdi); /* R14 */
    movq %r15, 0100(%rdi); /* R15 */
    movq %rdi, 0110(%rdi); /* RDI */
    movq %rsi, 0120(%rdi); /* RSI */
    movq %rbp, 0130(%rdi); /* RBP */
    movq %rbx, 0140(%rdi); /* RBX */
    movq %rdx, 0150(%rdi); /* RDX */
    movq $0,   0160(%rdi); /* RAX */
    movq %rcx, 0170(%rdi); /* RCX */

    movq (%rsp), %rcx
    movq %rcx, 0210(%rdi); /* func_addr */

    leaq 0010(%rsp), %rcx
    movq %rcx, 0200(%rdi); /* RSP */


    /* restore */
    movq 0010(%rsi), %r8; /* R8 */
    movq 0020(%rsi), %r9; /* R9 */
    movq 0030(%rsi), %r10; /* R10 */
    movq 0040(%rsi), %r11; /* R11 */
    movq 0050(%rsi), %r12; /* R12 */
    movq 0060(%rsi), %r13; /* R13 */
    movq 0070(%rsi), %r14; /* R14 */
    movq 0100(%rsi), %r15; /* R15 */
    movq 0110(%rsi), %rdi; /* RDI */
    movq 0130(%rsi), %rbp; /* RBP */
    movq 0140(%rsi), %rbx; /* RBX */
    movq 0150(%rsi), %rdx; /* RDX */
    movq 0160(%rsi), %rax; /* RAX */
    movq 0170(%rsi), %rax; /* RCX */
    movq 0200(%rsi), %rsp; /* RSP */

    pushq 0210(%rsi); /* func_addr */

    movq 0120(%rsi), %rsi; /* RSI */

    ret
.size liteco_context_swap,.-liteco_context_swap
