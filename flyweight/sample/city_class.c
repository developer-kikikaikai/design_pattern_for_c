#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "flyweight.h"
#include "city_class.h"
#include "human_class.h"
//
struct city_class {
	char name[NAME_MAX];
	int resident_max;//max size
	int resident_num;//num
	int *residents;
};

struct city_resident {
	char name[NAME_MAX];
	unsigned int age;
};

static void city_destructor(void *src) {
ENTER
	struct city_class * city = (struct city_class *)src;
	free(city->residents);
}

static int city_setter_cityclass(void *src, size_t srcsize, void *dist) {
ENTER
	struct city_class * city = (struct city_class *)src;
	memcpy(city, dist, srcsize);
	//allocate residents
	city->residents = calloc(city->resident_max, sizeof(int));
	return 0;
}

static int city_setter_add_resident(void *src, size_t srcsize, void *dist) {
ENTER
	struct city_class * city = (struct city_class *)src;
	struct city_resident * resident = (struct city_resident *)dist;
	city->residents[city->resident_num++] = human_new(resident->name, resident->age);
	return 0;
}

static int city_getter_residents(int city,int **dist) {
	struct city_class * city_class = (struct city_class *) flyweight_get(city);
	memcpy(dist, &city_class->residents, sizeof(int *));
	return city_class->resident_num;
}

static char * city_getter_name(int city) {
	struct city_class * city_class = (struct city_class *) flyweight_get(city);
	return city_class->name;
}

//init city, return city id.
int city_new(char *name, int resident_max) {
ENTER
	struct flyweight_init_s methods={
		NULL,//no constructor
		city_setter_cityclass,
		city_destructor
	};

	int id = flyweight_register_class(sizeof(struct city_class), 0, &methods);

	struct city_class city;
	memset(&city, 0, sizeof(city));
	snprintf(city.name, sizeof(city.name), "%s", name);
	printf("[city] new city: %s\n", city.name);
	city.resident_max = resident_max;
	//set class
	flyweight_set(id, &city, NULL);

	return id;
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
	struct city_class * city_class = (struct city_class *) flyweight_get(city);
	printf("[city] city name[%s]:\n",city_class->name);
	int *residents=city_class->residents;
	int length = city_class->resident_num;;
	int i=0;
	for( i < 0 ; i < length ; i++ ) {
		printf("[city]\tname[%s]:age(%u)\n",human_get_name(residents[i]),  human_get_age(residents[i]));
	}
}
//exit is implemented in desctuctor
