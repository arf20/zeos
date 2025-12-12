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

/* === IO port in and out */

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

/* === structural libc stuff */

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

int
atoi(const char *str) {
    int res = 0;
    while (*str >= '0' && *str <= '9')
        res = (res * 10) + *str++ - '0';
    return res;
}

char *
strchr(const char *str, char c) {
    while (*str) {
        if (*str == c) return (char*)str;
        str++;
    }

    return NULL;
}

int
strlen(const char *str) {
    int i = 0;
    while (str[i]) { i++; }
    return i;
}

/* === VGA text mode driver */

/* set VGA blinking cursor with VGA commands */
void
vga_set_cursor(int x, int y) {
    unsigned short off = (y * NUM_COLUMNS) + x;
    outb(VGA_CTRL_REGISTER, VGA_OFFSET_HIGH);
    outb(VGA_DATA_REGISTER, (unsigned char)((off >> 8) & 0xff));
    outb(VGA_CTRL_REGISTER, VGA_OFFSET_LOW);
    outb(VGA_DATA_REGISTER, (unsigned char)(off & 0xff));
}

/* calculate offset in buffer of position */
int
vga_xy_offset(int x, int y) {
    return 2 * (y * NUM_COLUMNS + x);
}

/* write character to VGA buffer */
void
vga_set_char(char c, int x, int y) {
    int off = vga_xy_offset(x, y);
    vgabuff[off] = c;
    vgabuff[off + 1] = color;
}

void
vga_scroll_line() {
    /* move buffer */
    memmove(
        (char*)(vga_xy_offset(0, 0) + VGA_ADDRESS),
        (char*)(vga_xy_offset(0, 1) + VGA_ADDRESS),
        NUM_COLUMNS * (NUM_ROWS - 1) * 2
    );

    /* clear bottom line */
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
handle_csi(char *csi) {
    /* map ANSI color codes to VGA 3-4 bit color byte */
    static const char ansi_color[] = {
        0x0,
        0x4,
        0x2,
        0xe,
        0x1,
        0x5,
        0x3,
        0xf
    };

    static const char ansi_color_bright[] = {
        0x7,
        0xc,
        0xa,
        0xe,
        0x9,
        0xd,
        0xb,
        0xf
    };


    /* parse the one or two numerical arguments separated by ; */
    int n = atoi(csi);
    char *mpos = strchr(csi, ';') + 1;
    int m = -1;
    if (mpos != (void*)1)
        m = atoi(mpos);
    
    /* CSI terminator is command */
    char cmd = csi[strlen(csi) - 1];
    switch (cmd) {
        /* move cursor command */
        case 'H': {
            /* requires two arguments */
            if (mpos == (void*)1)
                return;

            /* check bounds */
            if (n < 0 || n >= NUM_ROWS || m < 0 || m >= NUM_COLUMNS)
                return;

            /* set cursor */
            x = m; y = n;
            vga_set_cursor(m, n);
        } break;
        /* set color command */
        case 'm': {
            /* 0        reset color
             * 30-37    normal foreground
             * 90-97    bright foreground
             * 40-47    normal background
             * 100-107  bright baclground */
            if (n == 0)
                color = 0x0f; /* default */
            else if (n >= 30 && n <= 37)
                color = ansi_color[n - 30] | (color & 0xf0);
            else if (n >= 90 && n <= 97)
                color = ansi_color_bright[n - 90] | (color & 0xf0);
            else if (n >= 40 && n <= 47)
                color = ((ansi_color[n - 40] & 0x7) << 4) | (color & 0x0f);
            else if (n >= 100 && n <= 107)
                color = ((ansi_color_bright[n - 100] & 0x7) << 4) | (color & 0x0f);
            else return;
        } break;
    }
}

/* === terminal interface */

/* printc: ANSI state machine parser */
void
printc(char c) {
    /* Magic BOCHS debug: writes 'c' to port 0xe9 */
    __asm__ __volatile__ ( "movb %0, %%al; outb $0xe9" ::"a"(c)); 
    /* handle control characters */

    static int esc = 0, csi = 0;
    static char csibuff[32];
    static int pos = 0;

    switch (c) {
        /* states */
        case '\x1b': {  /* ESC */
            esc = 1;
        } break;
        case '[': {     /* ESC [ (CSI sequence) */
            csi = esc; 
        } break;

        /* ASCII control characters */
        case '\n': {    /* NL: goes to the first position in the next line */
            y++;
            x = 0;
        } break;
        case '\b': {    /* BS: backspace moves back 1 position with wrap around */
            x--;
            if (x < 0) {
                y--;
                x = NUM_COLUMNS - 1;
            }
        } break;
        default: {
            if (csi) {
                /* if at a CSI escape sequence has been initiated
                 * save characters in the CSI buffer until a sequence
                 * terminator */
                csibuff[pos++] = c;

                /* terminators */
                if (c == 'H' || c == 'm') {
                    csibuff[pos] = '\0';    /* C-string NUL */
                    esc = csi = pos = 0;    /* reset state machine and buff */
                    handle_csi(csibuff);    /* call CSI handler */
                }
                /* prevent printing the CSI characters */
                return;
            }

            /* write the character at the cursor position in the VGA buffer */
            vga_set_char(c, x, y);
            /* next position */
            x++;
            /* wrap around to next line */
            if (x >= NUM_COLUMNS) {
                y++;
                x = 0;
            }
        } break;
    }

    /* do scroll when cursor goes beyond the end of the buffer */
    if (y >= NUM_ROWS) {
        vga_scroll_line();
        y--;
    }

    /* set VGA blinking cursor to the current cursor position */
    vga_set_cursor(x, y);
}

/* DEPRECATED: backwards compatibility with legacy ZeOS code */
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

