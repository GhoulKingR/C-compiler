.globl _main
.p2align 2
_main:
    .cfi_startproc
    mov x12, sp
    sub sp, sp, #32
    mov x0, #20
    sub sp, sp, #16
    str x0, [sp]
    mov x0, #10
    ldr x1, [sp]
    add sp, sp, #16
    mul x0, x1, x0
    sub sp, sp, #16
    str x0, [sp]
    mov x0, #5
    ldr x1, [sp]
    add sp, sp, #16
    add x0, x1, x0
    sub sp, sp, #16
    str x0, [sp]
    mov x0, #10
    ldr x1, [sp]
    add sp, sp, #16
    add x0, x1, x0
    sub x12, x12, #8
    str w0, [x12, #0]
    mov x0, #97
    sub x12, x12, #8
    str w0, [x12, #0]
    ldr w0, [x12, #8]
    sub sp, sp, #16
    str x0, [sp]
    ldr w0, [x12, #0]
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
    sub x12, x12, #8
    str w0, [x12, #0]
    ldr w0, [x12, #16]
    sub x0, x0, #1
    str w0, [x12, #16]
    add sp, sp, #32
    ret
    .cfi_endproc

