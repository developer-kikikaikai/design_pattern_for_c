#include "members.h"

int get_any_people(int *peoples_id) {
	int cnt=0;

	peoples_id[cnt++]=Japan_new();
	peoples_id[cnt++]=US_new();
	peoples_id[cnt++]=Germany_new();
	peoples_id[cnt++]=alpha_complex_new(INFRA_RED);
	peoples_id[cnt++]=alpha_complex_new(ULTRA_VIOLET);
	return cnt;
}
