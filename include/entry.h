#ifndef INCLUDE_ENTRY_H_
#define INCLUDE_ENTRY_H_
#include "env.h"

// 开启分页机制之后的 Multiboot 数据指针
extern multiboot_t *glb_mboot_ptr;

// 开启分页机制之后的内核栈
extern char kern_stack[]; // 这里声明就自动放入高地址代码段的.data区域。

extern uint32_t kern_stack_top;

extern void kern_init();

extern void kern_entry();

#endif // INCLUDE_ENTRY_H

