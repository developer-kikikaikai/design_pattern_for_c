#include "flyweight.h"
#include "test.h"
int main() {
	int totalcnt=0;
	int cnt=0;
	cnt = test_normalclass(0);
	if(cnt == 0) {
		ERRLOG("test normal class unthreadsafe error\n")
	}
	totalcnt+=cnt;

	cnt = test_methodsclass(0);
	if(cnt == 0) {
		ERRLOG("test method class unthreadsafe error\n")
	}
	totalcnt+=cnt;

	cnt = test_normalclass(1);
	if(cnt == 0) {
		ERRLOG("test normal class threadsafe error\n")
	}
	totalcnt+=cnt;

	cnt = test_methodsclass(1);
	if(cnt == 0) {
		ERRLOG("test normal class threadsafe error\n")
	}
	totalcnt+=cnt;
	printf("Finish all test %d\n", totalcnt);
	return 0;
}
