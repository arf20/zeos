/*
 * sys.c - Syscalls implementation
 */
#include <devices.h>
#include <utils.h>
#include <io.h>
#include <mm.h>
#include <mm_address.h>
#include <sched.h>
#include <p_stats.h>
#include <errno.h>
#include <sys.h>

#define LECTURA 0
#define ESCRIPTURA 1

void * get_ebp();

int check_fd(int fd, int permissions)
{
  if (fd!=1) return -EBADF; 
  if (permissions!=ESCRIPTURA) return -EACCES; 
  return 0;
}

void user_to_system(void)
{
  update_stats(&(current()->p_stats.user_ticks), &(current()->p_stats.elapsed_total_ticks));
}

void system_to_user(void)
{
  update_stats(&(current()->p_stats.system_ticks), &(current()->p_stats.elapsed_total_ticks));
}

int sys_ni_syscall()
{
    return -ENOSYS; 
}

int sys_getpid()
{
    return current()->PID;
}

int global_PID = 1000;

int ret_from_fork()
{
  return 0;
}

int sys_fork(void)
{
  struct list_head *lhcurrent = NULL;
  union task_union *uchild;
  
  /* Any free task_struct? */
  if (list_empty(&freequeue)) return -ENOMEM;

  lhcurrent=list_first(&freequeue);
  
  list_del(lhcurrent);
  
  uchild=(union task_union*)list_head_to_task_struct(lhcurrent);
  
  /* Copy the parent's task struct to child's */
  copy_data(current(), uchild, sizeof(union task_union));
  
  /* new pages dir */
  allocate_DIR((struct task_struct*)uchild);
  
  /* Allocate pages for DATA+STACK */
  int new_ph_pag, pag, i;
  page_table_entry *process_PT = get_PT(&uchild->task);
  for (pag=0; pag<NUM_PAG_DATA; pag++)
  {
    new_ph_pag=alloc_frame();
    if (new_ph_pag!=-1) /* One page allocated */
    {
      set_ss_pag(process_PT, PAG_LOG_INIT_DATA+pag, new_ph_pag);
    }
    else /* No more free pages left. Deallocate everything */
    {
       goto error;
    }
  }

  /* allocate slots */
  slot_t *slots = current()->slots;
  for (int i = 0; i < NR_SLOTS; i++) {
    if (!slots[i].allocated)
      continue;

    for (pag = slots[i].place; pag < slots[i].place + slots[i].npages; pag++) {
      new_ph_pag = alloc_frame();
      if (new_ph_pag != -1)
        set_ss_pag(process_PT, slots[i].place + pag, new_ph_pag);
      else
         goto error;
    }
  }

  /* Copy parent's SYSTEM and CODE to child. */
  page_table_entry *parent_PT = get_PT(current());
  for (pag=0; pag<NUM_PAG_KERNEL; pag++)   {
    set_ss_pag(process_PT, pag, get_frame(parent_PT, pag));
  }
  for (pag=0; pag<NUM_PAG_CODE; pag++) {
    set_ss_pag(process_PT, PAG_LOG_INIT_CODE+pag, get_frame(parent_PT, PAG_LOG_INIT_CODE+pag));
  }
  /* Copy parent's DATA to child. We will use TOTAL_PAGES-1 as a temp logical page to map to */
  for (pag=NUM_PAG_KERNEL+NUM_PAG_CODE; pag<NUM_PAG_KERNEL+NUM_PAG_CODE+NUM_PAG_DATA; pag++)
  {
    /* Map one child page to parent's address space. */
    set_ss_pag(parent_PT, pag+NUM_PAG_DATA, get_frame(process_PT, pag));
    copy_data((void*)(pag<<12), (void*)((pag+NUM_PAG_DATA)<<12), PAGE_SIZE);
    del_ss_pag(parent_PT, pag+NUM_PAG_DATA);
  }
  /* copy slots */
  for (int i = 0; i < NR_SLOTS; i++) {
    if (!slots[i].allocated)
      continue;

    for (pag = slots[i].place; pag < slots[i].place + slots[i].npages; pag++) {
      /* Map one child page to parent's address space. */
      set_ss_pag(parent_PT, pag, get_frame(process_PT, pag));
      copy_data((void*)(pag << 12), (void*)(pag << 12), PAGE_SIZE);
      del_ss_pag(parent_PT, pag);
    }
  }

  /* Deny access to the child's memory space */
  set_cr3(get_DIR(current()));

  uchild->task.PID=++global_PID;
  uchild->task.state=ST_READY;

  int register_ebp;        /* frame pointer */
  /* Map Parent's ebp to child's stack */
  register_ebp = (int) get_ebp();
  register_ebp=(register_ebp - (int)current()) + (int)(uchild);

  uchild->task.register_esp=register_ebp + sizeof(DWord);

  DWord temp_ebp=*(DWord*)register_ebp;
  /* Prepare child stack for context switch */
  uchild->task.register_esp-=sizeof(DWord);
  *(DWord*)(uchild->task.register_esp)=(DWord)&ret_from_fork;
  uchild->task.register_esp-=sizeof(DWord);
  *(DWord*)(uchild->task.register_esp)=temp_ebp;

  /* Set stats to 0 */
  init_stats(&(uchild->task.p_stats));

  /* Queue child process into readyqueue */
  uchild->task.state=ST_READY;
  list_add_tail(&(uchild->task.list), &readyqueue);
  
  return uchild->task.PID;

error:
  /* Deallocate allocated pages. Up to pag. */
  for (i=0; i<pag; i++)
  {
    free_frame(get_frame(process_PT, PAG_LOG_INIT_DATA+i));
    del_ss_pag(process_PT, PAG_LOG_INIT_DATA+i);
  }
  /* Deallocate task_struct */
  list_add_tail(lhcurrent, &freequeue);
  
  /* Return error */
  return -EAGAIN;
}

