/*
 * io.c - 
 */

#include <io.h>

#include <types.h>
#include <util.h>

/**************/
/** Screen  ***/
/**************/

#define WIDTH       80
#define HEIGHT      25

#define VGA_WHITE_ON_BLACK          0x0f
#define VGA_WHITE_ON_BLACK_BLINK    0x8f
#define VGA_RED_ON_BLACK            0x0c

char *screen = (char*)0xb8000;

Byte x = 0, y = 0;

/* Read a byte from 'port' */
Byte inb(unsigned short port) {
    Byte v;
    __asm__ __volatile__ ("inb %w1,%0":"=a" (v):"Nd" (port));
    return v;
}

void screen_set_char(char c, int x, int y, unsigned char color) {
    int off = 2 * ((y * WIDTH) + x);
    screen[off] = c;
    screen[off + 1] = color;
}

void scroll_line() {
    /* Move buffer */
    memcpy(
        screen,
        screen + (2 * WIDTH),
        WIDTH * (HEIGHT - 1) * 2
    );

    /* Clear bottom line */
    //memset(screen + (2 * (HEIGHT - 1) * WIDTH), 2 * WIDTH, 0x00);
    for (int x = 0; x < WIDTH; x++)
        screen_set_char(' ', x, HEIGHT - 1, VGA_WHITE_ON_BLACK);
}

void clear_screen() {
    /* Clear buffer */
    for (int i = 0; i < 2 * WIDTH * HEIGHT; i++)
        screen[i] = 0x00;
    /* Reset cursor to top left */
    x = 0; y = 0;
}

void printc_color(char c, unsigned char color) {
     /* Magic BOCHS debug: writes 'c' to port 0xe9 */
     __asm__ __volatile__ ( "movb %0, %%al; outb $0xe9" ::"a"(c));

    /* handle control characters */
    if (c == '\n') {
        y++;
        x = 0;
    } else if (c == '\b') {
        x--;
        if (x<0) {
            y--;
            x = WIDTH - 1;
        }
    } else {
        screen_set_char(c, x, y, color);
        x++;
        if (x >= WIDTH) {
            y++;
            x = 0;
        }
    }

    /* if after printing, cursor is ouside the screen, do scroll */
    if (y >= HEIGHT) {
        scroll_line();
        y--;
    }
}

void printc(char c) {
    printc_color(c, VGA_WHITE_ON_BLACK);
}

void printc_xy(Byte mx, Byte my, char c) {
    screen_set_char(c, mx, my, VGA_WHITE_ON_BLACK);
}

void printk(const char *s) {
    while (*s)
        printc(*s++);
}

