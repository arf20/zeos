#ifndef _TASK_SWITCH_H
#define _TASK_SWITCH_H

void save_ebp(void *target);
void task_switch(union task_union *new);
void inner_task_return(void);

#endif /* _TASK_SWITCH_H */

