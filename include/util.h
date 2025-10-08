#ifndef _UTIL_H
#define _UTIL_H

void itoa(int a, char *b);
void reverse(char *str, int length);
int strlen(const char *str);
char *utoa(unsigned int num, int base);
char *leftpad(char *str, int n, char c);
void *memcpy(void *dst, const void *src, int n);
void *memmove(void *dest, const void *src, int n);
void *memset(void *s, char c, unsigned int n);

#endif /* _UTIL_H */

