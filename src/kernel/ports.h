#ifndef PORTS_H
#define PORTS_H

// VGA CRT Controller
#define PORT_VGA_INDEX 0x3D4
#define PORT_VGA_DATA 0x3D5

#define VGA_REG_CURSOR_START 0x0A
#define VGA_CURSOR_DISABLE 0x20

void outb(unsigned short port, unsigned char value);
unsigned char inb(unsigned short port);

#endif