
#ifndef INCLUDE_COMMON_H_
#define INCLUDE_COMMON_H_

#include "types.h"

// 端口写一个字节
void outb(uint16_t port, uint8_t value);

// 端口读一个字节
uint8_t inb(uint16_t port);

// 端口读一个字
uint16_t inw(uint16_t port);

// 开启中断
void enable_intr();

// 关闭中断
void disable_intr();



//enum {
//	// Kernel error codes -- keep in sync with list in lib/printfmt.c.
//	E_UNSPECIFIED	= 1,	// Unspecified or unknown problem
//	E_BAD_ENV	= 2,	    // Environment doesn't exist or otherwise
//                            // cannot be used in requested action
//	E_INVAL		= 3,	    // Invalid parameter
//	E_NO_MEM	= 4,	    // Request failed due to memory shortage
//	E_NO_FREE_ENV	= 5,	// Attempt to create a new environment beyond
//                            // the maximum allowed
//	E_FAULT		= 6,	    // Memory fault
//	E_NO_SYS	= 7,	    // Unimplemented system call
//
//	MAXERROR
//};


#endif // INCLUDE_COMMON_H_


