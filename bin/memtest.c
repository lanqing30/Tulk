#include "types.h"
#include "syscall.h"

MODULE("mem") void mementry() { memmain(); _exit(); }

MODULE("mem") int memmain() {

	void* target;
	target = malloc(120);
    free(target);
	return 0;

}
