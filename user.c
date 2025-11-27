#include <libc.h>


void
keyboard_thread(void *data)
{
    event_t e;
    while(1) {
        if (poll_event(&e) == 0) {
            write(1, &e.c, 1);
        }
    }
}


int __attribute__ ((__section__(".text.main")))
main(void)
{
    write(1, "hola", 4);
    write(1, "\x1b[20;20H", 8);
    write(1, "capullo", 7);
    write(1, "\x1b[31m", 5);
    write(1, "akjsdaf", 7);
    write(1, "\x1b[44m\x1b[93m", 10);
    write(1, "akjsdaf", 7);
    write(1, "\x1b[22;20H", 8);
    write(1, "\x1b[0m ", 5);

    char tstack[1024];
    clone(&keyboard_thread, NULL, &tstack[1023]);

    int i = 0;
    while (1) {
        if ((i % 100000000) == 0)
            write(1, "tick\n", 5);
        i++;
    }
}

