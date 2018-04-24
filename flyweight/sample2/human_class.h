#ifndef HUMAN_CLASS_
#define HUMAN_CLASS_
#define NAME_MAX (64)

//init human, return human class id
int human_new(char *name, unsigned int age);
//get human name
unsigned int human_get_age(int id);
char * human_get_name(int id);
//exit is implemented in desctuctor
#endif