#define TAM_BUFFER 512

int sys_write(int fd, char *buffer, int nbytes) {
    char localbuffer [TAM_BUFFER];
    int bytes_left;
    int ret;

    if ((ret = check_fd(fd, ESCRIPTURA)))
        return ret;
    if (nbytes < 0)
        return -EINVAL;
    if (!access_ok(VERIFY_READ, buffer, nbytes))
        return -EFAULT;
    
    bytes_left = nbytes;
    while (bytes_left > TAM_BUFFER) {
        copy_from_user(buffer, localbuffer, TAM_BUFFER);
        ret = sys_write_console(localbuffer, TAM_BUFFER);
        bytes_left-=ret;
        buffer+=ret;
    }
    if (bytes_left > 0) {
        copy_from_user(buffer, localbuffer,bytes_left);
        ret = sys_write_console(localbuffer, bytes_left);
        bytes_left-=ret;
    }
    return (nbytes-bytes_left);
}


extern int zeos_ticks;

int sys_gettime()
{
  return zeos_ticks;
}

void sys_exit()
{
    int pid = current()->PID;

    /* for all of our threads (task) */
    for (int i = 0; i < NR_TASKS; i++) {
        if (task[i].task.PID != pid)
            continue;

        page_table_entry *process_PT = get_PT(&task[i].task);

        // Deallocate all the propietary physical pages
        for (int p = 0; p < NUM_PAG_DATA; p++) {
            free_frame(get_frame(process_PT, PAG_LOG_INIT_DATA+i));
            del_ss_pag(process_PT, PAG_LOG_INIT_DATA+i);
        }
      
        /* Free task_struct */
        list_add_tail(&(task[i].task.list), &freequeue);
        task[i].task.PID = -1;
    }
  
    /* Restarts execution of the next process */
    sched_next_rr();
}

/* System call to force a task switch */
int sys_yield()
{
    force_task_switch();
    return 0;
}

extern int remaining_quantum;

int sys_get_stats(int pid, struct stats *st)
{
  int i;
  
  if (!access_ok(VERIFY_WRITE, st, sizeof(struct stats))) return -EFAULT; 
  
  if (pid<0) return -EINVAL;
  for (i=0; i<NR_TASKS; i++)
  {
    if (task[i].task.PID==pid)
    {
      task[i].task.p_stats.remaining_ticks=remaining_quantum;
      copy_to_user(&(task[i].task.p_stats), st, sizeof(struct stats));
      return 0;
    }
  }
  return -ESRCH; /*ESRCH */
}


/* circular buffer and pointers stored here */
event_t keybuff[KEYBUFF_SIZE];
event_t *keyin, *keyout;

/* poll_event handler */
int sys_poll_event(event_t *e) {
    /* check user address */
    if (!access_ok(VERIFY_READ, e, sizeof(event_t)))
        return -EFAULT;

    /* empty buffer */
    if (keyin == keyout)
        return 0;     /* try again */

    /* pop key off buffer */
    *e = *(keyout++);
    /* wrap around */
    if (keyout > (&keybuff[KEYBUFF_SIZE - 1]))
        keyout = keybuff;
    /* success */
    return 1;
}



