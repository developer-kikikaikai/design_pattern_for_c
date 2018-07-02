/**
 * @file publish_content.c
 *    @brief      Implement of Flyweight design petten library API, related to PublishContent class
**/
#include "publish_content.h"
#include <stdlib.h>
#include <pthread.h>
#include "dp_util.h"
struct subscriber_account_t {
	SubscriberAccount next;
	SubscriberAccount prev;
	int publish_type;
	void (*notify)(int publish_type, void * detail, void * ctx);
	void * ctx;
};

struct publish_content_t {
	SubscriberAccount head;
	SubscriberAccount tail;
	pthread_mutex_t lock;
};


/*! @name PublishContent private method */
/* @{ */
/*! push subscriber */
static inline void publish_content_push_subscriber(PublishContent this, SubscriberAccount account) {
	dputil_list_push((DPUtilList)this, (DPUtilListData)account);
}
/*! pull subscriber */
static inline void publish_content_pull_subscriber(PublishContent this, SubscriberAccount account) {
	dputil_list_pull((DPUtilList)this, (DPUtilListData)account);
	free(account);
}
/*! pop subscriber */
static inline SubscriberAccount publish_content_pop_subscriber(PublishContent this) {
	SubscriberAccount account = (SubscriberAccount)dputil_list_pop((DPUtilList)this);
	free(account);
	return account;
}
#define PUBLISH_CONTENT_LOCK(content) DPUTIL_LOCK(&(content->lock))
#define PUBLISH_CONTENT_UNLOCK DPUTIL_UNLOCK
/* @} */

/*************
 * public interface API implement
*************/
PublishContent publish_content_new(void) {
	PublishContent content = (PublishContent)calloc(1, sizeof(publish_content_t));
	pthread_mutex_init(&content->lock, NULL);
	return content;
}

SubscriberAccount publish_content_subscribe(PublishContent this, int publish_type, void (*notify)(int publish_type, void * detail, void * ctx), void * ctx) {
	SubscriberAccount account=NULL;

PUBLISH_CONTENT_LOCK(this)
	account = (SubscriberAccount)calloc(1, sizeof(publish_content_t));
	if(account) {
		account->publish_type = publish_type;
		account->notify = notify;
		account->ctx = ctx;
		publish_content_push_subscriber(this, account);
	}
PUBLISH_CONTENT_UNLOCK
	return account;
}

void publish_content_unsubscribe(PublishContent this, SubscriberAccount account) {
PUBLISH_CONTENT_LOCK(this)
	publish_content_pull_subscriber(this, account);
PUBLISH_CONTENT_UNLOCK
	
}

void publish_content_publish(PublishContent this, int publish_type, void * detail) {
PUBLISH_CONTENT_LOCK(this)
	SubscriberAccount account;
	for(account = this->head ; account != NULL ; account = account->next ) {
		/* check type */
		if((account->publish_type & publish_type) == publish_type) {
			/* notify message, notify is not null because publisher check it */
			account->notify(publish_type, detail, account->ctx);
		}
	}

PUBLISH_CONTENT_UNLOCK
}

void publish_content_free(PublishContent this ) {
PUBLISH_CONTENT_LOCK(this)
	SubscriberAccount account;

	/* free all account */
	account=this->head;
	while(account) {
		/* unsubscribe head */
		account = publish_content_pop_subscriber(this);
	}

PUBLISH_CONTENT_UNLOCK
	/* free this after unlick*/
	free(this);
}
