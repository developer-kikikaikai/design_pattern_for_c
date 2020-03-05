#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "prototype.h"

#define ERRORCASE \
			printf( "###(%d)failed\n", __LINE__);\
			return -1;

static int test_prototype_fail_safe() {
	void * data = NULL;
	prototype_manager_free(NULL);
	if(prototype_register(NULL, &data, sizeof(&data), NULL)) {
		ERRORCASE
	}

	PrototypeManager manager = prototype_manager_new(0);
	if(!manager) {
		ERRORCASE
	}

	if(prototype_register(manager, NULL, sizeof(&data), NULL)) {
		ERRORCASE
	}
	if(prototype_register(manager, &data, 0, NULL)) {
		ERRORCASE
	}

	prototype_unregister(manager, NULL);
	prototype_unregister(NULL, (PrototypeFactory)&data);

	if(prototype_clone(NULL)){
		ERRORCASE
	}

	prototype_free((PrototypeFactory)&data, NULL);
	prototype_free(NULL, &data);

	prototype_manager_free(manager);

	return 0;
}

static void tmp_api(void *ptr) {
}

static int test_prototype_default_cb() {
	PrototypeManager manager = prototype_manager_new(0);
	if(!manager) {
		ERRORCASE
	}

	int * data = malloc(sizeof(*data));
	*data = 10;

	PrototypeFactory factory = prototype_register(manager, data, sizeof(*data), NULL);
	if(!factory) {
		ERRORCASE
	}

	int *data_new = prototype_clone(factory);
	if(!data_new || *data_new != 10) {
		ERRORCASE;
	}

	prototype_free(factory, data_new);
	prototype_unregister(manager, factory);

	int data_static = 5;
	prototype_factory_method_t method={NULL, NULL, tmp_api};
	factory = prototype_register(manager, &data_static, sizeof(data_static), &method);
	if(!factory) {
		ERRORCASE
	}
	data_new = prototype_clone(factory);
	if(!data_new || *data_new != 5) {
		ERRORCASE;
	}
	prototype_free(factory, data_new);

	prototype_manager_free(manager);
	return 0;
}

#define TYPE_STRING (0)
#define TYPE_INT (1)
typedef struct test_msgdata{
	int type;
	union {
		char * string;
		int * value;
	} data;
}test_msgdata;

static void * test_msgdata_deep_copy(void * base, size_t base_length) {
	test_msgdata * msgdata = (test_msgdata *) base;
	void * clone_data = calloc(1, base_length);
	test_msgdata * clone_msg = (test_msgdata *)clone_data;

	clone_msg->type = msgdata->type;
	/*deep copy data*/
	if(clone_msg->type == TYPE_STRING) {
		clone_msg->data.string = calloc(1, strlen(msgdata->data.string) + 1);
		sprintf(clone_msg->data.string, "%s", msgdata->data.string);
	} else {
		clone_msg->data.value = calloc(1, sizeof(int));
		*clone_msg->data.value = *msgdata->data.value;
	}

	return clone_data;
}

static void test_msgdata_deep_free(void * data) {
	test_msgdata * msgdata = (test_msgdata *) data;
	if(msgdata->type == TYPE_STRING) {
		free(msgdata->data.string);
	} else {
		free(msgdata->data.value);
	}
	free(msgdata);
}

static void test_msgdata_int_init(test_msgdata **data, int value) {
	test_msgdata *msgdata = calloc(1, sizeof(test_msgdata));
	msgdata->type = TYPE_INT;
	msgdata->data.value = calloc(1, sizeof(int));
	*msgdata->data.value = value;
	*data = msgdata;
}

static void test_msgdata_string_init(test_msgdata **data, const char* string) {
	test_msgdata *msgdata = calloc(1, sizeof(test_msgdata));
	msgdata->type = TYPE_STRING;
	msgdata->data.string = calloc(1, strlen(string) + 1);
	sprintf(msgdata->data.string, "%s", string);
	*data = msgdata;
}

