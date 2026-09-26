.syntax unified
.cpu cortex-m0plus
.thumb

.section .isr_vector,"a",%progbits
.balign 256
.global g_pfnVectors
.type g_pfnVectors,%object
g_pfnVectors:
    .word _estack
    .word Reset_Handler
    .word Default_Handler /* NMI */
    .word Default_Handler /* HardFault */
    .rept 7
    .word 0
    .endr
    .word Default_Handler /* SVC */
    .word 0, 0
    .word Default_Handler /* PendSV */
    .word SysTick_Handler
    /* This application enables no peripheral IRQs. All 30 device IRQ slots
       enter the same fail-safe reset handler if unexpectedly enabled. */
    .rept 30
    .word Default_Handler
    .endr
.size g_pfnVectors, .-g_pfnVectors

.section .text.Reset_Handler,"ax",%progbits
.global Reset_Handler
.type Reset_Handler,%function
.thumb_func
Reset_Handler:
    ldr r0, =_sidata
    ldr r1, =_sdata
    ldr r2, =_edata
1:
    cmp r1, r2
    bcs 2f
    ldr r3, [r0]
    str r3, [r1]
    adds r0, #4
    adds r1, #4
    b 1b
2:
    ldr r1, =_sbss
    ldr r2, =_ebss
    movs r3, #0
3:
    cmp r1, r2
    bcs 4f
    str r3, [r1]
    adds r1, #4
    b 3b
4:
    ldr r0, =0xe000ed08 /* SCB VTOR */
    ldr r1, =g_pfnVectors
    str r1, [r0]
    dsb
    isb
    bl main
    bl Default_Handler
5:  b 5b
.size Reset_Handler, .-Reset_Handler
.section .note.GNU-stack,"",%progbits
