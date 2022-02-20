
#include "console.h"
#include "string.h"
#include "debug.h"
#include "gdt.h"
#include "idt.h"
#include "timer.h"
#include "pmm.h"
#include "vmm.h"
#include "keyboard.h"
#include "sched.h"
#include "entry.h"
#include "shell.h"
#include "heap.h"
#include "syscall.h"

// 内核初始化函数
void kern_init();

// 开启分页机制之后的 Multiboot 数据指针
multiboot_t *glb_mboot_ptr;

// 开启分页机制之后的内核栈
char kern_stack[KERN_STACK_SIZE]; // 这里声明就自动放入高地址代码段的.data区域。
uint32_t kern_stack_top;

// 内核使用的临时页表和页目录
// 该地址必须是页对齐的地址，内存 0-640KB 肯定是空闲的，这里的是放的临时页目录表（其实里面就两个页表项）
// 低地址的页表指针，高地址的页表指针，他们实际的位置是在1,2,3K的位置
__attribute__((section(".init.data"))) pde_t *pde_tmp  = (pde_t *)0x1000;
__attribute__((section(".init.data"))) pde_t *pte_low_0  = (pde_t *)0x2000;
__attribute__((section(".init.data"))) pde_t *pte_hign_0 = (pde_t *)0x3000;

// 上述关键字，的意思就是强制的将上述的三个指针放到低地址的区的代码段，否则会默认放到高地址区

// 内核入口函数
__attribute__((section(".init.text"))) void kern_entry()
{
    // 这时候还没有打开分页机制，因此访问的虚拟地址，就是物理地址，
    // 很简单只有两个，为什么只有两个，因为临时页表很小就行，但是又要符合cpu的MMU单元的工作情况
	pde_tmp[0] = (uint32_t)pte_low_0 | PTE_P | PTE_W; // 每个页表目录项也就是页表页表的大小是4M，因此只需要两个就行了

	pde_tmp[PDX(KERNBASE)] = (uint32_t)pte_hign_0 | PTE_P | PTE_W; // 分别映射高地址和低地址
	// 映射内核虚拟地址 4MB 到物理地址的前 4MB
	int i;
	for (i = 0; i < 1024; i++) {
		pte_low_0[i] = (i << 12) | PTE_P | PTE_W;
	}

	// 想想为什么要映射低地址的区域？假设不映射的话，那么一开开启分页，低地址的虚拟地址就找不到位置了。
	// 但是两个不会发生重叠吗？答案是不会，在ld链接脚本中，我们发现就算把低地址都加上 3G，也是不会重合的
	// 映射 0x00000000-0x00400000 的物理地址实际代表的虚拟地址 0xC0000000-0xC0400000
	for (i = 0; i < 1024; i++) {
		pte_hign_0[i] = (i << 12) | PTE_P | PTE_W;
	}

	// 设置临时页表
	asm volatile ("mov %0, %%cr3" : : "r" (pde_tmp));
	uint32_t cr0;
	// 启用分页，将 cr0 寄存器的分页位置为 1
	asm volatile ("mov %%cr0, %0" : "=r" (cr0));
	cr0 |= 0x80000000;
	asm volatile ("mov %0, %%cr0" : : "r" (cr0));

	// 切换内核栈make
	kern_stack_top = ROUNDDOWN((uint32_t)kern_stack + KERN_STACK_SIZE, 4);
	asm volatile ("mov %0, %%esp\n\t"
			"xor %%ebp, %%ebp" : : "r" (kern_stack_top));

	// because the lower address and higher address points to the same physical address.
	glb_mboot_ptr = mboot_ptr_tmp + KERNBASE;

	// 调用内核初始化函数
    kern_init();
}



void kern_init() {
    init_console();
    init_debug();
    init_gdt();
    init_idt();
    init_pmm();
    init_vmm();
    init_heap();
    init_env();
    init_sched();
    init_keyboard();
    init_timer(100);
    init_system_call();
    enable_intr();
    char *cmd;
    printk("Welcome to Tulk!'K> help' for instructions.\n");
    printk("K> ");
    // fixbug，乱序输出，如果有前台进程正在运行，就不要调度shell程序了。
    for(;;) {

        cmd = getstring();
        parse_cmd(cmd);
        pid_t pid;

        switch (runinfo.cmd_type) {

            case SHELL_JOBS:{
                listing_all_jobs();
                break;
            }

            case SHELL_FG: {
                if (!check_pid(pid = runinfo.tar_pid)) goto done;
                env_map[pid]->bg = false;
                has_front_env = true;
                schedule();
                env_map[pid]->env_status = ENV_RUNNABLE; // 原来可能： 可以运行，或者是阻塞的。
                break;
            }

            case SHELL_STOP: {
                if (!check_pid(pid = runinfo.tar_pid)) goto done;
                env_map[pid]->env_status = ENV_STOPED;
                break;
            }

            case SHELL_RERUN: {
                if (!check_pid(pid = runinfo.tar_pid)) goto done;
                env_map[pid]->env_status = ENV_RUNNABLE;
                break;
            }

            case SHELL_KILL: {
                if (!check_pid(pid = runinfo.tar_pid)) goto done;
                env_map[pid]->env_status = ENV_ZOMBIE;
                break;
            }

            case SHELL_HELP: {
                show_help();
                break;
            }

            default:
                load_icode(runinfo.exe_name, NULL, runinfo.bg); // TOOD：参数传递问题
                break;

        }
done:
    if (!has_front_env)
        printk("K> ");

    }
}