/* stack:  is bottom of thread stack */
int sys_clone(void (*function)(void*), void *parameter, char *stack) {
    struct list_head *lhcurrent = NULL;
    union task_union *uchild;
    
    if (!access_ok(VERIFY_READ, function, 1))
        return -EFAULT;

    if (!access_ok(VERIFY_WRITE, stack, 1))
        return -EFAULT;
    
    /* Any free task_struct? */
    if (list_empty(&freequeue)) return -ENOMEM;

    lhcurrent=list_first(&freequeue);
    
    list_del(lhcurrent);
    
    uchild=(union task_union*)list_head_to_task_struct(lhcurrent);
    
    /* Copy the parent's task struct to child's */
    copy_data(current(), uchild, sizeof(union task_union));
    
  
    uchild->task.PID = global_PID++;
    uchild->task.state = ST_READY;

    int register_ebp;        /* frame pointer */
    /* Map Parent's ebp to child's stack */
    register_ebp = (int)get_ebp();
    register_ebp = (register_ebp - (int)current()) + (int)(uchild);

    uchild->task.register_esp = register_ebp + sizeof(DWord);

    DWord temp_ebp = *(DWord*)register_ebp;

    /* Prepare child stack for context switch */
    uchild->task.register_esp -= sizeof(DWord);
    *(DWord*)(uchild->task.register_esp) = (DWord)&ret_from_fork;
    uchild->task.register_esp -= sizeof(DWord);
    *(DWord*)(uchild->task.register_esp) = temp_ebp;

    DWord *user_stack = (DWord*)stack;
    
    uchild->stack[KERNEL_STACK_SIZE - 2] = (DWord)&user_stack[-1];
    uchild->stack[KERNEL_STACK_SIZE - 5] = (DWord)function;
    
    /* prepare user stack */
    user_stack[-1]   = 0;                   /* function should never return */
    user_stack[0]    = (DWord)parameter;

    /* Set stats to 0 */
    init_stats(&(uchild->task.p_stats));

    /* Queue child process into readyqueue */
    uchild->task.state = ST_READY;
    list_add_tail(&(uchild->task.list), &readyqueue);
    
    return uchild->task.PID;
}


int sys_thread_exit() {
    /* Free task_struct */
    list_add_tail(&(current()->list), &freequeue);

    current()->PID = -1;
  
    /* Restarts execution of the next process */
    sched_next_rr();
    
    return 0;
}


sem_t* sys_sem_create(int initial_value) {
    static sem_t sems[2*NR_TASKS] = { 0 };  /* allocate twice the semaphores than tasks */

    int i = 0;
    for (; i < 2*NR_TASKS; i++)
        if (!sems[i].used)
            break;

    sems[i].used = 1;
    sems[i].count = initial_value;
    INIT_LIST_HEAD(&sems[i].blocked);

    return &sems[i];
}

int sys_sem_wait(sem_t* s) {
    s->count--;
    if (s->count < 0) {
        list_add_tail(&current()->list, &s->blocked);
        sched_next_rr();
    }
    return 0;
}

int sys_sem_signal(sem_t* s) {
    s->count++;
    if (s->count <= 0) {
        struct list_head *l = list_first(&s->blocked);
        list_del(l);
        list_add_tail(l, &readyqueue);
    }
    return 0;
}

int sys_sem_destroy(sem_t* s) {
    s->used = 0;
    return 0;
}

void *sys_get_slot(DWord size) {
    DWord npages = (size / PAGE_SIZE) + (size % PAGE_SIZE), new_ph_pag;
    slot_t *slots = current()->slots;

    /* search empty slot */
    int slot = 0;
    for (; slot < NR_SLOTS; slot++)
        if (!slots[slot].allocated)
            break;

    if (slot == NR_SLOTS)
        return NULL;

    /* search user space to put slot */
    DWord place = PAG_LOG_INIT_DATA + NUM_PAG_DATA;
    for (int i = 0; i < NR_SLOTS; i++) {
        if (!slots[i].allocated)
            continue;
        if (slots[i].place > place)
            place = slots[i].place + slots[i].npages;
    }

    /* allocate npages at place */
    page_table_entry *process_PT = get_PT(current());
    for (int pag = 0; pag < npages; pag++) {
        new_ph_pag = alloc_frame();
        if (new_ph_pag != -1) {
            set_ss_pag(process_PT, place + pag, new_ph_pag);
        } else {
            /* Deallocate allocated pages. Up to pag. */
            for (int i = 0; i < pag; i++) {
                free_frame(get_frame(process_PT, place + i));
                del_ss_pag(process_PT, place + i);
            }
            
            /* Return error */
            return NULL; 
        }
    }

    slots[slot].allocated = 1;
    slots[slot].place  = place;
    slots[slot].npages = npages;

    return (void*)(place << 12);
}

int sys_del_slot(void *s) {
    slot_t *slots = current()->slots;

    /* search empty slot */
    int slot = 0;
    for (; slot < NR_SLOTS; slot++)
        if (slots[slot].place == ((DWord)s >> 12))
            break;

    if (slot == NR_SLOTS)
        return 0;

    DWord place = slots[slot].place;

    /* free pages */
    page_table_entry *process_PT = get_PT(current());
    for (int i = 0; i < slots[slot].npages; i++) {
        free_frame(get_frame(process_PT, place + i));
        del_ss_pag(process_PT, place + i);
    }
    
    /* free slot */
    slots[slot].allocated = 0;

    return 0;
}

