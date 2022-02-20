

#ifndef INCLUDE_GDT_H_
#define INCLUDE_GDT_H_

#include "types.h"

// 参考 http://wiki.osdev.org/GDT

// Global descriptor numbers
#define GD_KT     0x08     // kernel text
#define GD_KD     0x10     // kernel data
#define GD_UT     0x18     // user text
#define GD_UD     0x20     // user data
#define GD_TSS0   0x28     // Task segment selector for CPU 0


#define GDT_LENGTH 5

// 全局描述符类型
typedef
struct gdt_entry_t {
	uint16_t limit_low;     // 段界限   15～0
	uint16_t base_low;      // 段基地址 15～0
	uint8_t  base_middle;   // 段基地址 23～16
	uint8_t  access;        // 段存在位、描述符特权级、描述符类型、描述符子类别
	uint8_t  granularity; 	// 其他标志、段界限 19～16
	uint8_t  base_high;     // 段基地址 31～24
} __attribute__((packed)) gdt_entry_t;

// GDTR
typedef
struct gdt_ptr_t {
	uint16_t limit; 	// 全局描述符表限长
	uint32_t base; 		// 全局描述符表 32位 基地址
} __attribute__((packed)) gdt_ptr_t;



// 全局描述符表定义
extern gdt_entry_t gdt_entries[GDT_LENGTH];

// GDTR
extern gdt_ptr_t gdt_ptr;

// 初始化全局描述符表
void init_gdt();



// GDT 加载到 GDTR 的函数[汇编实现]
extern void gdt_flush(uint32_t);

// 声明内核栈地址，在汇编中
extern uint32_t stack;

#endif 	// INCLUDE_GDT_H_
