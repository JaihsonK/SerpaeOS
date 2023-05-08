[BITS 32]

section .asm

global print:function
; void print(const char* filename)
print:
    push ebp
    mov ebp, esp
    push dword[ebp+8]
    mov eax, 1 ; Command print
    int 0x80
    add esp, 4
    
    pop ebp
    ret

global getkey:function
getkey:
    push ebp
    mov ebp, esp

    mov eax, 2
    int 0x80
    
    pop ebp
    ret

global sos_putchar:function
sos_putchar:
    push ebp
    mov ebp, esp

    mov eax, 3
    push dword[ebp + 8]
    int 0x80
    add esp, 4

    pop ebp
    ret

global sos_malloc:function
sos_malloc:
    push ebp
    mov ebp, esp
    mov eax, 4 ; Command malloc (Allocates memory for the process)
    push dword[ebp+8] ; Variable "size"
    int 0x80
    add esp, 4
    pop ebp
    ret

global sos_free:function
sos_free:
    push ebp 
    mov ebp, esp
    
    mov eax, 5
    push dword[ebp + 8]
    int 80h
    add esp, 4

    pop ebp
    ret

global sos_process_load:function
sos_process_load:
    push ebp
    mov ebp, esp

    mov eax, 6
    push dword[ebp + 8]
    int 0x80
    add esp, 4

    pop ebp
    ret

global sos_system:function
sos_system:
    push ebp
    mov ebp, esp

    mov eax, 7
    push dword[ebp + 8]
    int 80h
    add esp, 4

    pop ebp
    ret

global sos_process_get_arguments:function
sos_process_get_arguments:
    push ebp
    mov ebp, esp
    mov eax, 8
    push dword[ebp + 8]
    int 80h
    add esp, 4
    pop ebp
    ret

global sos_exit:function
sos_exit:
    push ebp 
    mov ebp, esp

    push eax ;save return code
    mov eax, 0
    int 80h ;terminate
    add esp, 4
    pop ebp

    jmp $

global clrscr:function
clrscr:
    push ebp
    mov ebp, esp
    mov eax, 9
    push dword[ebp + 8]
    int 0x80
    add esp, 4
    pop ebp
    ret

global fopen:function
fopen:
    push ebp
    mov ebp, esp

    mov eax, 10
    push dword[ebp + 8]
    push dword[ebp + 12]
    int 80h
    add esp, 8

    pop ebp
    ret

global fclose:function
fclose:
    push ebp
    mov ebp, esp

    mov eax, 11
    push dword[ebp + 8]
    int 80h
    add esp, 4

    pop ebp
    ret

global fread:function
fread:
    push ebp
    mov ebp, esp

    mov eax, 12
    push dword[ebp + 16]
    push dword[ebp + 12]
    push dword[ebp + 8]
    int 80h
    add esp, 12

    pop ebp
    ret

global fwrite:function
fwrite:
    push ebp
    mov ebp, esp

    mov eax, 13
    push dword[ebp + 16]
    push dword[ebp + 12]
    push dword[ebp + 8]
    int 80h
    add esp, 12

    pop ebp
    ret

global kill:function
kill:
    push ebp
    mov ebp, esp

    mov eax, 14
    push dword[ebp + 8]
    int 80h
    add esp, 4

    pop ebp
    ret

global sos_retcode:function
sos_retcode:
    push ebp
    mov ebp, esp

    mov eax, 255
    push dword[ebp + 8]
    int 80h
    add esp, 4

    pop ebp
    ret
global sos_processinf:function
sos_processinf:
    push ebp
    mov ebp, esp

    mov eax, 15
    push dword[ebp + 8]
    int 80h
    add esp, 4

    pop ebp
    ret

global sos_newthread:function
sos_newthread:
    push ebp
    mov ebp, esp

    mov eax, 16

    push dword[ebp + 16]
    push dword[ebp + 12]
    push dword[ebp + 8]

    int 80h
    add esp, 12

    pop ebp
    ret

global sos_writesector:function
sos_writesector:
    push ebp
    mov ebp, esp

    mov eax, 18
    push dword[ebp + 16]
    push dword[ebp + 12]
    push dword[ebp + 8]
    int 80h
    add esp, 12

    pop ebp
    ret

global sos_readsector:function
sos_readsector:
    push ebp
    mov ebp, esp

    mov eax, 19
    push dword[ebp + 16]
    push dword[ebp + 12]
    push dword[ebp + 8]
    int 80h
    add esp, 12

    pop ebp
    ret

global fstat:function
fstat:
    push ebp
    mov ebp, esp

    mov eax, 20
    push dword [ebp + 12]
    push dword [ebp + 8]
    int 80h
    add esp, 8

    pop ebp
    ret

global ftell:function
ftell:
    push ebp
    mov ebp, esp

    mov eax, 21
    push dword [ebp + 8]
    int 80h
    add esp, 4

    pop ebp
    ret   

global fseek:function
fseek:
    push ebp
    mov ebp, esp

    mov eax, 22
    push dword [ebp + 16]
    push dword [ebp + 12]
    push dword [ebp + 8]
    int 80h
    add esp, 12

    pop ebp
    ret   

global remove:function
remove:
    push ebp
    mov ebp, esp

    mov eax, 23
    push dword [ebp + 8]
    int 80h
    add esp, 4

    pop ebp
    ret

global set_cursor:function
set_cursor:
    push ebp
    mov ebp, esp

    mov eax, 24
    push dword [ebp + 8]
    push dword [ebp + 12]
    int 80h
    add esp, 8

    pop ebp
    ret

global draw_block:function
draw_block:
    push ebp
    mov ebp, esp

    mov eax, 25
    push dword [ebp + 12]
    push dword [ebp + 16]
    push dword [ebp + 8]
    int 80h
    add esp, 12

    pop ebp
    ret

global play:function
play:
    push ebp
    mov ebp, esp

    mov eax, 26
    push dword[ebp + 8]
    int 80h
    add esp, 4

    pop ebp
    ret

global quit_sound:function
quit_sound:
    push ebp
    mov ebp, esp

    mov eax, 27
    int 80h

    pop ebp
    ret

global fork:function
fork:
    push ebp
    mov ebp, esp

    mov eax, 28
    int 80h

    pop ebp
    ret