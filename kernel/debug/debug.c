#include "debug.h"

static void print_stack_trace();

static elf_t kernel_elf;

void init_debug()
{
	// 从 GRUB 提供的信息中获取到内核符号表和代码地址信息
	kernel_elf = elf_from_multiboot(glb_mboot_ptr); // 开启分页之后的结构体所在的内存的位置的指针

}

void print_cur_status()
{
	static int round = 0;
	uint16_t reg1, reg2, reg3, reg4;

	// 获取几个段寄存器的数值
	asm volatile ( 	"mov %%cs, %0;"
			"mov %%ds, %1;"
			"mov %%es, %2;"
			"mov %%ss, %3;"
			: "=m"(reg1), "=m"(reg2), "=m"(reg3), "=m"(reg4));

	// 打印当前的运行级别
	// 为什么最后两位是运行级别呢？这个时候实际上我们已经进入了保护模式，保护模式下段寄存器中的值，
	// 是一个selector，这个selector，关于这个寄存器的信息，可以参考这里 http://wiki.osdev.org/Selector
	printk("%d: @ring %d\n", round, reg1 & 0x3);
	printk("%d:  cs = %x\n", round, reg1);
	printk("%d:  ds = %x\n", round, reg2);
	printk("%d:  es = %x\n", round, reg3);
	printk("%d:  ss = %x\n", round, reg4);
	++round;
}

void panic(const char *msg)
{
	printk("*** System panic: %s\n", msg);
	print_stack_trace();
	printk("***\n");

	// 致命错误发生后打印栈信息后停止在这里
	for (;;);
}

void print_stack_trace()
{
	uint32_t *ebp, *eip;
	// 获取ebp寄存器的值
	asm volatile ("mov %%ebp, %0" : "=r" (ebp));
	while (ebp) {
		eip = ebp + 1; // 这个eip就是ebp增加4个字节，存放的是返回地址
		printk("   [0x%x] %s\n", *eip, elf_lookup_symbol(*eip, &kernel_elf));
		ebp = (uint32_t*)*ebp; // 根据汇编语言的规定，ebp指向的位置，存放了老的ebp的值，沿着链向上即可。
	}

	// 为什么最后的时候ebp的值是0的时候就停止了呢？因为我们在boot.s中进入entry之前将ebp设置成了0
}

void show_kernel_info() {
    printk("kernel in memory start: 0x%08X\n", kern_start);
    printk("kernel in memory end:   0x%08X\n", kern_end);
    printk("kernel in memory used:   %d KB\n\n", (kern_end - kern_start) / 1024);
}
