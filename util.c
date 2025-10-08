#include <util.h>

void
reverse(char *str, int length) {
    int start = 0;
    int end = length - 1;
    while (start < end) {
        char temp = str[start];
        str[start] = str[end];
        str[end] = temp;
        end--;
        start++;
    }
}

int
strlen(const char *str) {
    if (!str)
        return 0;
    int i = 0;
    while (str[i]) { i++; }
    return i;
}

char *
utoa(unsigned int num, int base) {
    static char buff[128];

    int i = 0;
    if (num == 0) {
        buff[i++] = '0';
            buff[i] = '\0';
            return buff;
    }

    while (num != 0) {
        unsigned int rem = num % base;
            buff[i++] = (rem > 9) ? (rem - 10) + 'a' : rem + '0';
            num /= base;
    }

    buff[i] = '\0';

    reverse(buff, i);

    return buff;
}

char *
leftpad(char *str, int n, char c) {
    int len = strlen(str);
    int pad = n - len;
    memmove(str + pad, str, len);
    memset(str, c, pad);
    return str;
}

void *
memcpy(void *dest, const void *src, int n) {
    char *d = (char*)dest; const char *s = (char*)src;
    for (int i = 0; i < n; i++)
        *(d + i) = *(s + i);
    return dest;
}

void *
memmove(void *dest, const void *src, int n) {
    char *d = (char*)dest; const char *s = (char*)src;
    if (s > d)
        for (int i = 0; i < n; i++)
            d[i] = s[i];
    else
        for (int i = n - 1; i >= 0; i--)
            d[i] = s[i];
    return dest;
}

void *
memset(void *s, char c, unsigned int n) {
    for (unsigned int i = 0; i < n; i++)
        ((char*)s)[i] = c;
    return s;
}

