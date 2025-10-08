/*
 * libc.h - macros per fer els traps amb diferents arguments
 *          definició de les crides a sistema
 */
 
#ifndef __LIBC_H__
#define __LIBC_H__

#include <stats.h>

int write(int fd, const char *buffer, int size);

void itoa(int a, char *b);

int strlen(const char *a);

int getpid();

int fork();

void exit();

int gettime(void);

void perror(const char *s);

#endif  /* __LIBC_H__ */
