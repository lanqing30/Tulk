
#include "multiboot.h"
#include "common.h"
#include "debug.h"
#include "pmm.h"
#include "vmm.h"
#include "string.h"

// 物理内存管理的栈指针
static uint32_t pmm_stack_top;
// 所有的这些全局变量的位置是在kernel的 .data位置里面

// 物理内存页面管理的栈
struct PageInfo* pmm_stack[MAX_PAGE_NUM+1]; // 这个大小应该是10000 * 4B = 40K的大小
// 物理内存实际的存储空间
// 实际上我们发现实际的物理地址和pageinfo的地址能够一一对应
struct PageInfo pages[MAX_PAGE_NUM+1];

// 物理内存页的数量
uint32_t phy_page_count;
void* physical_page_begin;

void show_memory_map() // debug， qemu设置的内存大小是128M左右
{
	uint32_t mmap_addr = glb_mboot_ptr->mmap_addr;
	uint32_t mmap_length = glb_mboot_ptr->mmap_length;

	printk("Memory map:\n");

	mmap_entry_t *mmap = (mmap_entry_t *)mmap_addr;
	for (mmap = (mmap_entry_t *)mmap_addr; (uint32_t)mmap < mmap_addr + mmap_length; mmap++) {
		printk("base_addr = 0x%X%08X, length = 0x%X%08X, type = 0x%X\n",
			(uint32_t)mmap->base_addr_high, (uint32_t)mmap->base_addr_low,
			(uint32_t)mmap->length_high, (uint32_t)mmap->length_low,
			(uint32_t)mmap->type); // 只有类型是1的才是可用的
	}
}

void init_pmm()
{
	mmap_entry_t *mmap_start_addr = (mmap_entry_t *)glb_mboot_ptr->mmap_addr;
	mmap_entry_t *mmap_end_addr = (mmap_entry_t *)glb_mboot_ptr->mmap_addr + glb_mboot_ptr->mmap_length;

	mmap_entry_t *map_entry;
    uint32_t counter = 0;
	for (map_entry = mmap_start_addr; map_entry < mmap_end_addr; map_entry++) {

		// 如果是可用内存 ( 按照协议，1 表示可用内存，其它数字指保留区域 )
		// 运行到虚拟机发现，其实可以使用的内存就2段，我们不使用低于1M的那部分内存。
		if (map_entry->type == 1 && map_entry->base_addr_low == 0x100000) {
			// 把内核结束位置到结束位置的内存段，按页存储到页管理栈里
			// 最多支持40MB的物理内存，这里的page_addr是物理内存地址
			uint32_t page_addr = map_entry->base_addr_low + (uint32_t)(kern_end - kern_start);
			physical_page_begin = (void*)page_addr;
			uint32_t page_addr_end = map_entry->base_addr_low + map_entry->length_low;
			while (page_addr < page_addr_end && phy_page_count < MAX_PAGE_NUM) { // 最多允许40M的物理内存，其他的用作其他用途
                pages[counter].physical_addr = page_addr;
                pages[counter].pp_ref = 1;
				pmm_stack[pmm_stack_top] = &pages[counter];
				++ pmm_stack_top; ++ counter;
				page_addr += PGSIZE;
				phy_page_count++;
			}
		}
	}
	// 应该将栈逆转一下，因为刚开始的空间如果是大的地址话，是没有被映射的。
	// 栈中的空间使用情况是[1, pmm_stack_top]，
	uint32_t mid = (1+pmm_stack_top)/2;
	uint32_t begin = 1;
	while (begin != mid) {
        uint32_t other = pmm_stack_top + 1 - begin;
        struct PageInfo*temp = pmm_stack[begin];
        pmm_stack[begin] = pmm_stack[other];
        pmm_stack[other] = temp;
        begin += 1;
	}
}

struct PageInfo* pmm_alloc_page(int clear) {
	assert(pmm_stack_top != 0, "out of memory");
	struct PageInfo* page = pmm_stack[pmm_stack_top--];
	if (clear) {
        memset(KADDR(page->physical_addr), 0, PGSIZE);
	}
	page->pp_ref = 0;
	return page;
}

void pmm_free_page(struct PageInfo* page)
{
    assert(page->pp_ref == 0, "page reference is not 0...");
	assert(pmm_stack_top != MAX_PAGE_NUM, "out of pmm_stack stack");
	pmm_stack[++pmm_stack_top] = page;
}










