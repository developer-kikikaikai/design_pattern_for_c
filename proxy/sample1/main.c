#include <stdio.h>
#ifndef PROXY
#include "testif.h"
#else
#include "proxyfunc.h"
#endif

int main() {
	printf("call testfunc(5): result=%d\n", testfunc(5));
	return 0;
}
