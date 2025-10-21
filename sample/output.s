.globl _main
.p2align 2
_main:
    .cfi_startproc
    sub sp, sp, #16
    mov x12, sp
    mov w0, #2
    str w0, [x12, #2]
    mov w0, #3
    strb w0, [x12, #0]
    ldr w0, [x12, #2]
    add sp, sp, #16
    ret
    .cfi_endproc

