ENTRY(Reset_Handler)
MEMORY
{
    FLASH (rx) : ORIGIN = 0x08000000, LENGTH = 64K
    RAM (xrw) : ORIGIN = 0x20000000, LENGTH = 8K
}
_estack = ORIGIN(RAM) + LENGTH(RAM);
/* Main's app + two frames and its nested calls fit within this reserved stack.
   No heap, constructors, dynamic allocation, RTOS, or flash score writes. */
_Min_Stack_Size = 2048;
SECTIONS
{
    .isr_vector : { . = ALIGN(256); KEEP(*(.isr_vector)) } > FLASH
    .text :
    {
        . = ALIGN(4);
        *(.text*) *(.rodata*)
        KEEP(*(.init)) KEEP(*(.fini))
        . = ALIGN(4);
    } > FLASH
    .ARM.extab : { *(.ARM.extab*) } > FLASH
    .ARM.exidx : { __exidx_start = .; *(.ARM.exidx*) __exidx_end = .; } > FLASH
    _sidata = LOADADDR(.data);
    .data :
    {
        . = ALIGN(4); _sdata = .;
        *(.data*)
        . = ALIGN(4); _edata = .;
    } > RAM AT > FLASH
    .bss (NOLOAD) :
    {
        . = ALIGN(4); _sbss = .;
        *(.bss*) *(COMMON)
        . = ALIGN(4); _ebss = .;
    } > RAM
    . = ALIGN(8);
    _end = .;
    ASSERT(_end + _Min_Stack_Size <= _estack, "RAM overflow into reserved stack")
    /DISCARD/ : { *(.comment) *(.note*) }
}
