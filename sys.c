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
#include <task_switch.h>

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
	return -ENOSYS;
}

int sys_getpid()
{
	return current()->PID;
}

/*int ret_from_fork() {
    return 0;
}*/


int sys_fork()
{
    int PID = -1;

    /* get a free PCB */
    struct list_head *e = list_first(&freequeue);
    if (!e)
        return -EAGAIN;
    struct task_struct *t = list_entry(e, struct task_struct, list);
    list_del(e);
    
    /* copy parent PCB and stack into child's */
    copy_data(current(), t, sizeof(union task_union));

    /* create new page table */
    allocate_DIR(t);

    /* search frames */
    page_table_entry *current_pt = get_PT(current());
    page_table_entry *new_pt = get_PT(t);

    /* shared system pages and user code (init all pages) */
    for (unsigned int pag = 0; pag < TOTAL_PAGES; pag++)
        new_pt[pag] = current_pt[pag];

    /* new user data pages */ 
    for (unsigned int pag = 0; pag < NUM_PAG_DATA; pag++) {
        new_pt[PAG_LOG_INIT_DATA + pag].entry = 0;
        new_pt[PAG_LOG_INIT_DATA + pag].bits.pbase_addr = alloc_frame();
        new_pt[PAG_LOG_INIT_DATA + pag].bits.user = 1;
        new_pt[PAG_LOG_INIT_DATA + pag].bits.rw = 1;
        new_pt[PAG_LOG_INIT_DATA + pag].bits.present = 1;
    }
    
    /* copy parent data to child */
    /* -> temp map child data to current pt */
    for (unsigned int pag = 0; pag < NUM_PAG_DATA; pag++) {
        set_ss_pag(
            current_pt,
            PAG_LOG_INIT_CODE + NUM_PAG_CODE + pag,         /* parent page */
            new_pt[PAG_LOG_INIT_DATA + pag].bits.pbase_addr /* child frame */
        );
    }

    /* -> copy data */
    copy_data(
        (const void*)L_USER_START,
        (void*)(L_USER_START + (PAGE_SIZE * (NUM_PAG_DATA + NUM_PAG_CODE))),
        PAGE_SIZE * NUM_PAG_DATA
    );

    /* -> undo temp mappings */
    for (unsigned int pag = 0; pag < NUM_PAG_DATA; pag++) {
        del_ss_pag(
            current_pt,
            PAG_LOG_INIT_CODE + NUM_PAG_CODE + pag /* parent page */
        );
    }

    /* flush TLB */
    set_cr3(current()->dir_pages_baseAddr);

    /* assign PID */
    PID = current()->PID + 1;
    t->PID = PID;

    /* prepare child system stack to go to fork return 0 */
    long unsigned int *new_sys_stack = ((union task_union*)t)->stack;
    new_sys_stack[KERNEL_STACK_SIZE-19] = (long unsigned int)ret_from_fork;  /* ra */
    new_sys_stack[KERNEL_STACK_SIZE-20] = 0;              /* ebp */

    t->kernel_esp = &new_sys_stack[KERNEL_STACK_SIZE-20];

    /* 1008 y 1007 */

    /* insert new process into ready_queue */
    list_add(&t->list, &readyqueue);
    
    /* return PID of child */
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

