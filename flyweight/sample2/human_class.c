#include "common.h"
#include "flyweight.h"
#include "human_class.h"

struct human_class {
	//private
	char name[NAME_MAX];
	unsigned int age;
};

//init human, return human class id
int human_new(char *name, unsigned int age) {
ENTER
	struct human_class human;
	int id = flyweight_register_class(sizeof(struct human_class), 0, NULL);

	sprintf(human.name, "%s", name);
	human.age = age;
	printf("  [human new human: %s,%d\n", human.name, human.age);
	//it's OK to call memcpy
	flyweight_set(id, &human, NULL);
	return id;
}

//get human name
unsigned int human_get_age(int id) {
	struct human_class * human = (struct human_class *)flyweight_get(id);
	return human->age;
}

char * human_get_name(int id) {
	struct human_class * human = (struct human_class *)flyweight_get(id);
	return human->name;
}
//exit is implemented in desctuctor

