#include "common.h"
#include "flyweight.h"
#include "area_class.h"
#include "city_class.h"

#define ENTERLOG ()
struct area_class {
	int cityid[AREA_MAX];
};

static int area_id_g=0;

static int area_init_hokkaido() {
ENTER
	//register for hokkaido
	int id = city_new("hokkaido", 10);
	city_accept_new_resident(id, "shirasaki", 28);
	city_accept_new_resident(id, "tashiro", 29);
	city_accept_new_resident(id, "takekuma", 29);
	city_accept_new_resident(id, "akashi", 32);
	city_accept_new_resident(id, "aoyama", 34);
	return id;
}

static int area_init_kanto() {
ENTER
	int id = city_new("kanto", 10);
	city_accept_new_resident(id, "tanaka", 30);
	city_accept_new_resident(id, "komiyama", 52);
	city_accept_new_resident(id, "yamamoto masa", 52);
	city_accept_new_resident(id, "maru", 28);
	city_accept_new_resident(id, "hukuura", 42);
	city_accept_new_resident(id, "Otosaka Rousselot Tomo Nicholas", 24);
	return id;
}

static int area_init_kyusyu() {
ENTER
	int id = city_new("kyusyu", 10);
	city_accept_new_resident(id,"aoki", 36);
	city_accept_new_resident(id,"terahara", 34);
	city_accept_new_resident(id,"takeda", 25);
	city_accept_new_resident(id,"matayoshi", 27);
	city_accept_new_resident(id,"ishimine", 32);
	return id;
}

//constructor
static void area_constructor(void *src) {
ENTER
	struct area_class * area = (struct area_class *)src;
	area->cityid[AREA_HOKKAIDO] = area_init_hokkaido();
	area->cityid[AREA_KANTO] = area_init_kanto();
	area->cityid[AREA_KYUSYU] = area_init_kyusyu();
}

void area_init() {
ENTER
	//only set constructor and destructor
	struct flyweight_init_s method={
		area_constructor,
		NULL,//no case to set area class
		NULL,//don't need to define destructor if you don't allocate memory ownself
	};

	area_id_g = flyweight_register_class(sizeof(struct area_class), 0, &method);
}

int japan_get_oldest_resident(char *name) {
ENTER
	struct area_class * area = flyweight_get(area_id_g);
	int i = 0;
	int oldest_age=0;
	int current_age=0;
	char current_name[NAME_MAX];
	for(i=0;i<AREA_MAX;i++) {
		current_age = city_get_oldest_resident(area->cityid[i], current_name );
		if(oldest_age < current_age) {
			oldest_age = current_age;
			sprintf(name,"%s", current_name);
		}
	}
	return oldest_age;
}

int area_get_oldest_resident(int areaid, char *name) {
ENTER
	struct area_class * area = flyweight_get(area_id_g);
	return city_get_oldest_resident(area->cityid[areaid], name);
}

void area_accept_new_resident(int areaid, char *name, unsigned int age) {
ENTER
	struct area_class * area = flyweight_get(area_id_g);
	city_accept_new_resident(area->cityid[areaid], name, age);
}

void area_show_all() {
ENTER
	struct area_class * area = (struct area_class *)flyweight_get(area_id_g);
	int i = 0;
	for(i=0;i<AREA_MAX;i++) {
		city_show_all_resident(area->cityid[i]);
	}
}

void area_exit() {
	//don't care allocation, only call exit
	flyweight_exit();
}
