.globl _main
.p2align 2
_main:
    .cfi_startproc
    sub sp, sp, #16
    mov x12, sp
    mov w0, #20
    sub sp, sp, #16
    str x0, [sp]
    mov w0, #10
    ldr x1, [sp]
    add sp, sp, #16
    add x0, x1, x0
    sub sp, sp, #16
    str x0, [sp]
    mov w0, #5
    ldr x1, [sp]
    add sp, sp, #16
    add x0, x1, x0
    str w0, [x12, #0]
    mov w0, #97
    str w0, [x12, #8]
    ldr w0, [x12, #0]
    add sp, sp, #16
    ret
    .cfi_endproc

