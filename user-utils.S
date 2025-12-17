#include <asm.h>

#define FAST_SYSCALL(name, num) \
ENTRY(name)                     \
    pushl   %ebp;               \
    mov     %esp, %ebp;         \
                                \
    pushl   %ebx;               \
    pushl   %esi;               \
    pushl   %edi;               \
                                \
    mov     $num, %eax;         \
                                \
    mov     40(%esp), %ebp;     \
    mov     36(%esp), %edi;     \
    mov     32(%esp), %esi;     \
    mov     28(%esp), %edx;     \
    mov     24(%esp), %ecx;     \
    mov     20(%esp), %ebx;     \
                                \
    pushl   $sysenter_return;   \
    pushl   %ebp;               \
    mov     %esp, %ebp;         \
                                \
    sysenter

sysenter_return:
    popl    %ebp
    popl    %ecx

    cmp     $0, %eax
    jge     syscall_ret

    neg     %eax
    push    %eax
    call    set_errno
    mov     $-1, %eax

    jmp     syscall_ret

syscall_ret:
    popl %edi
    popl %esi
    popl %ebx
    popl %ebp
 
    ret


/* void exit() */
FAST_SYSCALL(exit, 1)
/* int fork() */
FAST_SYSCALL(fork, 2)
/* int write(int fd, const void buf, size_t count) */
FAST_SYSCALL(write, 4)
/* int thread_exit() */
FAST_SYSCALL(thread_exit, 6)
/* int gettime() */
FAST_SYSCALL(gettime, 10)
/* int yield() */
FAST_SYSCALL(yield, 13)
/* int getpid() */
FAST_SYSCALL(getpid, 20)
/* int gettid() */
FAST_SYSCALL(gettid, 21)
/* int sem_create(int initial_value) */
FAST_SYSCALL(sem_create, 25)
/* int sem_wait(sem_t *s) */
FAST_SYSCALL(sem_wait, 26)
/* int sem_signal(sem_t *s) */
FAST_SYSCALL(sem_signal, 27)
/* int sem_destroy(sem_t *s) */
FAST_SYSCALL(sem_destroy, 28)
/* void *get_slot(DWord numbytes) */
FAST_SYSCALL(get_slot, 30)
/* int del_slot(void *slot) */
FAST_SYSCALL(del_slot, 31)
/* int get_stats(int pid, struct stats *st) */
FAST_SYSCALL(get_stats, 35)
/* int poll_event(event_t *e) */
FAST_SYSCALL(poll_event, 36)
/* int clone(void (*function)(void*), void *parameter, char *stack) */
FAST_SYSCALL(clone, 3)
/* void set_errno(int) */
FAST_SYSCALL(set_errno, 16)
/* int get_errno() */
FAST_SYSCALL(get_errno, 17)

