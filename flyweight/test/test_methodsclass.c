#include <stdlib.h>
#include <string.h>
#include "flyweight.h"
#include "test.h"

#define NAMELEN 64

//get name by id
#define MEMBER \
	int id;\
	char *name;

struct testclass1 {
	char * (*getname)(struct testclass1 *data);
MEMBER
};

struct testclass1_input {
MEMBER
};

#define COPY_MEMBER(src, dst) \
	(dst)->id=(src)->id;\
	sprintf((dst)->name, (src)->name);

//to check keep function
static char * getname_inplement(struct testclass1 *data) {
	return data->name;
}

//create 2 Flyweight class, to check member and to check ID.
static void constructor_member(void *this, size_t size, void *input_parameter) {
	struct testclass1 * class_instance = (struct testclass1 *)this;
	struct testclass1_input *input = (struct testclass1_input*)input_parameter;
	//only copy member
	class_instance->name = malloc(NAMELEN);

	class_instance->id = input->id;
	sprintf(class_instance->name, "%s", input->name);
	class_instance->getname=getname_inplement;
}

static void destructor_member(void *this) {
	struct testclass1 * class_instance = (struct testclass1 *)this;
	free(class_instance->name);
}

//for only id Flyweight class
static int equall_operand_onlyid(void *this, size_t size, void *input_parameter) {
	struct testclass1 * class_instance = (struct testclass1 *)this;
	struct testclass1_input *input = (struct testclass1_input*)input_parameter;
	return (class_instance->id == input->id);
}

static int set_for_operand_onlyid(void *this, size_t size, void *input_parameter) {
	struct testclass1 * class_instance = (struct testclass1 *)this;
	struct testclass1_input *input = (struct testclass1_input*)input_parameter;
	char * name = (char *)input_parameter;
	sprintf(class_instance->name, "%s", name);
	return 1;
}

static flyweight_methods_t onlyid_method={
	.constructor=constructor_member,
	.equall_operand=equall_operand_onlyid,
	.setter=set_for_operand_onlyid,
	.destructor=destructor_member,
};

//for only operand member
static int equall_operand_member(void *this, size_t size, void *input_parameter) {
	struct testclass1 * class_instance = (struct testclass1 *)this;
	struct testclass1_input *input = (struct testclass1_input*)input_parameter;
	return (class_instance->id == input->id) && (strcmp(class_instance->name, input->name)==0);
}

static flyweight_methods_t member_method={
	.constructor=constructor_member,
	.equall_operand=equall_operand_member,
	.setter=NULL,
	.destructor=destructor_member,
};

//override API for member_method
static char * getname_implement_withset(struct testclass1 *data) {
	static char name[20];
	sprintf(name, "Mr. %s", data->name);
	return name;
}
static int setter_override(void *this, size_t size, void *input_parameter) {
	struct testclass1 * class_instance = (struct testclass1 *)this;
	class_instance->getname = getname_implement_withset;
	return 0;
}

#define TESTDATA_LEN 5
int test_methodsclass(int is_threadsafe) {
	int testcnt=0;
	struct testclass1_input setting[TESTDATA_LEN] = {
		{0,"ootani"},
		{1,"ootani"},
		{2,"sakai"},
		{1,"ootani"},
		{2,"kato"},
	};

	//test1, check operator is check member
	{
	FlyweightFactory handle = flyweight_factory_new(sizeof(struct testclass1), is_threadsafe, &member_method);
	if(!handle) {
		ERRLOG("failed to create handle");
	}

	void *instances[TESTDATA_LEN];
	int i=0;
	for(i=0;i<TESTDATA_LEN;i++) {
		instances[i] = flyweight_get(handle, &setting[i]);
		if(!equall_operand_member(instances[i], sizeof(struct testclass1), &setting[i]) ) {
			ERRLOG("failed to operand == member");
		}
		if(((struct testclass1 *)instances[i])->getname != getname_inplement) {
			ERRLOG("Not initialize interface method")
		}
		struct testclass1 * instance = (struct testclass1 *)instances[i];
		printf("%d name:%s\n", i, instance->getname(instance));
	}

	//check instance, only same id => different, only same name => different, same all => OK
	if(instances[0] == instances[1] || instances[1] == instances[2] ||instances[1] != instances[3]) {
		ERRLOG("Instance usage failed")
	}
	flyweight_factory_free(handle);
	testcnt++;
	}

	//test2, check operator is check only ID
	{
	FlyweightFactory handle = flyweight_factory_new(sizeof(struct testclass1), is_threadsafe, &onlyid_method);
	if(!handle) {
		ERRLOG("failed to create handle");
	}

	void *instances[TESTDATA_LEN];
	int i=0;
	for(i=0;i<TESTDATA_LEN;i++) {
		instances[i] = flyweight_get(handle, &setting[i]);
		if(!equall_operand_onlyid(instances[i], sizeof(struct testclass1), &setting[i]) ) {
			ERRLOG("failed to operand == id");
		}
		if(((struct testclass1 *)instances[i])->getname != getname_inplement) {
			ERRLOG("Not initialize interface method")
		}
		struct testclass1 * instance = (struct testclass1 *)instances[i];
		printf("%d name:%s\n", i, instance->getname(instance));
	}

	//check instance, only same id => OK
	if(instances[0] == instances[1] || instances[1] != instances[3] ||instances[2] != instances[4]) {
		ERRLOG("Instance usage failed")
	}

	//setter use
	flyweight_set(handle, &setting[0], "update name", NULL);
	void *instance=flyweight_get(handle, &setting[0]);
	if(instance != instances[0]) {
		ERRLOG("Instance usage failed")
	}

	struct testclass1 * testclass1_instance = (struct testclass1 *)instance;
	printf("Update, %s\n", testclass1_instance->name);
	if(strcmp(testclass1_instance->name, "update name") != 0) {
		ERRLOG("Failed to set")
	}

	//setter set
	flyweight_set(handle, &setting[0], "update name", setter_override);
	instance=flyweight_get(handle, &setting[0]);
	if(instance != instances[0]) {
		ERRLOG("Instance usage failed")
	}

	testclass1_instance = (struct testclass1 *)instance;
	char *updatename = testclass1_instance->getname(testclass1_instance);
	printf("Update, %s\n", updatename);
	if(strcmp(updatename, "update name") == 0) {
		ERRLOG("Failed to set")
	}
	if(strcmp(updatename, "Mr. update name") != 0) {
		ERRLOG("Failed to set")
	}
	flyweight_factory_free(handle);
	testcnt++;
	}
	
	return testcnt;
}