static int test_prototype_base(PrototypeManager manager) {

	/**shallow copy test **/
	test_msgdata *basedata_int, *basedata_string;
	test_msgdata_int_init(&basedata_int, 10);
	test_msgdata_string_init(&basedata_string, "shallow_copy_base");
	prototype_factory_method_t shallow_method ={NULL, NULL, test_msgdata_deep_free};

	PrototypeFactory shallow_int_factory = prototype_register(manager, basedata_int, sizeof(test_msgdata), &shallow_method);
	if(!shallow_int_factory) {
		ERRORCASE
	}
	PrototypeFactory shallow_string_factory = prototype_register(manager, basedata_string, sizeof(test_msgdata), &shallow_method);
	if(!shallow_string_factory) {
		ERRORCASE
	}

	/*clone shallow copy int*/
	{
	test_msgdata * clone_data = (test_msgdata *)prototype_clone(shallow_int_factory);
	/*because shallow copy, pointer is same*/
	if(!clone_data || memcmp(clone_data, basedata_int, sizeof(test_msgdata)) != 0) {
		ERRORCASE
	}
	printf("shallow copy value:%d, change base value\n", *clone_data->data.value);
	*basedata_int->data.value = 5;
	printf("after change base value, shallow copy value:%d\n",  *clone_data->data.value);
	if(*clone_data->data.value != 5) {
		ERRORCASE
	}
	prototype_free(shallow_int_factory, clone_data);
	*basedata_int->data.value = 10;
	}
	prototype_unregister(manager, shallow_int_factory);

	/*clone shallow copy string*/
	{
	test_msgdata * clone_data = (test_msgdata *)prototype_clone(shallow_string_factory);
	/*because shallow copy, pointer is same*/
	if(!clone_data || memcmp(clone_data, basedata_string, sizeof(test_msgdata)) != 0) {
		ERRORCASE
	}
	printf("shallow copy value:%s, change base value\n", clone_data->data.string);
	snprintf(basedata_string->data.string, strlen(basedata_string->data.string), "change!");
	printf("after change base value, shallow copy value:%s\n", clone_data->data.string);
	if(strcmp(clone_data->data.string, "change!") != 0) {
		ERRORCASE
	}
	prototype_free(shallow_string_factory, clone_data);
	}
	prototype_unregister(manager, shallow_string_factory);


	/**deep copy test **/
	test_msgdata_int_init(&basedata_int, 10);
	test_msgdata_string_init(&basedata_string, "deep_copy_base");
	prototype_factory_method_t deep_method ={test_msgdata_deep_copy, test_msgdata_deep_free, test_msgdata_deep_free};
	PrototypeFactory deep_int_factory = prototype_register(manager, basedata_int, sizeof(test_msgdata), &deep_method);
	if(!deep_int_factory) {
		ERRORCASE
	}
	PrototypeFactory deep_string_factory = prototype_register(manager, basedata_string, sizeof(test_msgdata), &deep_method);
	if(!deep_string_factory) {
		ERRORCASE
	}

	/*clone deep copy int*/
	{
	test_msgdata * clone_data = (test_msgdata *)prototype_clone(deep_int_factory);
	/*because deep copy, pointer is different, but value is same*/
	if(!clone_data || memcmp(clone_data, basedata_int, sizeof(test_msgdata)) == 0 || (clone_data->type != basedata_int->type || *clone_data->data.value != *basedata_int->data.value) ) {
		ERRORCASE
	}
	printf("deep copy value:%d, change base value\n", *clone_data->data.value);
	*basedata_int->data.value = 5;
	printf("after change base value, deep copy value:%d\n", *clone_data->data.value);
	if(*clone_data->data.value != 10) {
		ERRORCASE
	}
	prototype_free(deep_int_factory, clone_data);
	*basedata_int->data.value = 10;
	}
	prototype_unregister(manager, deep_int_factory);

	/*clone shallow copy string*/
	{
	test_msgdata * clone_data = (test_msgdata *)prototype_clone(deep_string_factory);
	/*because shallow copy, pointer is same*/
	if(!clone_data || memcmp(clone_data, basedata_string, sizeof(test_msgdata)) == 0 || (clone_data->type != basedata_string->type || strcmp(clone_data->data.string, basedata_string->data.string) != 0) ) {
		ERRORCASE
	}
	printf("deep copy value:%s, change base value\n", clone_data->data.string);
	snprintf(basedata_string->data.string, strlen(basedata_string->data.string), "change!");
	printf("after change base value, dee; copy value:%s\n", clone_data->data.string);
	if(strcmp(clone_data->data.string, "change!") == 0) {
		ERRORCASE
	}
	prototype_free(deep_string_factory, clone_data);
	}
	prototype_unregister(manager, deep_string_factory);

	return 0;
}

static int test_prototype_single_thread() {
	PrototypeManager manager = prototype_manager_new(0);
	if(!manager) {
		ERRORCASE
	}

	if(test_prototype_base(manager)) {
		ERRORCASE
	}

	prototype_manager_free(manager);
	return 0;
}

struct testdata {
	pthread_t tid;
	PrototypeManager manager;
	int result;
};

static void * test_thread(void * arg) {
	struct testdata *testdata = (struct testdata *)arg;
	testdata->result = test_prototype_base(testdata->manager);
	pthread_exit(NULL);
	return NULL;
}

static int test_prototype_multi_thread() {
	PrototypeManager manager = prototype_manager_new(1);
	if(!manager) {
		ERRORCASE
	}

	struct testdata testdata[3];
	int i=0;
	for(i=0;i<sizeof(testdata)/sizeof(testdata[0]);i++) {
		testdata[i].manager = manager;
		testdata[i].result = 0;
		pthread_create(&testdata[i].tid, NULL, test_thread, &testdata[i]);
	}

	int result=0;
	for(i=0;i<sizeof(testdata)/sizeof(testdata[0]);i++) {
		pthread_join(testdata[i].tid, NULL);
		result+=testdata[i].result;
	}

	prototype_manager_free(manager);
	return result;
}

int main(int argc, char *argv[]) {
	if(test_prototype_fail_safe()) {
		ERRORCASE
	}

	if(test_prototype_default_cb()) {
		ERRORCASE
	}

	if(test_prototype_single_thread()) {
		ERRORCASE
	}

	if(test_prototype_multi_thread()) {
		ERRORCASE
	}

	printf("Success all test!!\n");
	return 0;
}
