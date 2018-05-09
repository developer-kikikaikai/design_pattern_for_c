/**
 *    @brief      Implement of Flyweight design petten library API, related to Publisher class
**/
#include <stdlib.h>
#include <string.h>
#include "publish_content.h"
#include "dp_util.h"

/*! @struct publisher_mng_t
 * @brief publisher class instance
*/
struct publisher_mng_t {
	int content_cnt;/*! content cnt, to use fail safe (care list size)*/
	PublishContent * contents;/*! list of PublishContent data, allocate content_cnt size*/
} publisher_g;

/* @brief definition convert from ID to INDEX macro */
#define PC_INDEX_FROM_ID(content_id) ((content_id)-1)

/*************
 * private interface API implement
*************/
/**
 * @brief get content from id
 * @param [in] content_id
 * @return PublishContent instance
 */
static PublishContent publisher_get_content(int content_id) {
	if ((0 <content_id) && (content_id<=publisher_g.content_cnt) && (publisher_g.contents)) {
		return publisher_g.contents[PC_INDEX_FROM_ID(content_id)];
	} else {
		return NULL;
	}
}

/*************
 * public interface API implement
*************/
int publisher_new(size_t contents_num) {
ENTERLOG
	/*is already called it?*/
	if(publisher_g.contents) {
		DEBUG_ERRPRINT("publisher_new was already called!\n");
		return PUBLISHER_FAILED;
	}

	if(contents_num==0) {
		DEBUG_ERRPRINT("Content num is 0!\n");
		return PUBLISHER_FAILED;
	}

	publisher_g.contents = (PublishContent *)calloc(contents_num, sizeof(PublishContent));
	if(!publisher_g.contents) {
		DEBUG_ERRPRINT("failed to alloc PublishContent list!\n");
		return PUBLISHER_FAILED;
	}

	publisher_g.content_cnt = contents_num;

	int i=0;
	for(i = 0; i < contents_num ; i ++) {
		publisher_g.contents[i] = publish_content_new();
		if(!publisher_g.contents[i]) {
			DEBUG_ERRPRINT("failed to alloc PublishContent[%d]!\n", i);
			goto err;
		}
	}

	return PUBLISHER_SUCCESS;
err:
EXITLOG
	publisher_free();
	return PUBLISHER_FAILED;	
}

void publisher_free(void) {
ENTERLOG
	if(!publisher_g.contents) {
		return;
	}

	int i=0;
	for(i = 0; i < publisher_g.content_cnt ; i ++) {
		publish_content_free(publisher_g.contents[i]);
	}

	free(publisher_g.contents);

	memset(&publisher_g, 0, sizeof(publisher_g));
EXITLOG
	return;
}

SubscriberAccount publisher_subscribe(int content_id, int publish_type, void (*notify)(int publish_type, void * detail) ) {
ENTERLOG
	if(!notify || publish_type == 0) {
		DEBUG_ERRPRINT("There is no notification information, please set it!\n");
		return NULL;
	}

	PublishContent content = publisher_get_content(content_id);
	if(!content) {
		DEBUG_ERRPRINT("invalid content_id\n");
		return NULL;
	}

	SubscriberAccount account= publish_content_subscribe(content, publish_type, notify);
EXITLOG
	return account;
}

void publisher_unsubscribe(int content_id, SubscriberAccount account) {
ENTERLOG
	PublishContent content = publisher_get_content(content_id);
	if(!content) {
		DEBUG_ERRPRINT("invalid content_id\n");
		return;
	}

	publish_content_unsubscribe(content, account);
EXITLOG
}

void publisher_publish(int content_id, int publish_type, void * detail) {
ENTERLOG
	PublishContent content = publisher_get_content(content_id);
	if(!content) {
		DEBUG_ERRPRINT("invalid content_id\n");
		return;
	}

	publish_content_publish(content, publish_type, detail);
EXITLOG
}
