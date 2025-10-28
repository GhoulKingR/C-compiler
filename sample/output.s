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
    mul x0, x1, x0
    sub sp, sp, #16
    str x0, [sp]
    mov w0, #5
    ldr x1, [sp]
    add sp, sp, #16
    add x0, x1, x0
    sub sp, sp, #16
    str x0, [sp]
    mov w0, #10
    ldr x1, [sp]
    add sp, sp, #16
    add x0, x1, x0
    str w0, [x12, #0]
    mov w0, #97
    str w0, [x12, #8]
    ldr w0, [x12, #0]
    sub sp, sp, #16
    str x0, [sp]
    ldr w0, [x12, #8]
    ldr x1, [sp]
    add sp, sp, #16
    cmp x1, #0
    bne L0
    cmp x0, #0
    bne L0
    mov x0, #0
    b L1
L0:
    mov x0, #1
L1:
    add sp, sp, #16
    ret
    .cfi_endproc

