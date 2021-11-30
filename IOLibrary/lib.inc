section .data

section .text
 
; Принимает код возврата и завершает текущий процесс
exit: 
    xor rax, rax,
    mov rax, 60           ; use exit system call to shut down correctly
    xor rdi, rdi
    syscall
    ret 

; Принимает указатель на нуль-терминированную строку, возвращает её длину
string_length:
    xor rax, rax
    .loop:
      cmp byte [rdi+rax], 0
      je .end 
      inc rax
      jmp .loop 
      ret
    .end:
      ret

; Принимает указатель на нуль-терминированную строку, выводит её в stdout
print_string:
    xor rax, rax
    cmp rdi,0x0
    je .print_zero
    push rdi
    call string_length
    pop rsi
    mov rdx, rax 
    mov rax, 1
    mov rdi, 1 
    syscall
    ret
    .print_zero:
      mov rdi,0x30
      push rdi
      mov rsi,rsp
      mov rdx,1
      mov rax,1
      mov rdi,1
      syscall
      pop rdi
      ret


; Принимает код символа и выводит его в stdout
print_char:
    push rdi
    mov rax,1
    mov rsi,rsp
    mov rdx,1
    mov rdi,1
    syscall
    pop rdi
    ret

; Переводит строку (выводит символ с кодом 0xA)
print_newline:
    xor rax, rax
    mov rdi, 0xA
    call print_char
    ret

; Выводит беззнаковое 8-байтовое число в десятичном формате 
; Совет: выделите место в стеке и храните там результаты деления
; Не забудьте перевести цифры в их ASCII коды.
print_uint:
    xor rax, rax
    xor r9,r9
    xor r10,r10
    mov r8,10
    mov rax , rdi
    .loop:
      xor rdx,rdx
      div r8
      add rdx,0x30
      push rdx
      inc r9
      cmp rax,0x0
      je .print
      jmp .loop
      .print:
        pop rdi
        call print_char
        inc r10
        cmp r10,r9
        jne .print
    .end:    
      ret

; Выводит знаковое 8-байтовое число в десятичном формате 
print_int:
    xor rax, rax
    cmp rdi,0
    jge .print
    neg rdi
    push rdi
    mov rdi,0x2D
    call print_char
    pop rdi
    .print:
      call print_uint
    ret

; Принимает два указателя на нуль-терминированные строки, возвращает 1 если они равны, 0 иначе
string_equals:
    xor rax, rax
    push rdi
    push rsi
    call string_length
    mov r8,rax
    mov rdi,rsi
    call string_length
    pop rdi
    pop rsi
    cmp r8,rax
    jne .notEquals
    .loop:
      cmp byte [rsi],0
      je .equals
      mov r8b ,byte [rsi]
      cmp byte [rdi],r8b
      jne .notEquals
      inc rsi
      inc rdi
      jmp .loop
    .notEquals:
      mov rax,0
      ret
    .equals:
      mov rax,1
      ret


; Читает один символ из stdin и возвращает его. Возвращает 0 если достигнут конец потока
read_char:
    push 0
    mov rax,0
    mov rsi,rsp
    mov rdx,1
    mov rdi,0
    syscall
    pop rax
    cmp rax,0x0
    jz .print_zero
    ret 
    .print_zero:
        mov rax,0
        ret

; Принимает: адрес начала буфера, размер буфера
; Читает в буфер слово из stdin, пропуская пробельные символы в начале, .
; Пробельные символы это пробел 0x20, табуляция 0x9 и перевод строки 0xA.
; Останавливается и возвращает 0 если слово слишком большое для буфера
; При успехе возвращает адрес буфера в rax, длину слова в rdx.
; При неудаче возвращает 0 в rax
; Эта функция должна дописывать к слову нуль-терминатор

read_word:
    mov r10,rdi
    xor r9,r9
    mov rcx,rdi
    mov r8,rsi
        .loop:
      push rcx
      call read_char
      pop rcx
      cmp al,0x9
      je .loop
      cmp al,0x20
      je .loop
      cmp al, 0xA
      je .loop
      cmp rax,0x0 
      je .end ; jump if [rax] contains 0
      mov byte [rcx], al ; push a char to the mem cell
      inc rcx ; the next char's addres
      inc r9 ; counter
      jmp .loop
    .end:
      inc r9
      mov rax,r10
      cmp r9,r8
      jne .end2
      mov byte [rcx], 0x0 
      ret
    .end2:
      mov rax,0x0
      ret
 

; Принимает указатель на строку, пытается
; прочитать из её начала беззнаковое число.
; Возвращает в rax: число, rdx : его длину в символах
; rdx = 0 если число прочитать не удалось
parse_uint:
    xor rax, rax
    xor rcx,rcx
    xor r8b,r8b
    mov r10,10
    .loop:
      mov r8b, [rdi]
      inc rdi
      cmp r8b, 0x30
      jb .end
      cmp r8b, 0x39
      ja .end
      inc rcx
      mul r10
      sub r8b,0x30
      add rax,r8
      jmp .loop
    .end:
      mov rdx,rcx
      ret




; Принимает указатель на строку, пытается
; прочитать из её начала знаковое число.
; Если есть знак, пробелы между ним и числом не разрешены.
; Возвращает в rax: число, rdx : его длину в символах (включая знак, если он был) 
; rdx = 0 если число прочитать не удалось
parse_int:
    xor rax, rax
    xor r8,r8
    mov r8b,[rdi]
    cmp r8b,'-'
    je .neg
    call parse_uint
    ret
    .neg:
      inc rdi
      call parse_uint
      neg rax
      inc rdx
      ret
    .pos:
      inc rdi
      call parse_uint
      inc rdx
      ret
; Принимает указатель на строку, указатель на буфер и длину буфера
; Копирует строку в буфер
; Возвращает длину строки если она умещается в буфер, иначе 0
string_copy:
    xor rax, rax
    call string_length
    cmp rax,rdx
    jg .print_zero
    .loop:
    mov rdx, [rdi]
    mov [rsi], rdx
    cmp dl,0 
    je .end
    inc rdi
    inc rsi
    jmp .loop
    .end:
      ret
    .print_zero:
      mov rax,0
      ret

