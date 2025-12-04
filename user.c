#include <plibc.h>
#include <game.h>
#include <ansi.h>

static sem_t *sem = NULL;

void
keyboard_thread(void *data)
{
    while(1) {
        sem_wait(sem);
        printf("%c\n", getchar()) ;
        sem_signal(sem);
    }
}


int __attribute__ ((__section__(".text.main")))
main(void)
{
    write(1, "\narfminesweeper for ZeOS\n", 25);

    int size = 8;
    gameInit(size, 10);

    sem = sem_create(1);

    char tstack[1024];
    clone(&keyboard_thread, NULL, &tstack[1023]);

    ansi_start(gameGetBoard(), size);


    ansi_destroy();

    gameDestroy();
    
    sem_destroy(sem);
}

