

#ifndef INCLUDE_PMM_H
#define INCLUDE_PMM_H

#include "multiboot.h"
#include "vmm.h"
#include "idt.h"
// 进程栈的大小
#define KERN_STACK_SIZE 4096 * 8 // 32K的栈空间，一共是64K空间因为中间有个gap


struct PageInfo {
	int32_t physical_addr;
	uint16_t pp_ref;
};

// 内核文件在内存中的起始和结束位置
// 在链接器脚本中要求链接器定义
extern uint8_t kern_start[];
extern uint8_t kern_end[];
extern uint32_t kern_stack_top;

// 动态分配物理内存页的总数
extern uint32_t phy_page_count;
// 物理内存页面管理的栈
extern struct PageInfo* pmm_stack[]; // 这个大小应该是10000 * 4B = 40K的大小
// 物理内存实际的存储空间
extern struct PageInfo pages[];
// 所有的这些全局变量的位置是在kernel的 .data位置里面

extern void* physical_page_begin;

// 输出 BIOS 提供的物理内存布局
void show_memory_map();

// 初始化物理内存管理
void init_pmm();

// 返回一个内存页的物理地址
struct PageInfo* pmm_alloc_page(int clear);

// 释放申请的内存
void pmm_free_page(struct PageInfo* p);


#endif 	// INCLUDE_PMM_H

