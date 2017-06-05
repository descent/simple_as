.section .rodata
LC1: 
    .string "1+2 = %d\n"
.text
.global main
.type main, @function
main:
pushl %ebp
movl %esp, %ebp
subl $0, %esp # reserve local variable
movl $1, %eax
ADD $2, %eax
pushl %eax
popl %eax
pushl %eax
pushl $LC1
call printf
addl $8, %esp
call xyz
leave
ret

xyz:
ret

