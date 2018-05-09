/**
 *    @brief      Implement of Flyweight design petten library API, related to PublishContent class
**/
#include "publish_content.h"
#include <stdlib.h>
#include <pthread.h>
#include "dp_util.h"
typedef struct subscriber_account {
	SubscriberAccount next;
	SubscriberAccount prev;
	int publish_type;
	void (*notify)(int publish_type, void * detail);
} subscriber_account_t;

struct publish_content {
	SubscriberAccount head;
	SubscriberAccount tail;
	pthread_mutex_t lock;
};


/*! @name PublishContent private method */
/* @{ */
/* ! push subscriber */
static inline void publish_content_push_subscriber(PublishContent this, SubscriberAccount account) {
	dputil_list_push((DPUtilList)this, (DPUtilListData)account);
}
/* ! pop subscriber */
static inline void publish_content_pop_subscriber(PublishContent this, SubscriberAccount account) {
	dputil_list_pop((DPUtilList)this, (DPUtilListData)account);
}
/* ! unsubscribe without lock */
static void publish_content_unsubscribe_no_lock(PublishContent this, SubscriberAccount account);
#define PUBLISH_CONTENT_LOCK(content) DPUTIL_LOCK(&(content->lock))
#define PUBLISH_CONTENT_UNLOCK DPUTIL_UNLOCK
/* @} */
/* @} */

/*************
 * private interface API implement
*************/
static void publish_content_unsubscribe_no_lock(PublishContent this, SubscriberAccount account) {
ENTERLOG
	publish_content_pop_subscriber(this, account);
	free(account);
EXITLOG
	
}

/*************
 * public interface API implement
*************/
PublishContent publish_content_new(void) {
ENTERLOG
	PublishContent content = (PublishContent)calloc(1, sizeof(publish_content_t));
	pthread_mutex_init(&content->lock, NULL);
	return content;
EXITLOG
}

SubscriberAccount publish_content_subscribe(PublishContent this, int publish_type, void (*notify)(int publish_type, void * detail)) {
ENTERLOG
	SubscriberAccount account=NULL;

PUBLISH_CONTENT_LOCK(this)
	account = (SubscriberAccount)calloc(1, sizeof(publish_content_t));
	if(!account) {
		goto end;
	}
	account->publish_type = publish_type;
	account->notify = notify;
	publish_content_push_subscriber(this, account);
end:
PUBLISH_CONTENT_UNLOCK
EXITLOG
	return account;
}

void publish_content_unsubscribe(PublishContent this, SubscriberAccount account) {
ENTERLOG
PUBLISH_CONTENT_LOCK(this)
	publish_content_unsubscribe_no_lock(this, account);
PUBLISH_CONTENT_UNLOCK
EXITLOG
	
}

void publish_content_publish(PublishContent this, int publish_type, void * detail) {
ENTERLOG
PUBLISH_CONTENT_LOCK(this)
	SubscriberAccount account;
	for(account = this->head ; account != NULL ; account = account->next ) {
		/* check type */
		if((account->publish_type & publish_type) == publish_type) {
			/* notify message, notify is not null because publisher check it */
			account->notify(publish_type, detail);
		}
	}

PUBLISH_CONTENT_UNLOCK
EXITLOG
}
void publish_content_free(PublishContent this ) {
ENTERLOG
PUBLISH_CONTENT_LOCK(this)
	SubscriberAccount account;

	/* free all account */
	while(this->head) {
		/* unsubscribe head */
		publish_content_unsubscribe_no_lock(this, this->head);
	}

PUBLISH_CONTENT_UNLOCK
	/* free this after unlick*/
	free(this);
EXITLOG
}
