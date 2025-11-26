#include <libc.h>

char buff[24];

int pid;

int __attribute__ ((__section__(".text.main")))
  main(void)
{
    /* Next line, tries to move value 0 to CR3 register. This register is a privileged one, and so it will raise an exception */
     /* __asm__ __volatile__ ("mov %0, %%cr3"::"r" (0) ); */

    write(1, "hola", 4);
    write(1, "\x1b[20;20H", 8);
    write(1, "capullo", 7);
    write(1, "\x1b[31m", 5);
    write(1, "akjsdaf", 7);
    write(1, "\x1b[44m\x1b[93m", 10);
    write(1, "akjsdaf", 7);
    write(1, "\x1b[22;20H", 8);
    write(1, "\x1b[0m ", 5);

    event_t e;
    while(1) {
        if (poll_event(&e) == 0) {
            write(1, &e.c, 1);
        }
    }
}

