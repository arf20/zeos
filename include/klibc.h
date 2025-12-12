#ifndef _KLIBC_H
#define _KLIBC_H

#define NULL (void*)0

void *memmove(void *dest, const void *src, int n);
int atoi(const char *str);
char *strchr(const char *str, char c);
int strlen(const char *str);
void *memset(void *s, char c, unsigned int n);
void *memcpy(void *dest, const void *src, int n);
char *leftpad(char *str, int n, char c);
void reverse(char *str, int length);
char *utoa(unsigned int num, int base);

#endif /* _KLIBC_H */

