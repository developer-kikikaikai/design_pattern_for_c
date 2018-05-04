/**
 *    @brief      Implement of Flyweight design petten library API, related to PublishContent class
**/
#include "publish_content.h"
#include <stdlib.h>
#include <pthread.h>
#include "debug.h"
typedef struct subscriber_account {
	SubscriberAccount next;
	SubscriberAccount prev;
	int publish_type;
	void (*notify)(int publish_type, PublishDetail detail);
} subscriber_account_t;

struct publish_content {
	SubscriberAccount head;
	SubscriberAccount tail;
	pthread_mutex_t lock;
};


/*! @name PublishContent private method */
/* @{ */
/* ! push subscriber */
static void publish_content_push_subscriber(PublishContent this, SubscriberAccount account);
/* ! pop subscriber */
static void publish_content_pop_subscriber(PublishContent this, SubscriberAccount account);
/* ! unsubscribe without lock */
static void publish_content_unsubscribe_no_lock(PublishContent this, SubscriberAccount account);
/*! lock */
static inline void publish_content_lock(void *handle);
/*! unlock */
static inline void publish_content_unlock(void *handle);
/*! lock macro*/
#define PUBLISH_CONTENT_LOCK(content) \
	publish_content_lock(&content->lock);\
	pthread_cleanup_push(publish_content_unlock, &content->lock);

#define PUBLISH_CONTENT_UNLOCK pthread_cleanup_pop(1);
/* @} */
/* @} */

/*************
 * private interface API implement
*************/
static void publish_content_push_subscriber(PublishContent this, SubscriberAccount account) {
ENTERLOG
	/* add to tail */
	account->prev = this->tail;
	//slide tail
	if(this->tail) {
		this->tail->next = account;
	}
	this->tail = account;

	/* if head is null, set to head */
	if(!this->head) {
		this->head = account;
	}
EXITLOG
}

static void publish_content_pop_subscriber(PublishContent this, SubscriberAccount account) {
ENTERLOG

	if(!account) {
		return;
	}

	/* update content */
	if(this->head == account) {
		this->head = account->next;
	} else {
		/* else case, account is not head. So there is a prev. */
		account->prev->next = account->next;
	}

	if(this->tail == account) {
		this->tail = account->prev;
	} else {
		/* else case, account is not tail. So there is a next. */
		account->next->prev = account->prev;
	}
EXITLOG
}

static void publish_content_unsubscribe_no_lock(PublishContent this, SubscriberAccount account) {
ENTERLOG
	publish_content_pop_subscriber(this, account);
	free(account);
EXITLOG
	
}

static inline void publish_content_lock(void *handle) {
ENTERLOG
	pthread_mutex_t * lock=(pthread_mutex_t *)handle;
	pthread_mutex_lock(lock);
EXITLOG
}

static inline void publish_content_unlock(void *handle) {
ENTERLOG
	pthread_mutex_t * lock=(pthread_mutex_t *)handle;
	pthread_mutex_unlock(lock);
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

SubscriberAccount publish_content_subscribe(PublishContent this, int publish_type, void (*notify)(int publish_type, PublishDetail detail)) {
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

void publish_content_publish(PublishContent this, int publish_type, PublishDetail detail) {
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
