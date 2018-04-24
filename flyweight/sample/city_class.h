#ifndef CITY_CLASS_
#define CITY_CLASS_

//init city, return city class id.
int city_new(char *name, int resident_max);
//come new resident! accect him(her)!
void city_accept_new_resident(int city, char *name, unsigned int age);
//get oldest human name and age(return age)
unsigned int city_get_oldest_resident(int city, char * name);
//show all
void city_show_all_resident(int city);
//exit is implemented in desctuctor
#endif
