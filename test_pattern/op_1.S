
  mov a, 2 
        
.text
                
.globl main
  .type main, @function
main:
pushl %ebp
movl %esp,%ebp
subl $0, %esp # reserve local variable
movl $1, %eax
ADD $2   ,      %eax
pushl %eax
leave
ret
