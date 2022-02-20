
#include "common.h"
#include "string.h"
#include "elf.h"
#include "vmm.h"
// 如果想深入理解ELF的相关信息，参考这里 http://wiki.osdev.org/ELF

// 这里存放这elf目标段的相关信息
struct Envlocation env_loc[MAX_ENV_NUM];
uint32_t env_loc_counter;

// 从 multiboot_t 结构获取ELF信息，注意因为我们都是在完成分页之后进行操作，因此在这个基础上要加上3G的偏移
elf_t elf_from_multiboot(multiboot_t *mb)
{
	int i;
	elf_t elf;
	elf_section_header_t *sh = (elf_section_header_t *)mb->addr; // addr成员代表的是几个header结构体数组的最开始的地址

	uint32_t shstrtab = sh[mb->shndx].addr; // shndx是和字符串相关的那个header结构体，相对于addr成员的偏移索引，因此这条语句获取了字符串那个header的真正的开始地址
	for (i = 0; i < mb->num; i++) {
		const char *name = (const char *)(shstrtab + sh[i].name) + KERNBASE;
		// 在 GRUB 提供的 multiboot 信息中寻找内核 ELF 格式所提取的字符串表和符号表，两个部分在内存中的开始地址，和长度
        if (strcmp(name, ".text") == 0) continue;
        if (strcmp(name, ".init.text") == 0) continue;
		if (strcmp(name, ".strtab") == 0) {
			elf.strtab = (const char *)sh[i].addr + KERNBASE;
			elf.strtabsz = sh[i].size;
		}
		if (strcmp(name, ".symtab") == 0) {
			elf.symtab = (elf_symbol_t *)(sh[i].addr + KERNBASE);
			elf.symtabsz = sh[i].size;
		}
		if (endswith(name, ".text")) {
            strcpy(&env_loc[env_loc_counter].name, name);
            env_loc[env_loc_counter].address = sh[i].addr;
            env_loc[env_loc_counter].length = sh[i].size;
            env_loc_counter += 1;
		}
	}
	return elf;
}

// 查看ELF的符号信息
const char *elf_lookup_symbol(uint32_t addr, elf_t *elf)
{
	int i;
	for (i = 0; i < (elf->symtabsz / sizeof(elf_symbol_t)); i++) {
		if (ELF32_ST_TYPE(elf->symtab[i].info) != 0x2) { // 不是函数
		      continue;
		}
		// 通过函数调用地址查到函数的名字(地址在该函数的代码段地址区间之内)
		if ( (addr >= elf->symtab[i].value) && (addr < (elf->symtab[i].value + elf->symtab[i].size)) ) {
			return (const char *)((uint32_t)elf->strtab + elf->symtab[i].name);
		}
	}

	return NULL;
}

