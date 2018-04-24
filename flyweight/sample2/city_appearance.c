#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "flyweight.h"
#include "city_class.h"
#include "cover_up_the_fact_city.h"
#include "human_class.h"

//init city, return city id.
int city_new(char *name, int resident_max) {
ENTER
	return cover_up_city_new(name, resident_max);
}

//come new resident! accect him(her)!
void city_accept_new_resident(int city, char *name, unsigned int age) {
ENTER
	struct city_resident resident;
	char *cityname = city_getter_name(city);
	printf("[city] come %s in city: %s\n", name, cityname);
	snprintf(resident.name, sizeof(resident.name), "%s", name);
	resident.age = age;
	//try to use setter!
	flyweight_set(city, &resident, city_setter_add_resident);
}

//get oldest human name and age(return age)
unsigned int city_get_oldest_resident(int city, char * name) {
ENTER
	int *residents;
	unsigned int oldest_age=0;
	unsigned int cur_age=0;
	int cur_residents_num = city_getter_residents(city, &residents);
	int i=0;
	for( i < 0 ; i < cur_residents_num ; i++ ) {
		cur_age = human_get_age(residents[i]);
		if(oldest_age < cur_age) {
			oldest_age = cur_age;
			snprintf(name, NAME_MAX, "%s", human_get_name(residents[i]) );
		}
	}
	return oldest_age;
}

//show all
void city_show_all_resident(int city) {
ENTER
	printf("[city] city name[%s]:\n",city_getter_name(city));
	int *residents;
	int length = city_getter_residents(city, &residents);
	int i=0;
	for( i < 0 ; i < length ; i++ ) {
		printf("[city]\tname[%s]:age(%u)\n",human_get_name(residents[i]),  human_get_age(residents[i]));
	}
}
//exit is implemented in desctuctor
