#include <asm.h>

/*
esp>edi
ebp>ebp de init_idle
    ra  de init_idle
    &kernel_esp
    ...
    ebp de init_sched
    ra  de init_sched
*/

/* void save_ebp(void*) */
ENTRY(save_ebp)
    pushl %ebp
    mov %esp, %ebp

    pushl %edi

    mov (%ebp), %eax    // coger %ebp de idle a %eax

    mov 8(%ebp), %edi   // %edi = &kernel_esp
    mov %eax, (%edi)    // *kernel_esp = %ebp de idle

    popl %edi
    
    popl %ebp
    ret


/*
esp>0   %ebx
    4   %edi
    8   %esi
    12  ra keyboard_routine
    16  new
*/

//void task_switch(union task_union *new)
ENTRY(task_switch)
    pushl   %esi
    pushl   %edi
    pushl   %ebx
    pushl   16(%esp)
    call    inner_task_switch
    sub     -4, %ebp
    popl    %ebx
    popl    %edi
    popl    %esi

   
/*
    0   ra      de inner_task_switch
    4   %ebp    de task_switch
    8   ra      de task_switch
    12  &new
*/

/* void inner_task_return(void): set stack to new */
ENTRY(inner_task_return)
    mov 4(%esp), %eax  /* %eax = &new */
    mov 8(%eax), %esp   /* %esp = new->kernel_esp ESTAMOS EN NUEVO */
     
    popl %ebp
    ret

/*
kesp>   1022    %ebp = 0
        1023    ra   -> cpu_idle
*/




ENTRY(ret_from_fork)
    xor %eax, %eax
    popl %ebp
    ret

