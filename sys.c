/*
 * sys.c - Syscalls implementation
 */
#include <devices.h>

#include <utils.h>

#include <io.h>

#include <mm.h>

#include <mm_address.h>

#include <sched.h>

#include <errno.h>

#define LECTURA 0
#define ESCRIPTURA 1

#define MIN(x, y)   (x > y ? y : x)

#define IOBS    4096
static char iobuffer[IOBS];

extern int zeos_ticks;

int check_fd(int fd, int permissions)
{
    if (fd != 1)
        return -EBADFD;
    if (permissions != ESCRIPTURA)
        return -EACCES;
    return 0;
}

int sys_ni_syscall()
{
	return -38; /*ENOSYS*/
}

int sys_getpid()
{
	return current()->PID;
}

int sys_fork()
{
    int PID=-1;

    // creates the child process
    
    return PID;
}

void sys_exit()
{  
}

int sys_gettime()
{
    return zeos_ticks;
}

int sys_write(int fd, const void *buf, int count)
{
    /* check arguemnts */
    int cfdr = check_fd(fd, ESCRIPTURA);
    if (cfdr < 0)
        return cfdr;
    if (!buf)
        return -EFAULT;
    if (count < 0)
        return -EINVAL;

    /* print data 1 block at a time */
    int written = 0, sysr = 0;
    for (; count > 0;) {
        copy_from_user(buf, iobuffer, IOBS);
        buf += IOBS; /* next block */
        sysr = sys_write_console(iobuffer, MIN(IOBS, count));
        if (sysr < 0)
            return sysr;
        written += sysr;
        count -= IOBS;
    }

    return written;
}

