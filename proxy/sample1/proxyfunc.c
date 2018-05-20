#include <stdio.h>
#include "proxyfunc.h"

int testfunc_(const char *filename, const char *funcname, const int line, int data) {
	printf("[%s:%d(%s)] enter\n", filename, line, funcname);
	return data*10;
}
