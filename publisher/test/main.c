#include <stdio.h>
#include <string.h>
#include "publisher.h"
#include "dp_util.h"

typedef struct testdata {
	int notify1_cnt;
	int notify2_cnt;
	int notify3_cnt;
} testdata_t, *TestData;

#define NTYPE(slide) (0x01)<<(slide)

static void test_notify1(int publish_type, void * detail, void *ctx);
static void test_notify2(int publish_type, void * detail, void *ctx);
static void test_notify3(int publish_type, void * detail, void *ctx);

static void test_notify_with_ctx(int publish_type, void * detail, void *ctx);

static void test_notify1(int publish_type, void * detail, void *ctx) {
	printf("%s, %d\n", __FUNCTION__, publish_type);
	TestData testdata = (TestData) detail;
	testdata->notify1_cnt++;
}
static void test_notify2(int publish_type, void * detail, void *ctx) {
	printf("%s, %d\n", __FUNCTION__, publish_type);
	TestData testdata = (TestData) detail;
	testdata->notify2_cnt++;
}

static void test_notify3(int publish_type, void * detail, void *ctx) {
	printf("%s, %d\n", __FUNCTION__, publish_type);
	TestData testdata = (TestData) detail;
	testdata->notify3_cnt++;
}

static void test_notify_with_ctx(int publish_type, void * detail, void *ctx) {
	printf("%s, %d\n", __FUNCTION__, publish_type);
	TestData testdata = (TestData) detail;
	int * ctxdata = (int *)ctx;
	testdata->notify1_cnt = ++(*ctxdata);
	testdata->notify2_cnt = ++(*ctxdata);
	testdata->notify3_cnt = ++(*ctxdata);
}

static int test_failsate() {
	//call free before publisher_new, not dead process 
	publisher_free();

	//new input parameter failed
	if(publisher_new(0) == PUBLISHER_SUCCESS) {
		printf("####unfaied to call new parameter with 0\n");
		return -1;
	}

	if(publisher_subscribe(1, 1, test_notify1, NULL) != NULL) {
		printf("####unfaied to call subscribe before new publisher\n");
		return -1;
	}

	//call unsubscribe before publisher_new, not dead process 
	publisher_unsubscribe(1, NULL);

	//call publish before publisher_new, not dead process 
	publisher_publish(1, 1, NULL);

	return 0;
}

enum {
	PULISH_CONTENT_FOR_NORMAL=1,
	PULISH_CONTENT_FOR_MULTI_TYPE,
	PULISH_CONTENT_FOR_UNSUBSCRIBE,
	PULISH_CONTENT_WITH_ONETHOT,
	MAX_PUBLISHERTE
};
#define CHECK_TYPE_MAX (0xF)
static int testdata_init() {
printf("%s enter\n", __FUNCTION__);
	if(publisher_new(MAX_PUBLISHERTE) != PUBLISHER_SUCCESS) {
		printf("####1st create failed\n");
		return -1;
	}

	if(publisher_new(MAX_PUBLISHERTE) == PUBLISHER_SUCCESS) {
		printf("####multi create failed\n");
		return -1;
	}
	publisher_free();

	if(publisher_new(MAX_PUBLISHERTE) != PUBLISHER_SUCCESS) {
		printf("####2nd create failed\n");
		return -1;
	}
	return 0;
}

static int test_normally_subscribe() {
printf("%s enter\n", __FUNCTION__);
	if(publisher_subscribe(PULISH_CONTENT_FOR_NORMAL, NTYPE(1), test_notify1, NULL) == NULL) {
		printf("####failed to add test_notify1 subscribe\n");
		return -1;
	}

	if(publisher_subscribe(PULISH_CONTENT_FOR_NORMAL, NTYPE(2), test_notify2, NULL) == NULL) {
		printf("####failed to add test_notify2 subscribe\n");
		return -1;
	}

	if(publisher_subscribe(PULISH_CONTENT_FOR_NORMAL, NTYPE(3), test_notify3, NULL) == NULL) {
		printf("####failed to add test_notify3 subscribe\n");
		return -1;
	}

	testdata_t prev_data, current_data;
	memset(&prev_data, 0, sizeof(prev_data));
	memset(&current_data, 0, sizeof(current_data));

	int i=0;
	for(i=0;i<CHECK_TYPE_MAX;i++) {
		publisher_publish(PULISH_CONTENT_FOR_NORMAL, i, &current_data);
		if(i==0) {
			prev_data.notify1_cnt++;
			prev_data.notify2_cnt++;
			prev_data.notify3_cnt++;
		} else if(NTYPE(1) == i) {
			prev_data.notify1_cnt++;
		} else if(NTYPE(2) == i) {
			prev_data.notify2_cnt++;
		} else if(NTYPE(3) == i) {
			prev_data.notify3_cnt++;
		}

		if(memcmp(&prev_data, &current_data, sizeof(prev_data)) != 0) {
			printf("%d publish failed\n", i);
			return -1;
		}
	}
	return 0;
}

