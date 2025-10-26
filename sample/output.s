.globl _main
.p2align 2
_main:
    .cfi_startproc
    sub sp, sp, #16
    mov x12, sp
    mov w0, #200
    str w0, [x12, #0]
    mov w0, #97
    str w0, [x12, #8]
    ldr w0, [x12, #0]
    cmp w0, #0
    cset w0, eq
    add sp, sp, #16
    ret
    .cfi_endproc

