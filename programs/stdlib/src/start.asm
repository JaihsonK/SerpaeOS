[BITS 32]

global _start
extern c_start
extern sos_exit

section .asm

_start:
    call c_start
    call sos_exit
    ret