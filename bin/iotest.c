#include "types.h"
#include "syscall.h"

MODULE("io") void ioentry(void*argv) { iomain(); _exit(); }

MODULE("io") int iomain(void*argv) {
    char c;
    c = getchar();
    putchar(c);
    char*s;
    s = getstring();
    putstring(s);
	return 0;
}
