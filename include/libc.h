/*
 * libc.h - macros per fer els traps amb diferents arguments
 *          definició de les crides a sistema
 */
 
#ifndef __LIBC_H__
#define __LIBC_H__

#include <stats.h>
#include <types.h>

extern int errno;

int write(int fd, char *buffer, int size);

int gettime();

int clone(void (*function)(void*), void *parameter, char *stack);

void itoa(int a, char *b);

int strlen(char *a);

void perror();

int getpid();

int fork();

void exit();

int yield();

int get_stats(int pid, struct stats *st);

int poll_event(event_t *e);

#endif  /* __LIBC_H__ */
