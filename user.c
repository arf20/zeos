#include <libc.h>

char buff[24];

int pid;

int add(int a, int b)
{
    return a + b;
}

extern int addAsm(int a, int b);

int __attribute__ ((__section__(".text.main")))
  main(void)
{
    /* Next line, tries to move value 0 to CR3 register. This register is a privileged one, and so it will raise an exception */
     /* __asm__ __volatile__ ("mov %0, %%cr3"::"r" (0) ); */

    int c = addAsm(69, 420);

    int time = gettime();

    write(1, "asdf\n", 5);

    if (write(-1, "", -1) < 0)
        perror("write(1) failed");

    char pid = getpid() + '0';
    write(1, "pid=", 4);
    write(1, &pid, 1);
    write(1, "\n", 1);

    /*int cpid = fork();
    if (cpid == 0) {
        write(1, "child\n", 6);
    } else {
        char ccpid = cpid + '0';
        write(1, "parent, child pid = ", 20);
        write(1, &ccpid, 1);
        write(1, "\n", 1);
    }*/
    
    while(1) { }
}

