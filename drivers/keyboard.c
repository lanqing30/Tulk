#include "keyboard.h"
#include "common.h"
#include "types.h"
#include "idt.h"
#include "console.h"
// 这个文件的参考 http://www.osdever.net/bkerndev/Docs/keyboard.htm

/* KBDUS means US Keyboard Layout. This is a scancode table
*  used to layout a standard US keyboard. I have left some
*  comments in to give you an idea of what key is what, even
*  though I set it's array index to 0. You can change that to
*  whatever you want using a macro, if you wish! */

static unsigned char keytable[128] = {
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8',	/* 9 */
  '9', '0', '-', '=', '\b',	/* Backspace */
  '\t',			/* Tab */
  'q', 'w', 'e', 'r',	/* 19 */
  't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',	/* Enter key */
    0,			/* 29   - Control */
  'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',	/* 39 */
 '\'', '`',   0,		/* Left shift */
 '\\', 'z', 'x', 'c', 'v', 'b', 'n',			/* 49 */
  'm', ',', '.', '/',   0,				/* Right shift */
  '*',
    0,	/* Alt */
  ' ',	/* Space bar */
    0,	/* Caps lock */
    0,	/* 59 - F1 key ... > */
    0,   0,   0,   0,   0,   0,   0,   0,
    0,	/* < ... F10 */
    0,	/* 69 - Num lock*/
    0,	/* Scroll Lock */
    0,	/* Home key */
    0,	/* Up Arrow */
    0,	/* Page Up */
  '-',
    0,	/* Left Arrow */
    0,
    0,	/* Right Arrow */
  '+',
    0,	/* 79 - End key*/
    0,	/* Down Arrow */
    0,	/* Page Down */
    0,	/* Insert Key */
    0,	/* Delete Key */
    0,   0,   0,
    0,	/* F11 Key */
    0,	/* F12 Key */
    0,	/* All other keys are undefined */
};

static unsigned char keytable_shift[128] = {
      0,  27, '!', '@', '#', '$', '%', '^', '&', '*',	/* 9 */
  '(', ')', '_', '+', '\b',	/* Backspace */
  '\t',			/* Tab */
  'Q', 'W', 'E', 'R',	/* 19 */
  'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',	/* Enter key */
    0,			/* 29   - Control */
  'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':',	/* 39 */
 '\"', '~',   0,		/* Left shift */
 '|', 'Z', 'X', 'C', 'V', 'B', 'N',			/* 49 */
  'M', '<', '>', '?',   0,				/* Right shift */
  '*',
    0,	/* Alt */
  ' ',	/* Space bar */
    0,	/* Caps lock */
    0,	/* 59 - F1 key ... > */
    0,   0,   0,   0,   0,   0,   0,   0,
    0,	/* < ... F10 */
    0,	/* 69 - Num lock*/
    0,	/* Scroll Lock */
    0,	/* Home key */
    0,	/* Up Arrow */
    0,	/* Page Up */
  '-',
    0,	/* Left Arrow */
    0,
    0,	/* Right Arrow */
  '+',
    0,	/* 79 - End key*/
    0,	/* Down Arrow */
    0,	/* Page Down */
    0,	/* Insert Key */
    0,	/* Delete Key */
    0,   0,   0,
    0,	/* F11 Key */
    0,	/* F12 Key */
    0,	/* All other keys are undefined */
};

// 这个检测函数同时检测按下和松开的

static int shiftdown = 0;
static int ctrldown = 0;
char buffer;

void keyboard_handler(pt_regs *regs) {
    uint8_t ret;
	ret = inb(0x60);//读入键盘输入
    if (ret & 0x80) {
        if (ret == 157) ctrldown = 0;
        if (ret == 170) shiftdown = 0;
    } else {

        if (ret == 42 || ret == 54) {
            shiftdown = 1;
            return;
        } else if (ret == 29) {
            ctrldown = 1;
            return;
        }


        // 首先检查是不是ctrl-B（暂停当前前台进程中运行的程序，暂停状态下的程序）
        if (ctrldown) {
            if (ret == 48) {
                // console_putc_color('b', rc_black, rc_white);
                if (has_front_env) {
                    cur_env->bg = true;
                    has_front_env = false;
                    console_putc_color('>', rc_black, rc_white);
                }
            }
            return;
        }

        if (!shiftdown) buffer = keytable[ret];
        else buffer = keytable_shift[ret];
        console_putc_color(buffer, rc_black, rc_white);
    }
}

void init_keyboard() {
	register_interrupt_handler(IRQ1, keyboard_handler);
}

