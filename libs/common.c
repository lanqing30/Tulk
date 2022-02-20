
#include "common.h"

/*
关于IO的内联汇编可以参考这里：http://wiki.osdev.org/Inline_Assembly/Examples#I.2FO_access
*/

// 端口写一个字节
inline void outb(uint16_t port, uint8_t value)
{
	asm volatile ("outb %1, %0" : : "dN" (port), "a" (value));
	// 没有输出output，只有input
	// Nd allows for one-byte constant values to be assembled as constants

}

// 端口读一个字节
inline uint8_t inb(uint16_t port)
{
	uint8_t ret;

	asm volatile("inb %1, %0" : "=a" (ret) : "dN" (port));

	return ret;
}

// 端口读一个字
inline uint16_t inw(uint16_t port)
{
	uint16_t ret;

	asm volatile ("inw %1, %0" : "=a" (ret) : "dN" (port));

	return ret;
}

// 开启中断
inline void enable_intr()
{
    asm volatile ("sti");
}

// 关闭中断
inline void disable_intr()
{
    asm volatile ("cli" ::: "memory");
}


