.weak liteco_cas_yield;
liteco_cas_yield = __cas_yield;

.global __cas_yield;
.align 2;
.type __cas_yield, @function;
__cas_yield:
    /* @param rdi: n */
    /* void liteco_cas_yield(n) */
    
    movq %rdi, %rax

cas_yield_loop:
    pause
    subq $1, %rax
    JNZ cas_yield_loop

    ret

.end __cas_yield;
.size __cas_yield,.-__cas_yield;