static int test_unsubscribe_check(int content_id, int *ntype1_p, int *ntype2_p, int *ntype3_p) {
	testdata_t prev_data, current_data;
	memset(&prev_data, 0, sizeof(prev_data));
	memset(&current_data, 0, sizeof(current_data));

	int i=0;
	for(i=0;i<=CHECK_TYPE_MAX;i++) {
		publisher_publish(content_id, i, &current_data);
		if((ntype1_p) && ((i & ~(*ntype1_p))==0)) {
			//only event bit
			prev_data.notify1_cnt++;
		}

		if((ntype2_p) && ((i & ~(*ntype2_p))==0)) {
			//only odd bit
			prev_data.notify2_cnt++;
		}
		if((ntype3_p) && ((i & (~(*ntype3_p)))==0)) {
			//all
			prev_data.notify3_cnt++;
		}

		if(memcmp(&prev_data, &current_data, sizeof(prev_data)) != 0) {
			printf("####%d publish failed\n", i);
			return -1;
		}
	}
	return 0;
}

static int test_multi_type_subscribe() {
printf("%s enter\n", __FUNCTION__);
	int ntype1=0, ntype2=0, ntype3=CHECK_TYPE_MAX;
	int i=0;
	for(i=0;i<CHECK_TYPE_MAX;i+=2) {
		//even slide bit
		ntype1+=NTYPE(i);
		//odd slide bit
		ntype2+=NTYPE(i+1);
	}

	//even slide bit
	if(publisher_subscribe(PULISH_CONTENT_FOR_MULTI_TYPE, ntype1, test_notify1, NULL) == NULL) {
		printf("####failed to add test_notify1 subscribe\n");
		return -1;
	}

	//odd slide bit
	if(publisher_subscribe(PULISH_CONTENT_FOR_MULTI_TYPE, ntype2, test_notify2, NULL) == NULL) {
		printf("####failed to add test_notify2 subscribe\n");
		return -1;
	}

	//all bit
	if(publisher_subscribe(PULISH_CONTENT_FOR_MULTI_TYPE, ntype3, test_notify3, NULL) == NULL) {
		printf("####failed to add test_notify3 subscribe\n");
		return -1;
	}

	if(test_unsubscribe_check(PULISH_CONTENT_FOR_MULTI_TYPE, &ntype1, &ntype2, &ntype3)) {
		return -1;
	}
	return 0;
}

static int test_unsubscribe() {
printf("%s enter\n", __FUNCTION__);
	int ntype1=0, ntype2=0, ntype3=CHECK_TYPE_MAX;
	int i=0;
	SubscriberAccount account1, account2, account3;
	for(i=0;i<CHECK_TYPE_MAX;i+=2) {
		//even bit
		ntype1+=NTYPE(i);
		//odd bit
		ntype2+=NTYPE(i+1);
	}

	//even bit
	account1 = publisher_subscribe(PULISH_CONTENT_FOR_UNSUBSCRIBE, ntype1, test_notify1, NULL);
	if(account1 == NULL) {
		printf("####failed to add test_notify1 subscribe\n");
		return -1;
	}

	//odd bit
	account2 = publisher_subscribe(PULISH_CONTENT_FOR_UNSUBSCRIBE, ntype2, test_notify2, NULL);
	if(account2 == NULL) {
		printf("####failed to add test_notify2 subscribe\n");
		return -1;
	}

	//all bit
	account3 = publisher_subscribe(PULISH_CONTENT_FOR_UNSUBSCRIBE, ntype3, test_notify3, NULL);
	if(account3 == NULL) {
		printf("####failed to add test_notify3 subscribe\n");
		return -1;
	}

	//unsubscribe middle place
	publisher_unsubscribe(PULISH_CONTENT_FOR_UNSUBSCRIBE, account2);
	if(test_unsubscribe_check(PULISH_CONTENT_FOR_UNSUBSCRIBE, &ntype1, NULL, &ntype3)) {
		printf("####failed to unsubscribe test_notify2\n");
		return -1;
	}

	//Unsubscribe tail place
	publisher_unsubscribe(PULISH_CONTENT_FOR_UNSUBSCRIBE, account3);
	if(test_unsubscribe_check(PULISH_CONTENT_FOR_UNSUBSCRIBE, &ntype1, NULL, NULL)) {
		printf("####failed to unsubscribe test_notify2\n");
		return -1;
	}

	//readd
	//odd bit
	account2 = publisher_subscribe(PULISH_CONTENT_FOR_UNSUBSCRIBE, ntype2, test_notify2, NULL);
	if(account2 == NULL) {
		printf("####failed to add test_notify2 subscribe\n");
		return -1;
	}
	//all bit
	account3 = publisher_subscribe(PULISH_CONTENT_FOR_UNSUBSCRIBE, ntype3, test_notify3, NULL);
	if(account3 == NULL) {
		printf("####failed to add test_notify3 subscribe\n");
		return -1;
	}

	//Unsubscribe head place
	publisher_unsubscribe(PULISH_CONTENT_FOR_UNSUBSCRIBE, account1);
	if(test_unsubscribe_check(PULISH_CONTENT_FOR_UNSUBSCRIBE, NULL, &ntype2, &ntype3)) {
		printf("####failed to unsubscribe test_notify1\n");
		return -1;
	}

	//Unsubscribe all
	publisher_unsubscribe(PULISH_CONTENT_FOR_UNSUBSCRIBE, account2);
	publisher_unsubscribe(PULISH_CONTENT_FOR_UNSUBSCRIBE, account3);
	if(test_unsubscribe_check(PULISH_CONTENT_FOR_UNSUBSCRIBE, NULL, NULL, NULL)) {
		printf("####failed to unsubscribe test_notify1\n");
		return -1;
	}
	
	return 0;
}

