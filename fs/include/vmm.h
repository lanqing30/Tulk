#ifndef INCLUDE_VMM_H
#define INCLUDE_VMM_H

#include "common.h"
#include "types.h"

#include "vmm.h"
#include "pmm.h"
#include "env.h"
#include "idt.h"
// 内存的布局模式，仿照JOS的内存布局模式

#define KERNBASE 0xC0000000

#define UVPT 0xbf400000

#define UPAGES 0xbf000000

#define UENVS 0xbf000000

#define KSTACKTOP KERNBASE

#define UTOP UENVS

#define UXSTACKTOP UTOP

#define USTACKTOP 0xbebfe000

#define MAX_PAGE_NUM 10000


#define UTEXT 0x00800000

/**
 * P--位0是存在（Present）标志，用于指明表项对地址转换是否有效。P=1表示有效；P=0表示无效。
 * 在页转换过程中，如果说涉及的页目录或页表的表项无效，则会导致一个异常。
 * 如果P=0，那么除表示表项无效外，其余位可供程序自由使用。
 * 例如，操作系统可以使用这些位来保存已存储在磁盘上的页面的序号。
 */

/**
 * R/W--位1是读/写（Read/Write）标志。如果等于1，表示页面可以被读、写或执行。
 * 如果为0，表示页面只读或可执行。
 * 当处理器运行在超级用户特权级（级别0、1或2）时，则R/W位不起作用。
 * 页目录项中的R/W位对其所映射的所有页面起作用。
 */

/**
 * U/S--位2是用户/超级用户（User/Supervisor）标志。
 * 如果为1，那么运行在任何特权级上的程序都可以访问该页面。
 * 如果为0，那么页面只能被运行在超级用户特权级（0、1或2）上的程序访问。
 * 页目录项中的U/S 位对其所映射的所有页面起作用。
 */

// Page table/directory entry flags.
#define PTE_P		0x001	// Present
#define PTE_W		0x002	// Writeable
#define PTE_U		0x004	// User
#define PTE_PWT		0x008	// Write-Through
#define PTE_PCD		0x010	// Cache-Disable
#define PTE_A		0x020	// Accessed
#define PTE_D		0x040	// Dirty
#define PTE_PS		0x080	// Page Size
#define PTE_G		0x100	// Global

#define KADDR(x)    ((uint32_t)x+KERNBASE)
#define PADDR(x)    ((uint32_t)x-KERNBASE)

#define NPDENTRIES 1024
// 虚拟分页大小
#define PGSIZE 	4096 // 4KB
#define PTSIZE 0x00400000 // 4MB
#define KSTKSIZE	(8*PGSIZE)  // size of a kernel stack
// 页掩码，用于 4KB 对齐
#define PAGE_MASK      0xFFFFF000

#define PTXSHIFT	12		// offset of PTX in a linear address
#define PDXSHIFT	22		// offset of PDX in a linear address

// page directory index
#define PDX(la)		((((uint32_t) (la)) >> PDXSHIFT) & 0x3FF)
// page table index
#define PTX(la)		((((uint32_t) (la)) >> PTXSHIFT) & 0x3FF)

#define PTE_ADDR(pte)	((uint32_t) (pte) & ~0xFFF)

// 获取一个地址的页內偏移
#define OFFSET_INDEX(x) ((x) & 0xFFF)

// 页表成员数
#define PGD_SIZE (PGSIZE/sizeof(pte_t)) // 1024

// 页表成员数
#define PTE_SIZE (PGSIZE/sizeof(uint32_t)) // 1024

#define USERTEXT 0x00800000

// 内核页目录区域
extern pde_t pgd_kern[];
extern char kern_stack[];
// 初始化虚拟内存管理
void init_vmm();

// 更换当前的页目录
void switch_pgd(uint32_t pd);


void page_remove(pde_t *pgdir, void *va);
struct PageInfo* page_lookup(pde_t *pgdir, void *va, pte_t **pte_store); // 将二级页表指针的存起来
int page_insert(pde_t *pgdir, struct PageInfo *pp, void *va, int perm);
void page_decref(struct PageInfo* pp);
// TODO:conflict bug fix
// void map_region(pde_t *pgdir, uint32_t va, size_t size, physaddr_t pa, int perm);
pte_t* pgdir_walk(pde_t *pgdir, const void *va, int create);
#endif 	// INCLUDE_VMM_H
