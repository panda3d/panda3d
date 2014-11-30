; Syntax Highlighting Test File for NASM
; Some Comments about this file
; Hello World in NASM

section .text

_start:
    push    dword len
    push    dword msg
    push    dword 1
    mov     eax, 0x4
    call    _syscall
    add     esp, 12

    push    dword 0
    mov     eax, 0x1
    call    _syscall

_syscall:
    int     0x80
    ret

msg db      "Hello World",0xa
len equ     $ - msg
