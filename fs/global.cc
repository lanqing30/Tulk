
#include "global.h"

void panic(const char* msg) {
    printf("Some thing goes wrong %s\n", msg);
    while(1);
}
