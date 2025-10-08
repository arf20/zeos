/*
 * interrupt.c -
 */
#include <types.h>
#include <interrupt.h>
#include <segment.h>
#include <hardware.h>
#include <io.h>
#include <entry.h>

#include <zeos_interrupt.h>

Gate idt[IDT_ENTRIES];
Register    idtR;

int zeos_ticks;

char char_map[] =
{
  '\0','\0','1','2','3','4','5','6',
  '7','8','9','0','\'','¡','\0','\0',
  'q','w','e','r','t','y','u','i',
  'o','p','`','+','\0','\0','a','s',
  'd','f','g','h','j','k','l','ñ',
  '\0','º','\0','ç','z','x','c','v',
  'b','n','m',',','.','-','\0','*',
  '\0','\0','\0','\0','\0','\0','\0','\0',
  '\0','\0','\0','\0','\0','\0','\0','7',
  '8','9','-','4','5','6','+','1',
  '2','3','0','\0','\0','\0','<','\0',
  '\0','\0','\0','\0','\0','\0','\0','\0',
  '\0','\0'
};

void setInterruptHandler(int vector, void (*handler)(), int maxAccessibleFromPL)
{
  /***********************************************************************/
  /* THE INTERRUPTION GATE FLAGS:                          R1: pg. 5-11  */
  /* ***************************                                         */
  /* flags = x xx 0x110 000 ?????                                        */
  /*         |  |  |                                                     */
  /*         |  |   \ D = Size of gate: 1 = 32 bits; 0 = 16 bits         */
  /*         |   \ DPL = Num. higher PL from which it is accessible      */
  /*          \ P = Segment Present bit                                  */
  /***********************************************************************/
  Word flags = (Word)(maxAccessibleFromPL << 13);
  flags |= 0x8E00;    /* P = 1, D = 1, Type = 1110 (Interrupt Gate) */

  idt[vector].lowOffset       = lowWord((DWord)handler);
  idt[vector].segmentSelector = __KERNEL_CS;
  idt[vector].flags           = flags;
  idt[vector].highOffset      = highWord((DWord)handler);
}

void setTrapHandler(int vector, void (*handler)(), int maxAccessibleFromPL)
{
  /***********************************************************************/
  /* THE TRAP GATE FLAGS:                                  R1: pg. 5-11  */
  /* ********************                                                */
  /* flags = x xx 0x111 000 ?????                                        */
  /*         |  |  |                                                     */
  /*         |  |   \ D = Size of gate: 1 = 32 bits; 0 = 16 bits         */
  /*         |   \ DPL = Num. higher PL from which it is accessible      */
  /*          \ P = Segment Present bit                                  */
  /***********************************************************************/
  Word flags = (Word)(maxAccessibleFromPL << 13);

  //flags |= 0x8F00;    /* P = 1, D = 1, Type = 1111 (Trap Gate) */
  /* Changed to 0x8e00 to convert it to an 'interrupt gate' and so
     the system calls will be thread-safe. */
  flags |= 0x8E00;    /* P = 1, D = 1, Type = 1110 (Interrupt Gate) */

  idt[vector].lowOffset       = lowWord((DWord)handler);
  idt[vector].segmentSelector = __KERNEL_CS;
  idt[vector].flags           = flags;
  idt[vector].highOffset      = highWord((DWord)handler);
}


void setIdt()
{
  /* Program interrups/exception service routines */
  idtR.base  = (DWord)idt;
  idtR.limit = IDT_ENTRIES * sizeof(Gate) - 1;
  
  set_handlers();

  /* ADD INITIALIZATION CODE FOR INTERRUPT VECTOR */
  setTrapHandler(14, page_fault_handler_new, 0);
  setInterruptHandler(33, keyboard_handler, 0);
  setInterruptHandler(32, clock_handler, 0);
  setInterruptHandler(0x80, syscall_handler, 3);

  set_idt_reg(&idtR);
}


/* interrupt routines */

void keyboard_routine(void)
{
    unsigned char read_byte = inb(0x60);
    unsigned char scancode = read_byte & 0x7f;
    char make = (read_byte >> 7) & 1;

    if (make) {
        char c = char_map[scancode];
        if (c == 0) c = 'C';
        printc_xy(0, 0, c);
    }
}

void clock_routine(void)
{
    zeos_show_clock();
    zeos_ticks++;
}

static void itoa(int a, char *b)
{
  int i, i1;
  char c;
  
  if (a==0) { b[0]='0'; b[1]=0; return ;}
  
  i=0;
  while (a>0)
  {
    b[i]=(a%10)+'0';
    a=a/10;
    i++;
  }
  
  for (i1=0; i1<i/2; i1++)
  {
    c=b[i1];
    b[i1]=b[i-i1-1];
    b[i-i1-1]=c;
  }
  b[i]=0;
}


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

void *
memset(void *s, char c, unsigned int n) {
    for (unsigned int i = 0; i < n; i++)
        ((char*)s)[i] = c;
    return s;
}

void *
memmove(void *s, void *d, int n) {
    if (s > d)
        for (int i = 0; i < n; i++)
            ((char*)d)[i] = ((char*)s)[i];
    else
        for (int i = n - 1; i >= 0; i--)
            ((char*)d)[i] = ((char*)s)[i];
    return d;
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
    memmove(str, str + pad, len);
    memset(str, c, pad);
    return str;
}


void page_fault_routine_new(unsigned int eip)
{
    printk("\nPage Fault at EIP 0x");
    printk(leftpad(utoa(eip, 16), 8, '0'));
    printk("\n");
    while (1) {}
}

