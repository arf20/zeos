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
    int PID = -1;

    /* get a free PCB */
    struct list_head *e = list_first(&freequeue);
    if (!e)
        return -EAGAIN;
    struct task_struct *t = list_entry(e, struct task_struct, list);
    list_del(e);
    
    /* copy parent PCB into child's */
    copy_data(current(), t, sizeof(union task_union));
    /* create new page table */
    allocate_DIR(t);
    /* search frames */
    alloc_frame();

    int pag; 
    int new_ph_pag;
    page_table_entry *process_PT = get_PT(t);


    /* CODE */
    for (pag=0;pag<NUM_PAG_CODE;pag++){
        new_ph_pag=alloc_frame();
        process_PT[PAG_LOG_INIT_CODE+pag].entry = 0;
        process_PT[PAG_LOG_INIT_CODE+pag].bits.pbase_addr = new_ph_pag;
        process_PT[PAG_LOG_INIT_CODE+pag].bits.user = 1;
        process_PT[PAG_LOG_INIT_CODE+pag].bits.present = 1;
    }
    
    /* DATA */ 
    for (pag=0;pag<NUM_PAG_DATA;pag++){
        new_ph_pag=alloc_frame();
        process_PT[PAG_LOG_INIT_DATA+pag].entry = 0;
        process_PT[PAG_LOG_INIT_DATA+pag].bits.pbase_addr = new_ph_pag;
        process_PT[PAG_LOG_INIT_DATA+pag].bits.user = 1;
        process_PT[PAG_LOG_INIT_DATA+pag].bits.rw = 1;
        process_PT[PAG_LOG_INIT_DATA+pag].bits.present = 1;
    }
    
    
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

