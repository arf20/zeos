#include <plibc.h>
#include <game.h>
#include <ansi.h>


int __attribute__ ((__section__(".text.main")))
main(void)
{
    write(1, "\narfminesweeper for ZeOS\n", 25);

    int size = 8;
    gameInit(size, 10);

    ansi_start(gameGetBoard(), size);
    ansi_destroy();

    gameDestroy();
    
    exit();
}

