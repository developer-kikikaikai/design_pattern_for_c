#include<stdio.h>
int test_normalclass(int);//test normal class
int test_methodsclass(int);//test class, set methods

#define ERRLOG(...) printf("####<%s:%s:%d> error", __FILE__, __FUNCTION__, __LINE__);\
	printf(__VA_ARGS__);printf("\n");\
	return 0;
