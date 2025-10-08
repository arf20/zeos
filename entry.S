/*
 * entry.S - Entry point to system mode from user mode
 */

#include <asm.h>
#include <segment.h>
#include <errno.h>


/**************************************************/
/**** Save & Restore ******************************/
/**                                              **/
/** When we change to privilege level 0 (kernel) **/
/** (through an interrupt, a system call, an     **/
/** exception ...) we must save the state of the **/
/** currently running task (save).               **/
/**                                              **/
/** Stack layout in 'systemCall':                **/
/**                                              **/
/**   0(%esp) - %edx    \                        **/
/**   4(%esp) - %ecx     |                       **/
/**   8(%esp) - %ebx     |                       **/
/**   C(%esp) - %esi     | Register saved        **/
/**  10(%esp) - %edi     |  by 'save'            **/
/**  14(%esp) - %ebp     |                       **/
/**  18(%esp) - %eax     |                       **/
/**  1C(%esp) - %ds      |                       **/
/**  20(%esp) - %es      |                       **/
/**  24(%esp) - %fs      |                       **/
/**  28(%esp) - %gs     /                        **/
/**  2C(%esp) - %eip    \                        **/
/**  30(%esp) - %cs      |                       **/
/**  34(%esp) - %eflags  |  Return context saved **/
/**  38(%esp) - %oldesp  |   by the processor.   **/
/**  3C(%esp) - %oldss  /                        **/
/**                                              **/
/**************************************************/

#define SAVE_ALL \
    pushl   %gs; \
    pushl   %fs; \
    pushl   %es; \
    pushl   %ds; \
    pushl   %eax; \
    pushl   %ebp; \
    pushl   %edi; \
    pushl   %esi; \
    pushl   %ebx; \
    pushl   %ecx; \
    pushl   %edx; \
    movl    $__KERNEL_DS, %edx; \
    movl    %edx, %ds; \
    movl    %edx, %es;

#define RESTORE_ALL \
    popl    %edx; \
    popl    %ecx; \
    popl    %ebx; \
    popl    %esi; \
    popl    %edi; \
    popl    %ebp; \
    popl    %eax; \
    popl    %ds; \
    popl    %es; \
    popl    %fs; \
    popl    %gs;

/* indicates to APIC ok for more interrupts */
#define EOI \
    movb $0x20, %al ; \
    outb %al, $0x20 ;


ENTRY(keyboard_handler)
    SAVE_ALL
    EOI
    call keyboard_routine
    RESTORE_ALL
    iret

ENTRY(clock_handler)
    SAVE_ALL
    EOI
    call clock_routine
    RESTORE_ALL
    iret

ENTRY(page_fault_handler_new)
    SAVE_ALL
    mov     0x30(%esp), %eax    // retrieve error code pushed on top of eip
    pushl   %eax                // pass as argument to routine cdelc
    call page_fault_routine_new
    popl    %eax                // remove parameter
    RESTORE_ALL
    add     $-4, %esp            // clear error
    iret

#define MAX_SYSCALL     10

ENTRY(syscall_handler)
    SAVE_ALL
    cmpl $0, %eax
    jl err
    cmpl $MAX_SYSCALL, %eax
    jg err
    call *sys_call_table(, %eax, 0x04)
    jmp fin
err:
    movl $-ENOSYS, %eax
fin:
    movl %eax, 0x18(%esp)
    RESTORE_ALL
    iret

ENTRY(fast_syscall_handler)
    pushl   $__USER_DS
    pushl   %ebp
    pushfl
    pushl   $__USER_CS
    pushl   4(%ebp)

    SAVE_ALL

    cmpl $0, %eax
    jl enosys
    cmpl $MAX_SYSCALL, %eax
    jg enosys

    call *sys_call_table(, %eax, 0x04)
    jmp fast_syscall_handler_end

enosys:
    movl $-ENOSYS, %eax
fast_syscall_handler_end:
    movl %eax, 0x18(%esp)
    RESTORE_ALL

    movl    (%esp), %edx
    movl    12(%esp), %ecx

    sti
    sysexit

