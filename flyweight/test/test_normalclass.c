#include <stdlib.h>
#include <string.h>
#include "flyweight.h"
#include "test.h"

struct testclass1 {
	int data;
};

int test_normalclass(int is_threadsafe) {
	int testcnt=0;
	struct testclass1 data1={.data=0};
	struct testclass1 data2_different1={.data=1};

	//test1, create handle
	FlyweightFactory handle = flyweight_factory_new(sizeof(struct testclass1), is_threadsafe, NULL);
	if(!handle) {
		ERRLOG("failed to create handle");
	}
	testcnt++;

	//test2, get instance
	{
	void * data1_instance = flyweight_get(handle, &data1);
	if(!data1_instance || (memcmp(&data1, data1_instance, sizeof(struct testclass1)) != 0) ) {
		ERRLOG("failed to create instance");
	}
	void * data1_instance_twice = flyweight_get(handle, &data1);
	if(data1_instance != data1_instance_twice) {
		ERRLOG("failed to reuse same data");
	}
	testcnt++;
	}

	{
	void * data2_instance = flyweight_get(handle, &data2_different1);
	if(!data2_instance || (memcmp(&data2_different1, data2_instance, sizeof(struct testclass1)) != 0 )) {
		ERRLOG("failed to create instance");
	}
	void * data1_instance = flyweight_get(handle, &data1);
	if(data1_instance == data2_instance) {
		ERRLOG("failed to separate data");
	}
	testcnt++;
	}

	{
	//set API
	struct testclass1 data1_cpy=data1;
	void * data1_instance = flyweight_get(handle, &data1);
	if(!data1_instance) {
		ERRLOG("failed to get instance");
	}
	void * data1_instance_cpy = flyweight_get(handle, &data1_cpy);
	if(data1_instance != data1_instance_cpy) {
		ERRLOG("failed to reuse instance");
	}
	//change instance data to 2
	data1_cpy.data=2;
	flyweight_set(handle, &data1, &data1_cpy, NULL);

	data1_instance_cpy = flyweight_get(handle, &data1_cpy);
	if(data1_instance != data1_instance_cpy || (memcmp(&data1_cpy, data1_instance_cpy,sizeof(struct testclass1)) != 0) ) {
		ERRLOG("failed to set value");
	}
	data1_instance = flyweight_get(handle, &data1);
	if(!data1_instance || data1_instance == data1_instance_cpy || (memcmp(&data1, data1_instance,sizeof(struct testclass1)) != 0) ) {
		ERRLOG("failed to get new value");
	}
	testcnt++;
	}

	{
	if(flyweight_factory_new(0,0,NULL) != NULL) {
		ERRLOG("fail safe check error");
	}
	if(flyweight_get(NULL,NULL) != NULL) {
		ERRLOG("fail safe check error");
	}
	if(flyweight_get(handle,NULL) == NULL) {
		ERRLOG("fail safe accept NULL constructor");
	}
	if(flyweight_set(NULL,NULL, NULL, NULL) != -1) {
		ERRLOG("fail safe check error");
	}
	if(flyweight_set(handle,NULL, NULL, NULL) != -1) {
		ERRLOG("fail safe check error");
	}
	testcnt++;
	}
	//exit

	flyweight_factory_free(NULL);
	flyweight_factory_free(handle);
	return testcnt;
}
