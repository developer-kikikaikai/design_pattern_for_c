#include "interviewer.h"
#include <stdio.h>
#include <stdlib.h>

#define LENGTH(array) (sizeof(array)/sizeof(array[0]))

static Interviewer interviewer_infra_red_new() {
	printf("インフラレッドに質問機会など与えられません。\n");
	exit(1);
}

static Interviewer get_interviewer_by_computer(char *argv[]) {
	Answer (*answer_new[])() = {answer_infra_red_new, answer_red_new, answer_blue_new};
	int answer_color=atoi(argv[1]);
	if(LENGTH(answer_new)<answer_color) answer_color = LENGTH(answer_new) - 1;

	Interviewer (*interviewer_new[])(Answer) = {interviewer_infra_red_new, interviewer_red_new, interviewer_blue_new};
	int interviewer_color=atoi(argv[2]);
	if(LENGTH(interviewer_new)<interviewer_color) interviewer_color= LENGTH(interviewer_new) - 1;

	/*create answer class from input type*/
	Answer answer_instance = answer_new[answer_color]();

	/*create interviewer command class with answer*/
	return interviewer_new[interviewer_color](answer_instance);
}

int main(int argc, char *argv[]) {
	if(argc<3) return -1;

	/*create interviewer command class with answer*/
	Interviewer interviewer_instance = get_interviewer_by_computer(argv);

	interviewer_instance->interview();
	return 0;
}
