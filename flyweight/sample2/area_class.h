#ifndef AREA_CLASS_
#define AREA_CLASS_

enum JAPAN {
	AREA_HOKKAIDO,
	AREA_KANTO,
	AREA_KYUSYU,
	AREA_MAX,
};

//init area
void area_init();
//get oldest human name and age(return age)
int japan_get_oldest_resident(char *name);
//get oldest human name and age(return age) in this area
int area_get_oldest_resident(int areaid, char *name);
//come new resident!
void area_accept_new_resident(int areaid, char *name, unsigned int age);
//show all residents
void area_show_all();
void area_exit();
#endif
