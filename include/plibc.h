/*

    arfminesweeper: Cross-plataform multi-frontend game
    Copyright (C) 2023 arf20 (Ángel Ruiz Fernandez)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.

*/

#ifndef _PLIBC_H
#define _PLIBC_H

#include <types.h>
#include <stats.h>

typedef unsigned long size_t;

#define LONG_MAX ((long)(~0UL>>1))
#define LONG_MIN (~LONG_MAX)

#define     errno   get_errno()

extern char char_map[];

/* SYSCALLS */
int write(int fd, const char *buffer, int size);
int gettime();
int clone(void (*function)(void*), void *parameter, char *stack);
int getpid();
int gettid();
int fork();
void exit();
int yield();
int get_stats(int pid, struct stats *st);
int poll_event(event_t *e);
sem_t* sem_create(int initial_value);
int sem_wait(sem_t* s);
int sem_signal(sem_t* s);
int sem_destroy(sem_t* s);
void *get_slot(DWord numbytes);
int del_slot(void *slot);


char *itoa(int num, int base);
char *utoa(unsigned int num, int base);
int atoi(const char *str);
long strtol(const char *nptr, char **endptr, int base);

int isalpha(int c);
int toupper(int c);
int tolower(int c);

int strlen(const char *str);
int strcmp(const char *s1, const char *s2);
int strncmp(const char *s1, const char *s2, size_t n);
char *strchr(const char *str, char c);
char *strstr(const char *haystack, const char *needle);
char *strtok(char *str, const char *delim);
char *strcpy(char *dst, const char *src);
char *strcat(char *dst, const char *src);
void *memmove(void *dest, const void *src, int n);
void *memset(void *s, char c, size_t n);
void *memcpy(void *dest, const void*src, int n);

char putchar(char c);
int puts(const char *s);
int printf(char *format, ...);
char getchar();
char *getsn(char *buff, size_t n);

#endif /* _PLIBC_H */

