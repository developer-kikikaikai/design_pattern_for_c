#ifndef COVER_UP_THE_FACT_CITY_
#define CITY_CLASS_

//city class structure
struct city_resident {
	char name[NAME_MAX];
	unsigned int age;
};

int cover_up_city_new(char *name, int resident_max);

//setter when accept new resident
int city_setter_add_resident(void *src, size_t srcsize, void *dist);
//get city name
char * city_getter_name(int city);
//get residents information 
int city_getter_residents(int city,int **dist);
//exit is implemented in desctuctor
#endif
