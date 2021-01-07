.weak liteco_cas;
liteco_cas = __cas;

.global __cas;
.align 2;
.type __cas, @function;
__cas:
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

.end __cas;
.size __cas,.-__cas;
