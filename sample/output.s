.globl _main
.p2align 2
_main:
    .cfi_startproc
    sub sp, sp, #16
    mov x12, sp
    mov w0, #2
    neg w0, w0
    str w0, [x12, #0]
    mov w0, #97
    str w0, [x12, #8]
    mov w0, [x12, #0]
    add sp, sp, #16
    ret
    .cfi_endproc

