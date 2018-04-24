#include "area_class.h"
#include "common.h"
int main() {
	//show all first
	area_init();
	area_show_all();

	char name[NAME_MAX];
	printf("[FROM MAIN] check oldest human in Japan\n");
	unsigned int age = japan_get_oldest_resident(name);
	printf("[FROM MAIN] oldest:%s age(%u)\n", name, age);
	int i=0;
	for(i = 0; i < AREA_MAX; i ++ ) {
		printf("[FROM MAIN] area[%d] oldest human:", i);
		age = area_get_oldest_resident(i, name);
		printf("[FROM MAIN] oldest:%s age(%u)\n", name, age);
	}

	//add new men
	area_accept_new_resident(AREA_HOKKAIDO, "ootani san", 20);
	area_accept_new_resident(AREA_KANTO, "matsuzaka san", 34);
	area_accept_new_resident(AREA_KYUSYU, "yanagida san", 25);
	area_accept_new_resident(AREA_KANTO, "Mr, nagashima san", 82);

	area_show_all();

	printf("[FROM MAIN] check oldest human in Japan (maybe nagashima san)\n");
	age = japan_get_oldest_resident(name);
	printf("[FROM MAIN] oldest:%s age(%u)\n", name, age);

	area_exit();
	return 0;
}
