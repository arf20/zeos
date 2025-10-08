/*
 * io.h - Definició de l'entrada/sortida per pantalla en mode sistema
 */

#ifndef __IO_H__
#define __IO_H__

#include <types.h>

Byte inb (unsigned short port);

/** Screen functions **/
/**********************/
void clear_screen();
void printc_color(char c, unsigned char color);
void printc(char c);
void printc_xy(Byte x, Byte y, char c);
void printk(const char *string);

#endif  /* __IO_H__ */
