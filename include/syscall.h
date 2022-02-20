
#include "types.h"

void _exit();

void putchar(char c);

uint32_t getpid();

uint32_t getchar();

char* getstring();

int32_t syscall(int num, uint32_t a1, uint32_t a2, uint32_t a3, uint32_t a4, uint32_t a5);