int test_subscribe_with_context() {
	int ctxdata=0;
	testdata_t current_data;
	memset(&current_data, 0, sizeof(current_data));

	if(publisher_subscribe(PULISH_CONTENT_FOR_NORMAL, NTYPE(1), test_notify_with_ctx, &ctxdata) == NULL) {
		printf("####failed to add test_notify1 subscribe\n");
		return -1;
	}

	publisher_publish(PULISH_CONTENT_FOR_NORMAL, NTYPE(1), &current_data);
	if(current_data.notify1_cnt != 1 || current_data.notify2_cnt != 2 || current_data.notify3_cnt != 3 || ctxdata != 3 ) {
		printf("####failed to update context\n");
		return -1;
	}
	return 0;
}

static int test_subscribe_with_oneshot() {
printf("%s enter\n", __FUNCTION__);
	if(publisher_subscribe(PULISH_CONTENT_WITH_ONETHOT, NTYPE(1), test_notify1, NULL) == NULL) {
		printf("####failed to add test_notify1 subscribe\n");
		return -1;
	}

	publisher_subscribe_oneshot(PULISH_CONTENT_WITH_ONETHOT, NTYPE(2), test_notify2, NULL);

	if(publisher_subscribe(PULISH_CONTENT_WITH_ONETHOT, NTYPE(3), test_notify3, NULL) == NULL) {
		printf("####failed to add test_notify3 subscribe\n");
		return -1;
	}

	testdata_t prev_data, current_data;
	memset(&prev_data, 0, sizeof(prev_data));
	memset(&current_data, 0, sizeof(current_data));

	int i=0;
	for(i=0;i<CHECK_TYPE_MAX;i++) {
		publisher_publish(PULISH_CONTENT_WITH_ONETHOT, i, &current_data);
		if(i==0) {
			prev_data.notify1_cnt++;
			if(!prev_data.notify2_cnt) prev_data.notify2_cnt++;
			prev_data.notify3_cnt++;
		} else if(NTYPE(1) == i) {
			prev_data.notify1_cnt++;
		} else if(NTYPE(2) == i) {
			if(!prev_data.notify2_cnt) prev_data.notify2_cnt++;
		} else if(NTYPE(3) == i) {
			prev_data.notify3_cnt++;
		}

		if(memcmp(&prev_data, &current_data, sizeof(prev_data)) != 0) {
			printf("%d publish failed 1st\n", i);
			return -1;
		}
	}

	memset(&prev_data, 0, sizeof(prev_data));
        memset(&current_data, 0, sizeof(current_data));

	/*re-call*/
	for(i=0;i<CHECK_TYPE_MAX;i++) {
		publisher_publish(PULISH_CONTENT_WITH_ONETHOT, i, &current_data);
		if(i==0) {
			prev_data.notify1_cnt++;
//			prev_data.notify2_cnt++;
			prev_data.notify3_cnt++;
		} else if(NTYPE(1) == i) {
			prev_data.notify1_cnt++;
		} else if(NTYPE(2) == i) {
//			prev_data.notify2_cnt++;
		} else if(NTYPE(3) == i) {
			prev_data.notify3_cnt++;
		}

		if(memcmp(&prev_data, &current_data, sizeof(prev_data)) != 0) {
			printf("%d publish failed 2nd\n", i);
			return -1;
		}
	}
	return 0;
}

int main() {
	if(test_failsate()) {
		printf("####test_failsate test case failed!!!\n");
		return -1;
	}

	if(testdata_init()) {
		printf("####testdata_init test case failed!!!\n");
		return -1;
	}

	if(test_normally_subscribe()) {
		printf("####test_normally_subscribe test case failed!!!\n");
		return -1;
	}

	if(test_multi_type_subscribe()) {
		printf("####test_multi_type_subscribe test case failed!!!\n");
		return -1;
	}

	if(test_unsubscribe()) {
		printf("####test_unsubscribe test case failed!!!\n");
		return -1;
	}

	if(test_subscribe_with_context()) {
		printf("####test_subscribe_with_context test case failed!!!\n");
		return -1;
	}
	if(test_subscribe_with_oneshot()) {
		printf("####test_subscribe_with_oneshot test case failed!!!\n");
		return -1;
	}

	publisher_free();
	printf("All test is success!!\n");
	return 0;
}
