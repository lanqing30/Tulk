
#include "idt.h"
#include "string.h"
#include "debug.h"
#include "vmm.h"
#include "pmm.h"
#include "env.h"

// 内核页目录区域大小是1K
pde_t pgd_kern[PGD_SIZE] __attribute__ ((aligned(PGSIZE)));

// 这个函数的作用是，给定一个虚拟地址，找到这个虚拟地址所在的物理页面，返回这个物理页面在二级页表中的那个指针
pte_t* pgdir_walk(pde_t *pgdir, const void *va, int create)
{
	unsigned int page_off;
	pte_t *page_base = NULL;
	struct PageInfo* new_page;
	unsigned int dic_off = PDX(va);
	if(!(pgdir[dic_off] & PTE_P))           // 如果这个页表目录项不存在
	{
		if(create) {
			new_page = pmm_alloc_page(1);   // 为什么开辟一个页就够呢？ 4B × 1024个页面信息就是 4K
            // 注意这里ppm_alloc返回的是物理地址，可能很大
			pgdir[dic_off] = (new_page->physical_addr | PTE_P | PTE_W);
            // 这里存放的也是物理地址，因为虚拟地址位数根本不够，三个标志分别代表，存在，可写，用户
		}
		else
			return NULL;                // 不创建对象，直接返回失败的情况
	}
	page_off = PTX(va);
	page_base = (pte_t*)KADDR(PTE_ADDR(pgdir[dic_off])); // 首先将一个二级页表的指针转化成物理地址
	// 获取到物理地址之后，KADDR的含义大概就是将物理地址转化成虚拟地址。 // TODO这里的KADDR的含义是将一个物理地址转化成虚拟地址，为什么是正确的呢？
	// 因为进程的创建一定是在内核模式下进行的
	return &page_base[page_off]; // 返回的也是虚拟地址。
}

// 给定一个页表目录，将一个虚拟地址映射到目标物理地址，并且给定一定的长度，和权限perm
void map_region(pde_t *pgdir, uint32_t va, size_t size, physaddr_t pa, int perm)
{
	int nadd;
	pte_t *entry = NULL;
	for(nadd = 0; nadd < size; nadd += PGSIZE)
	{
		entry = pgdir_walk(pgdir,(void *)va, 1);    //Get the table entry of this page.
		assert(entry != NULL, "can not get the entry of this page.\n");
		*entry = (pa | perm | PTE_P);
		pa += PGSIZE;
		va += PGSIZE;

	}
}


// 下面的某些函数到后面进程的部分才能进行测试
void page_decref(struct PageInfo* pp) {
	if (--pp->pp_ref == 0) {
        pmm_free_page(pp);
	}
}

int page_insert(pde_t *pgdir, struct PageInfo *pp, void *va, int perm) {
	pte_t *entry = NULL;
	entry =  pgdir_walk(pgdir, va, 1);              //Get the mapping page of this address va.
	assert(entry != NULL, "can not get the entry of this page.\n");

	pp->pp_ref++;
	if((*entry) & PTE_P) 	                        // If this virtual address is already mapped.
	{
		asm volatile ("invlpg (%0)" : : "a" (va)); // 通知CPU更新页表缓存
		page_remove(pgdir, va);                    // 把原来和va关联的页面信息去掉
	}
	*entry = (pp->physical_addr | perm | PTE_P);   // 发现这一步是需要加上PTE_P的，因为在上一步中，可能已经remove掉了标记
	pgdir[PDX(va)] |= perm;			               // Remember this step!，但是这里是不需要加上述标记的，因为一定存在。

	return 0;
}

struct PageInfo* page_lookup(pde_t *pgdir, void *va, pte_t **pte_store) // 将二级页表指针的存起来
{
	pte_t *entry = NULL;
	struct PageInfo *ret = NULL;

	entry = pgdir_walk(pgdir, va, 0);   // 必须明白walk返回的是所在二级页表的指针，虚拟地址
	if(entry == NULL)                   // 也就是页表目录的那个位置，根本没有指向任何二级页表
		return NULL;
    if(!(*entry & PTE_P))               // 二级页表发现了，但是被指定是无效的。TODO：多余的？
		 return NULL;
    // 先解引用得到的是二级页表的实际的值->然后得到物理地址->通过那个内存的实际位置和物理内存的位置一一对应的关系，找到对应的页面。
	void* phyadd = (PTE_ADDR(*entry));
	uint32_t offset = (phyadd - physical_page_begin) / PGSIZE;
	ret = &pages[offset];
    // 这里得到的是虚拟内存的
	if(pte_store != NULL) {
		*pte_store = entry;
	}
	return ret;
}

void page_remove(pde_t *pgdir, void *va)
{
	pte_t *pte = NULL;
	struct PageInfo *page = page_lookup(pgdir, va, &pte);
	assert(page != NULL, "page find error, page entry doesn't present.\n");

	page_decref(page);
	asm volatile ("invlpg (%0)" : : "a" (va)); // 通知CPU更新页表缓存
	*pte = 0;
}

void init_vmm()
{
    map_region(pgd_kern, UPAGES, (MAX_PAGE_NUM + 1) * sizeof(struct PageInfo), PADDR(pages), 0);
    map_region(pgd_kern, UENVS, ((MAX_ENV_NUM)*sizeof(struct Env)), PADDR(envs), 0); // 这里不用负责PTE_P
    map_region(pgd_kern, KERNBASE, 0xffffffff-KERNBASE, 0, PTE_W);

	uint32_t pgd_kern_phy_addr = (uint32_t)pgd_kern - KERNBASE;
	// 注册页错误中断的处理函数 ( 14 是页故障的中断号 )
	register_interrupt_handler(14, &page_fault);
	switch_pgd(pgd_kern_phy_addr);
    // 现在低地址不能够访问了
}


void switch_pgd(uint32_t pd) {
	asm volatile ("mov %0, %%cr3" : : "r" (pd));
}


