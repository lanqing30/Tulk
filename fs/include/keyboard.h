#ifndef INCLUDE_KEYBOARD_H_
#define INCLUDE_KEYBOARD_H_

#include "common.h"
#include "idt.h"
#include "console.h"


#define BUF_SIZE 80

void keyboard_handler(pt_regs *regs);
void init_keyboard();

extern char buffer;

#endif // INCLUDE_KEYBOARD_H_
