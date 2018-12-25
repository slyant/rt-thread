#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

int main(void)
{
    printf("hello rt-thread\n");

	extern int sqlite3_initialize(void);
	int r0 = sqlite3_initialize();
	rt_kprintf("r0:%d\n", r0);
    return 0;
}

