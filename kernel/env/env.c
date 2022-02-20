
#include "gdt.h"
#include "pmm.h"
#include "vmm.h"
#include "string.h"
#include "debug.h"
#include "env.h"

// TOOD：遇到了一个问题，在进程切换的时候不能改变页表，因此这里先实现了内核的进程，用户进程以后再实现

static uint16_t free_env_stack_top;

uint16_t env_counter;
struct Env envs[MAX_ENV_NUM];
struct Env* env_map[MAX_ENV_NUM];                // 如何通过进程的id找到一个进程的指针
struct Env* free_env_stack[MAX_ENV_NUM];
struct Env* cur_env;                             // 等待被调度的进程链表，双指针链表
uint32_t kern_env_stack[MAX_ENV_NUM];            // 内核进程程栈栈底的映射位置，因为没有实现用户进程不得不这样做
bool has_front_env = false;

//============用户进程使用==============
//static int env_setup_vm(struct Env *e)
//{
//	int i;
//	struct PageInfo *p = NULL;
//	if (!(p = pmm_alloc_page(1))) // 新建的这一页用作pgdir
//		return -E_NO_MEM;
//
//	e->env_pgdir = (pde_t *)KADDR(p->physical_addr);
//
//	//在虚拟空间中UTOP下面的位置，先不要做任何的映射，实际上在内核中，也是没有进行任何的映射的．
//	for(i = 0; i < PDX(UTOP); i++) {
//		e->env_pgdir[i] = 0;
//	}
//
//	//在UTOP以上的位置，我们直接采用内核的映射方式，
//	for(i = PDX(UTOP); i < NPDENTRIES; i++) {
//		e->env_pgdir[i] = pgd_kern[i];
//	}
//	return 0;
//}

static void region_alloc(struct Env *e, void *va, size_t len)
{
	void* start = (void *)ROUNDDOWN((uint32_t)va, PGSIZE);  // 注意这里的对齐方式，一个是向上对齐，一个是向下对齐
	void* end = (void *)ROUNDUP((uint32_t)va+len, PGSIZE);

	struct PageInfo *p = NULL;
	void* i;
	int r;
	for(i=start; i<end; i+=PGSIZE) {
		p = pmm_alloc_page(1);
		if(p == NULL)
			panic("region alloc, allocation failed.");
		r = page_insert(pgd_kern, p, i, PTE_W);             //　TODO: 直接使用的内核页表目录
		if(r != 0) {
			panic("region alloc error");
		}
	}
}

void init_env()
{
    int i;
    for (i=0; i<MAX_ENV_NUM; i++) {
        memset(&envs[i], 0, sizeof(struct Env));
        free_env_stack[free_env_stack_top++] = &envs[i];
    }
    for (i=0; i<MAX_ENV_NUM; i++) {
        kern_env_stack[i] = USTACKTOP-PGSIZE * 16 *(i + 1);
    }

}


int env_alloc(struct Env **newenv_store, envid_t parent_id) {
	struct Env *e;
    assert(free_env_stack_top != 0, "no free env space.\n");
    e = free_env_stack[--free_env_stack_top];
	*newenv_store = e;
	return 0;
}

void env_free(struct Env* e) {
    env_map[e->env_id] = NULL;
    // 释放栈的内存空间，4K×16每个
    int i;
    for (i=0; i<16; i++)
        page_remove(pgd_kern, (void*)kern_env_stack[e->env_id] + PGSIZE * i);
    memset(e, 0, sizeof(struct Env));
    free_env_stack[free_env_stack_top++] = e;
}


void load_icode(const char* module_name, void*argv, bool bg) {
    struct Env* new_env;
    int ret;
    int tar_idx;
    ret = env_alloc(&new_env, 0);           // TODO: 最后的时候，错误检查，优化
    assert(ret == 0, "page_alloc_error");

    for (tar_idx=0; tar_idx<env_loc_counter; tar_idx ++) {
        if (strcmp(env_loc[tar_idx].name, module_name)==0) break;
    }

    if (tar_idx == env_loc_counter) {
        printk("Error: can not locate %s\n", module_name);
        return;
    }

    void* from = (void*)env_loc[tar_idx].address;
    // uint32_t sz = env_loc[tar_idx].length;

    // 设置一些进程的初始默认状态
    memset(&new_env->context, sizeof(struct context_t), 0);
    new_env->env_id = env_counter;
    env_map[env_counter] = new_env;
    env_counter ++;
    new_env->env_parent_id = 0;
    new_env->env_type = ENV_TYPE_KERN;
    new_env->env_status = ENV_RUNNABLE;
    new_env->env_runs = 0;
    new_env->bg = bg;
    printk("-->> New process %04d created. \n", new_env->env_id);
/** TODO : why bug? */
//    uint32_t *stack_bottom = (uint32_t *)(USTACKTOP - KERN_STACK_SIZE * (new_env->env_id + 1));
//    uint32_t *stack_top = (uint32_t *)(stack_bottom + (KERN_STACK_SIZE >> 1));
//    // 开辟一个栈的空间
//    region_alloc(new_env, (void*)stack_bottom, KERN_STACK_SIZE);
//    *(--stack_top) = 0;                          // 参数省略
//    *(--stack_top) = (uint32_t)destroy;           // 结束后的回调函数
//    *(--stack_top) = (uint32_t)(from);             // 本模块函数开始的位置
//    new_env->context.esp = --stack_top;

    region_alloc(new_env,(void *)(USTACKTOP-PGSIZE * 16 *(new_env->env_id + 1)), PGSIZE * 16);
    uint32_t *stack_top = (uint32_t *)(USTACKTOP-PGSIZE * 16 *(new_env->env_id + 1) + KERN_STACK_SIZE);
    *(--stack_top) = (uint32_t)argv;
    *(--stack_top) = (uint32_t)0;
    *(--stack_top) = (uint32_t)from;
    new_env->context.esp = USTACKTOP-PGSIZE * 16 *(new_env->env_id + 1) + KERN_STACK_SIZE - sizeof(uint32_t) * 3;
    new_env->context.eflags = 0x200;            // 设置可中断，很重要

    // 插入到链表的最后
    new_env->next_env = cur_env;

    struct Env* tail;
    tail = cur_env;
    while(tail->next_env != cur_env) {
        tail = tail->next_env;
    }

    tail->next_env = new_env;
}




