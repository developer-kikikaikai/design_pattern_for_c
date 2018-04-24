#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "common.h"
#include "flyweight.h"
#include "cover_up_the_fact_city.h"
#include "human_class.h"

struct city_class {
	char name[NAME_MAX];
	int resident_max;//max size
	int resident_num;//num
	int *residents;//real residents, 
	int *dummy_residents;//dumy num, 
	int smuggling_resident_num;//smuggling cnt, they are private member
};

#define MAX 5
static int cover_up_city_number_of_People_smuggling_Inc (void){
	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);
	int poeople_num = ts.tv_nsec%MAX;
	return poeople_num;
}

#define ASCII_MIN (46)
#define ASCII_MAX (122)

#define ASCII_RANDOM(seed) ((seed%(ASCII_MAX-ASCII_MIN))+ASCII_MIN)

static void cover_up_city_name_of_People_smuggling_Inc (char *name) {
	int i=0;
	struct timespec ts;
	for(i=0;i<10;i++) {
		clock_gettime(CLOCK_REALTIME, &ts);
		name[i]=ASCII_RANDOM(ts.tv_nsec);
	}
}

static void city_destructor(void *src) {
ENTER
	struct city_class * city = (struct city_class *)src;
	printf("[cover_up_city] city: %s\n", city->name);
	printf("[cover_up_city] real residents\n");
	int i=0;
	for(i=0;i<city->resident_num;i++) {
		printf("[cover_up_city]\tname[%s]:age(%u)\n",human_get_name(city->residents[i]),  human_get_age(city->residents[i]));
	}
	free(city->residents);
	free(city->dummy_residents);
}

static int city_setter_cityclass(void *src, size_t srcsize, void *dist) {
ENTER
	struct city_class * city = (struct city_class *)src;
	memcpy(city, dist, srcsize);
	//allocate residents
	city->residents = calloc(city->resident_max, sizeof(int));
	city->dummy_residents = calloc(city->resident_max-city->smuggling_resident_num, sizeof(int));
	int i=0;
	for(i=0;i<city->smuggling_resident_num;i++ ) {
		char name[NAME_MAX]={0};
		cover_up_city_name_of_People_smuggling_Inc(name);
		city->residents[i]=human_new(name, 20);
	}
	city->resident_num=i;
	return 0;
}

int city_setter_add_resident(void *src, size_t srcsize, void *dist) {
ENTER
	struct city_class * city = (struct city_class *)src;
	struct city_resident * resident = (struct city_resident *)dist;
	city->residents[city->resident_num] = human_new(resident->name, resident->age);
	city->resident_num++;
	return 0;
}

int city_getter_residents(int city,int **dist) {
	struct city_class * city_class = (struct city_class *) flyweight_get(city);
	int slide=city_class->smuggling_resident_num;
	int size=(city_class->resident_num - slide);
	memcpy(city_class->dummy_residents, &city_class->residents[slide], sizeof(int)*size);
	memcpy(dist, &city_class->dummy_residents, sizeof(int *));
	return size;
}

char * city_getter_name(int city) {
	struct city_class * city_class = (struct city_class *) flyweight_get(city);
	return city_class->name;
}

//init city, return city id.
int cover_up_city_new(char *name, int resident_max) {
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
	//add People smuggling inc
	city.smuggling_resident_num = cover_up_city_number_of_People_smuggling_Inc();
	city.resident_max = resident_max + city.smuggling_resident_num;
	//set class
	flyweight_set(id, &city, NULL);

	return id;
}

