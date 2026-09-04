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
	int is_oneshot;
	int publish_type;
	void (*notify)(int publish_type, void * detail, void * ctx);
	void * ctx;
};

struct publish_content_t {
	SubscriberAccount head;
	SubscriberAccount tail;
	pthread_mutex_t lock;
};

typedef struct publish_notification_t {
	void (*notify)(int publish_type, void * detail, void * ctx);
	void *ctx;
} publish_notification_t;


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
	if(!content) {
		return NULL;
	}
	pthread_mutex_init(&content->lock, NULL);
	return content;
}

SubscriberAccount publish_content_subscribe(PublishContent this, int publish_type, void (*notify)(int publish_type, void * detail, void * ctx), void * ctx) {
	SubscriberAccount account=NULL;

PUBLISH_CONTENT_LOCK(this)
	account = (SubscriberAccount)calloc(1, sizeof(*account));
	if(account) {
		account->publish_type = publish_type;
		account->notify = notify;
		account->ctx = ctx;
		publish_content_push_subscriber(this, account);
	}
PUBLISH_CONTENT_UNLOCK
	return account;
}

void publish_content_subscribe_oneshot(PublishContent this, int publish_type, void (*notify)(int publish_type, void * detail, void * ctx), void * ctx) {

PUBLISH_CONTENT_LOCK(this)
	SubscriberAccount account = (SubscriberAccount)calloc(1, sizeof(*account));
	if(account) {
		account->is_oneshot = 1;
		account->publish_type = publish_type;
		account->notify = notify;
		account->ctx = ctx;
		publish_content_push_subscriber(this, account);
	}
PUBLISH_CONTENT_UNLOCK
}

void publish_content_unsubscribe(PublishContent this, SubscriberAccount account) {
PUBLISH_CONTENT_LOCK(this)
	publish_content_pull_subscriber(this, account);
PUBLISH_CONTENT_UNLOCK
	
}

void publish_content_publish(PublishContent this, int publish_type, void * detail) {
	size_t notification_count = 0;
	size_t notification_index = 0;
	publish_notification_t *notifications = NULL;
	SubscriberAccount account;

PUBLISH_CONTENT_LOCK(this)
	for(account = this->head; account; account = account->next) {
		if((account->publish_type & publish_type) == publish_type) {
			notification_count++;
		}
	}

	if(notification_count) {
		notifications = calloc(notification_count, sizeof(*notifications));
	}

	/*
	 * Copy callback values while the subscriber list is protected.  User
	 * callbacks must run without this lock so that they may subscribe or
	 * unsubscribe recursively.
	 */
	account = this->head;
	while(account && notifications) {
		SubscriberAccount account_next = account->next;
		/* check type */
		if((account->publish_type & publish_type) != publish_type) {
			account=account_next;
			continue;
		}

		notifications[notification_index].notify = account->notify;
		notifications[notification_index].ctx = account->ctx;
		notification_index++;
		if(account->is_oneshot) {
			publish_content_pull_subscriber(this, account);
		}
		account=account_next;
	}
PUBLISH_CONTENT_UNLOCK

	for(notification_index = 0; notification_index < notification_count && notifications; notification_index++) {
		notifications[notification_index].notify(publish_type, detail, notifications[notification_index].ctx);
	}
	free(notifications);
}

void publish_content_free(PublishContent this ) {
PUBLISH_CONTENT_LOCK(this)
	SubscriberAccount account;

	/* free all account */
	account=this->head;
	while(account) {
		/* unsubscribe head */
		account = publish_content_pop_subscriber(this);
		free(account);
		account = this->head;
	}

PUBLISH_CONTENT_UNLOCK
	pthread_mutex_destroy(&this->lock);
	/* free this after unlick*/
	free(this);
}
