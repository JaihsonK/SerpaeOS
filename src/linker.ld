ENTRY(k_start)
OUTPUT_FORMAT(binary)
SECTIONS
{
    . = 0x100000;
    .text : ALIGN(4096)
    {
        *(.text)
    }

    .asm : ALIGN(4096)
    {
        *(.asm)
    }

    .rodata : ALIGN(4096)
    {
        *(.rodata)
    }

    .data : ALIGN(4096)
    {
        *(.data)
    }

    .bss : ALIGN(4096)
    {
        *(COMMON)
        *(.bss)
    }
    .config_data : ALIGN(4096)
    {
        *(.config_data)
    }
}