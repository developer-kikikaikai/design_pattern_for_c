#include <stdio.h>
#include "test_state_manager.h"
#include "test_state_machine.h"

int main() {
	if(test_state_manager()) {
		printf("Failed to test state_manager\n");
		return -1;
	}
	printf("Success state_manager test!!\n");

	if(test_state_machine()) {
		printf("Failed to test state_machine\n");
		return -1;
	}
	printf("Success state_machine test!!\n");
	return 0;
}
