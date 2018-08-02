#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <assert.h>
#include "mement.h"

#define ERRORCASE \
			printf( "###(%d)failed\n", __LINE__);\
			return -1;

static int test_mement_fail_safe() {
	void * data = NULL;
	/*is it dead?*/
	mement_unregister(NULL);
	mement_remember(NULL, &data, 0);
	mement_remember((MementRegister)&data, NULL, 0);

	/*register parameter check*/
	if(mement_register(NULL, sizeof(&data), NULL)) {
		ERRORCASE
	}

	if(mement_register(&data, 0, NULL)) {
		ERRORCASE
	}

	return 0;
}

static void tmp_api(void *ptr, size_t len) {
	(void)ptr;
	(void)len;
}

static int test_mement_default_cb() {
	int data = 10;

	MementRegister mement = mement_register(&data, sizeof(data), NULL);
	if(!mement) {
		ERRORCASE
	}

	data = 5;
	mement_remember(mement, &data, 0);
	if(data != 10) {
		ERRORCASE;
	}

	mement_unregister(mement);

	int data_static = 5;
	mement_method_t method={NULL, NULL, tmp_api};
	mement = mement_register(&data_static, sizeof(data_static), &method);
	if(!mement) {
		ERRORCASE
	}

	data = 10;
	mement_remember(mement, &data, 0);
	if(data != 5) {
		ERRORCASE;
	}
	mement_unregister(mement);
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

static void test_msgdata_constructor(void *instance, void * base, size_t base_length) {
	test_msgdata * msgdata = (test_msgdata *) base;
	test_msgdata * msgdata_instance = (test_msgdata *) instance;

	msgdata_instance->type = msgdata->type;
	/*deep copy data*/
	if(msgdata_instance->type == TYPE_STRING) {
		msgdata_instance->data.string = calloc(1, strlen(msgdata->data.string) + 1);
		sprintf(msgdata_instance->data.string, "%s", msgdata->data.string);
	} else {
		msgdata_instance->data.value = calloc(1, sizeof(int));
		*msgdata_instance->data.value = *msgdata->data.value;
	}
}

static void test_msgdata_free(void * data, size_t data_length) {
	assert(data_length == sizeof(test_msgdata));

	test_msgdata * msgdata = (test_msgdata *) data;
	if(msgdata->type == TYPE_STRING) {
		free(msgdata->data.string);
	} else {
		free(msgdata->data.value);
	}
}

static void test_msgdata_copy(void *broken_data, void * base, size_t base_length) {
	test_msgdata * msgdata = (test_msgdata *) base;
	test_msgdata * broken_msg = (test_msgdata *)broken_data;

	test_msgdata_free(broken_msg, base_length);
	test_msgdata_constructor(broken_msg, msgdata, base_length);
}


static void test_msgdata_int_init(test_msgdata *msgdata, int value) {
	msgdata->type = TYPE_INT;
	msgdata->data.value = calloc(1, sizeof(int));
	*msgdata->data.value = value;
}

static void test_msgdata_string_init(test_msgdata *msgdata, const char* string) {
	msgdata->type = TYPE_STRING;
	msgdata->data.string = calloc(1, strlen(string) + 1);
	sprintf(msgdata->data.string, "%s", string);
}

#define MSG_OK "no problem msg!!"
#define MSG_NG "missing msg!!"

static int test_mement_normally(void) {

	/**deep copy test **/
	test_msgdata basedata_int, basedata_string;
	test_msgdata_int_init(&basedata_int, 10);
	mement_method_t deep_method ={test_msgdata_constructor, test_msgdata_copy, test_msgdata_free};
	MementRegister mement = mement_register(&basedata_int, sizeof(test_msgdata), &deep_method);
	if(!mement) {
		ERRORCASE
	}

	/*same type*/
	*basedata_int.data.value = 20;

	mement_remember(mement, &basedata_int, 0);
	if(basedata_int.type != TYPE_INT || *basedata_int.data.value != 10) {
		ERRORCASE
	}

	/*different type*/
	free(basedata_int.data.value);
	test_msgdata_string_init(&basedata_int, MSG_NG);

	mement_remember(mement, &basedata_int, 0);
	if(basedata_int.type != TYPE_INT || *basedata_int.data.value != 10) {
		ERRORCASE
	}
	mement_unregister(mement);
	free(basedata_int.data.value);

	/*check string type*/
	test_msgdata_string_init(&basedata_string, MSG_OK);
	mement = mement_register(&basedata_string, sizeof(test_msgdata), &deep_method);
	if(!mement) {
		ERRORCASE
	}

	strcpy(basedata_string.data.string, MSG_NG);
	mement_remember(mement, &basedata_string, 0);
	if(basedata_string.type != TYPE_STRING || strcmp(basedata_string.data.string, MSG_OK) != 0 ) {
		ERRORCASE
	}

	/*different type*/
	free(basedata_string.data.string);
	test_msgdata_int_init(&basedata_string, 10);
	mement_remember(mement, &basedata_string, 1);
	if(basedata_string.type != TYPE_STRING || strcmp(basedata_string.data.string, MSG_OK) != 0 ) {
		ERRORCASE
	}
	free(basedata_string.data.string);

	return 0;
}

int main() {
	if(test_mement_fail_safe()) {
		ERRORCASE
	}

	if(test_mement_default_cb()) {
		ERRORCASE
	}

	if(test_mement_normally()) {
		ERRORCASE
	}

	printf("Success all test!!\n");
	return 0;
}
