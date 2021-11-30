section .data
message: db "123123Hello World!",0
message1: db "Hello World!",0
buffer: times 12 db 0x0 
global _start
section .text
%include "lib.inc"
_start:
    mov rdi,buffer
    mov rsi,2
    call read_word
    mov rdi,rax
    call print_string
    .exit:
      call exit
