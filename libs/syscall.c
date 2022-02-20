
#include "syscall.h"


void _exit() { syscall(SYS_destroy, 0, 0, 0, 0, 0); }// 让内核销毁当前进程

void putchar(char c) { syscall(SYS_cputc, c, 0, 0, 0, 0); }

uint32_t getpid() { return syscall(SYS_getenvid, 0, 0, 0, 0, 0); }


static char usr_buff[80];

// TOOD：这里只能再想一种方法了，因为不支持嵌套中断，因此只能将循环放在外面，因此输入过快的话，将回遗漏数据。
uint32_t getchar() {
    while (1) {
        uint32_t tmp = syscall(SYS_cgetc, 0, 0, 0, 0, 0);
        if (tmp != 0) {
            return tmp;
        }
    }
}

char* getstring() {
    uint32_t c;
    uint32_t cnt = 0;
    while (1) {
        c = syscall(SYS_cgetc, 0, 0, 0, 0, 0);
        if (c == 0) continue;
        if (c == '\n') {
            usr_buff[cnt++] = '\0';
            return usr_buff;
        } else if (c == '\b'){
            if (cnt > 0) cnt--;
        } else {
            usr_buff[cnt++] = c;
        }
    }
}

// TODO: 实现用户级别的格式化输出
void putstring(const char* s) {
    while(s && *s) {
        putchar(*s++);
    }
}

void* malloc(uint32_t size) {
    return (void*)syscall(SYS_malloc, size, 0, 0, 0, 0);
}

void free(void* location) {
    syscall(SYS_free, (uint32_t)location, 0, 0, 0, 0);
}

inline int32_t syscall(int num, uint32_t a1, uint32_t a2, uint32_t a3, uint32_t a4, uint32_t a5)
{
	int32_t ret;
	// Generic system call: pass system call number in AX,
	// up to five parameters in DX, CX, BX, DI, SI.
	// Interrupt kernel with T_SYSCALL.

	// The "volatile" tells the assembler not to optimize
	// this instruction away just because we don't use the
	// return value.

	// The last clause tells the assembler that this can
	// potentially change the condition codes and arbitrary
	// memory locations.

	asm volatile("int %1\n"
		: "=a" (ret)
		: "i" (255),
		  "a" (num), // eax存放的是系统调用的号码
		  "d" (a1),  // 最多五个参数
		  "c" (a2),
		  "b" (a3),
		  "D" (a4),
		  "S" (a5)
		: "cc", "memory");
    // 返回值放在寄存器eax中
	return ret;
}



