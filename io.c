/*
 * io.c - 
 */

#include <io.h>

#include <hardware.h>

#include <types.h>

/**************/
/** Screen  ***/
/**************/

#define NUM_COLUMNS 80
#define NUM_ROWS    25

#define VGA_ADDRESS 0xb8000

/* VGA registers */
#define VGA_CTRL_REGISTER   0x3d4
#define VGA_DATA_REGISTER   0x3d5
#define VGA_OFFSET_LOW      0x0f
#define VGA_OFFSET_HIGH     0x0e

static unsigned char *vgabuff = (unsigned char*)VGA_ADDRESS;

static int x = 0, y = 0;
static unsigned char color = 0x0f;

/* text mode colors */
static const unsigned char color_map[] = {
    /* VGA_WHITE_ON_BLACK       */ 0x0f,
    /* VGA_WHITE_ON_BLACK_BLINK */ 0x8f,
    /* VGA_BLACK_ON_BLACK       */ 0x00,
    /* VGA_RED_ON_BLACK         */ 0x0c,
    /* VGA_DRED_ON_BLACK        */ 0x04,
    /* VGA_GREEN_ON_BLACK       */ 0x0a,
    /* VGA_DGREEN_ON_BLACK      */ 0x02,
    /* VGA_BLUE_ON_BLACK        */ 0x09,
    /* VGA_DBLUE_ON_BLACK       */ 0x01,
    /* VGA_CYAN_ON_BLACK        */ 0x0b,
    /* VGA_DCYAN_ON_BLACK       */ 0x03,
    /* VGA_DGREY_ON_BLACK       */ 0x08
};

unsigned char
inb(unsigned short port) {
    unsigned char result;
    __asm__("in %%dx, %%al" : "=a" (result) : "d" (port));
    return result;
}

void
outb(unsigned short port, unsigned char data) {
    __asm__("out %%al, %%dx" : : "a" (data), "d" (port));
}

static void *
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


void
vga_set_cursor(int x, int y) {
    unsigned short off = (y * NUM_COLUMNS) + x;
    outb(VGA_CTRL_REGISTER, VGA_OFFSET_HIGH);
    outb(VGA_DATA_REGISTER, (unsigned char)((off >> 8) & 0xff));
    outb(VGA_CTRL_REGISTER, VGA_OFFSET_LOW);
    outb(VGA_DATA_REGISTER, (unsigned char)(off & 0xff));
}

void
vga_set_color(int color_index){
    color = color_map[color_index] & 0xff;
}

int
vga_xy_offset(int x, int y) {
    return 2 * (y * NUM_COLUMNS + x);
}

void
vga_set_char(char c, int x, int y) {
    int off = vga_xy_offset(x, y);
    vgabuff[off] = c;
    vgabuff[off + 1] = color;
}

void
vga_scroll_line() {
    /* Move buffer */
    memmove(
        (char*)(vga_xy_offset(0, 0) + VGA_ADDRESS),
        (char*)(vga_xy_offset(0, 1) + VGA_ADDRESS),
        NUM_COLUMNS * (NUM_ROWS - 1) * 2
    );

    /* Clear bottom line */
    for (int x = 0; x < NUM_COLUMNS; x++)
        vga_set_char(' ', x, NUM_ROWS - 1);
}


void
vga_clear() {
    /* Clear buffer */
    for (int i = 0; i < 2 * NUM_COLUMNS * NUM_ROWS; i += 2) {
        vgabuff[i] = 0x00;
        vgabuff[i+1] = color;
    }
    /* Reset cursor to top left */
    vga_set_cursor(0, 0);
}



void
printc(char c) {
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
            x = NUM_COLUMNS - 1;
        }
    } else {
        vga_set_char(c, x, y);
        x++;
        if (x >= NUM_COLUMNS) {
            y++;
            x = 0;
        }
    }

    /* if after printing, cursor is ouside the screen, do scroll */
    if (y >= NUM_ROWS) {
        vga_scroll_line();
        y--;
    }

    vga_set_cursor(x, y);
}

/* DEPRECATED : backwards comaptibility with zeos */
void
printc_xy(Byte mx, Byte my, char c) {
    vga_set_char(c, mx, my);
}

void
printk(char *string) {
    int i;
    for (i = 0; string[i]; i++)
        printc(string[i]);
}

