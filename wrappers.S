#include <asm.h>

.extern errno

#define SYSCALL(name, num)  \
ENTRY(name)                 \
    pushl %ebp;             \
    mov %esp, %ebp;         \
                            \
    pushl %ebx;             \
    pushl %esi;             \
    pushl %edi;             \
                            \
    mov     $num, %eax;     \
                            \
    mov     40(%esp), %ebp; \
    mov     36(%esp), %edi; \
    mov     32(%esp), %esi; \
    mov     28(%esp), %ebx; \
    mov     24(%esp), %ecx; \
    mov     20(%esp), %edx; \
                            \
    int     $0x80;          \
                            \
    cmp     $0, %eax;       \
    jge     syscall_ret;    \
                            \
    neg     %eax;           \
    mov     %eax, errno;    \
    mov     $-1, %eax;      \
                            \
    jmp     syscall_ret

syscall_ret:
    popl %edi
    popl %esi
    popl %ebx
    popl %ebp
 
    ret


// edx and ecx are caller saved, not callee, according to C convention
// pop return address to ecx
#define FAST_SYSCALL(name, num)  \
ENTRY(name)                 \
    pushl   %ebp;           \
    mov     %esp, %ebp;     \
                            \
    pushl   %ebx;           \
    pushl   %esi;           \
    pushl   %edi;           \
                            \
    mov     $num, %eax;     \
                            \
    mov     40(%esp), %ebp; \
    mov     36(%esp), %edi; \
    mov     32(%esp), %esi; \
    mov     28(%esp), %ebx; \
    mov     24(%esp), %ecx; \
    mov     20(%esp), %edx; \
                            \
    pushl   $sysenter_return;\
    pushl   %ebp;           \
    mov     %esp, %ebp;     \
                            \
    sysenter

sysenter_return:
    popl    %ebp
    popl    %ecx

    cmp     $0, %eax
    jge     syscall_ret

    neg     %eax
    mov     %eax, errno
    mov     $-1, %eax

    jmp     syscall_ret

// int gettime();
FAST_SYSCALL(gettime, 10)
// int write(int fd, const void buf, size_t count)
FAST_SYSCALL(write, 4)


