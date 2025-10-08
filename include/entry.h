/*
 * entry.h - Definició del punt d'entrada de les crides al sistema
 */

#ifndef __ENTRY_H__
#define __ENTRY_H__

void keyboard_handler(void);
void clock_handler(void);
void page_fault_handler_new(void);
void syscall_handler(void);
void fast_syscall_handler(void);

#endif  /* __ENTRY_H__ */
