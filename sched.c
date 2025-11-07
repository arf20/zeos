/*
 * sched.c - initializes struct for task 0 anda task 1
 */

#include <sched.h>
#include <mm.h>
#include <io.h>
#include <task_switch.h>
#include <hardware.h>
#include <util.h>

#define DEFAULT_QUANTUM 100

union task_union task[NR_TASKS]
  __attribute__((__section__(".data.task")));

#if 0
struct task_struct *list_head_to_task_struct(struct list_head *l)
{
  return list_entry( l, struct task_struct, list);
}
#endif

extern struct list_head blocked;

struct list_head freequeue;
struct list_head readyqueue;

struct task_struct *idle_task;

static int current_quantum = DEFAULT_QUANTUM;


/* get_DIR - Returns the Page Directory address for task 't' */
page_table_entry *get_DIR(struct task_struct *t) 
{
	return t->dir_pages_baseAddr;
}

/* get_PT - Returns the Page Table address for task 't' */
page_table_entry *get_PT(struct task_struct *t) 
{
	return (page_table_entry *)(((unsigned int)(t->dir_pages_baseAddr->bits.pbase_addr))<<12);
}


int allocate_DIR(struct task_struct *t) 
{
	int pos;

	pos = ((int)t - (int)task)/sizeof(union task_union); /* index de task en arry */

	t->dir_pages_baseAddr = (page_table_entry*)&dir_pages[pos]; 

	return 1;
}

void cpu_idle(void)
{
	__asm__ __volatile__("sti": : :"memory");

	while (1) {

	}
}

void init_idle(void)
{
    struct list_head *e = list_first(&freequeue);
    idle_task = list_entry(e, struct task_struct, list);
    list_del(e);

    idle_task->PID = 0;
    allocate_DIR(idle_task);
    unsigned long *idle_stack = ((union task_union*)idle_task)->stack;
    idle_stack[1023] = (unsigned long)cpu_idle; /* function for the process to execute */
    idle_stack[1022] = (unsigned long)0; /* process %ebp, 0 because it doesnt use the stack, so it doesnt need a valid ebp */

    //save_ebp(&idle_task->kernel_esp);
    idle_task->kernel_esp = &idle_stack[1022];

    idle_task->quantum = DEFAULT_QUANTUM;
}

void init_task1(void)
{
    /* get a free PCB */
    struct list_head *e = list_first(&freequeue);
    struct task_struct *t = list_entry(e, struct task_struct, list);
    list_del(e);

    /* assign PID1 and allocate pages */
    t->PID = 1;
    allocate_DIR(t);
    set_user_pages(t);

    /* set stack and return address */
    DWord system_stack = (DWord)&((union task_union*)t)->stack[1023];
    tss.esp0 = (DWord)system_stack;
    write_msr(0x175, system_stack, 0);

    /* set cr3 to this process table */
    set_cr3(t->dir_pages_baseAddr);

    t->quantum = DEFAULT_QUANTUM;
    t->state = ST_READY;
}

void init_sched()
{
    INIT_LIST_HEAD(&freequeue);
    for (int i = 0; i < NR_TASKS; i++)
        list_add_tail(&task[i].task.list, &freequeue);

    INIT_LIST_HEAD(&readyqueue);
}

struct task_struct *current()
{
    int ret_value;
    
    __asm__ __volatile__(
    	"movl %%esp, %0"
      : "=g" (ret_value)
    );
    return (struct task_struct*)(ret_value&0xfffff000);
}


int get_quantum(struct task_struct *t)
{
    return t->quantum;
}

void set_quantum(struct task_struct *t, int quantum)
{
    t->quantum = quantum;
}

void inner_task_switch(union task_union *new)
{
    /* set new sysenter system stack */
    tss.esp0 = (DWord)&new->stack[1023];
    write_msr(0x175, tss.esp0, 0);
    /* set new page table */
    set_cr3(new->task.dir_pages_baseAddr);
    /* sabe ebp */
    save_ebp(&current()->kernel_esp);
   
    inner_task_return(); /* call inner_task_return */
}

void update_process_state_rr(struct task_struct *t, struct list_head *dest)
{
    
    if (t != current())
        list_del(&t->list);

    if (!dest) {
        t->state = ST_RUN;
        return;
    }
    list_add_tail(&t->list, dest);

    if (dest == &readyqueue)
        t->state = ST_READY;
    else if (dest == &blocked)
        t->state = ST_BLOCKED;
}

void sched_next_rr()
{
    union task_union *next = NULL;
    struct list_head *entry = list_first(&readyqueue);
    if (entry != &readyqueue) {
        next = (union task_union*)list_entry(entry, struct task_struct, list);
        if (entry->next && entry->prev)
            list_del(entry);
    }
    else if (current()->state != ST_EXITED)
        next = (union task_union*)current();
    else
        next = (union task_union*)idle_task;

    set_quantum(current(), DEFAULT_QUANTUM);
    if ((current() != idle_task) && (current()->state != ST_EXITED))
        update_process_state_rr(current(), &readyqueue);
    
    current_quantum = get_quantum(&next->task);
    if (next != current())
        task_switch(next);
}

int needs_sched_rr()
{
    return current_quantum == 0 ? 1 : 0;
}

void update_sched_data_rr()
{
    current_quantum--;
}


void schedule()
{
    printk("current pid: ");
    printk(utoa(current()->PID, 10));
    printk("\n");

    update_sched_data_rr();
    if (needs_sched_rr()) {
        sched_next_rr();
    }
}

