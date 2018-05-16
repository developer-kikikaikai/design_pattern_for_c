#include <stdio.h>
#include "state_manager.h"

static int test(void *arg) {
	return -1;
}

int main() {
	state_info_t state1=STATE_MNG_SET_INFO_INIT(1, test);
	printf("state1: [state:%d] [name:%s]\n", state1.state, state1.name);
	state_info_t state2;
	STATE_MNG_SET_INFO(state2, 1, test);
	printf("state1: [state:%d] [name:%s]\n", state2.state, state2.name);
	return 0;
}
