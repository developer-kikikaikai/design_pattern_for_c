/**
 *    @brief      Implement of dp_timelog API, defined in dp_debug.h
**/

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <config.h>
#include "dp_debug.h"
#include "dp_mutex.h"

/*************
 * public define
*************/
/*! @name Error definition */
/* @{ */
#define DPTLOG_FAILED (-1) /*! error */
#define DPTLOG_SUCCESS (0) /*! success */
/* @} */

/*! @brief default delimiter definition */
#define DPTLOG_DELIMITER_DEFAULT (char *)" "

/*! @struct dp_timelog_data_t
 * @brief log data with timestamp, defined like class
*/
typedef struct dp_timelog_data_t{
	//private member
	struct timespec time;/*! timestamp */
	char * buf;/*! tmp buffer for using store log */
} dp_timelog_data_t;

/*! @name log class API for dp_timelog_data_t */
/* @{ */
static inline void dp_timelog_data_store(DPTimeLog mng);
/* @} */

/*! @struct dp_timelog_t
 * @brief storaged log data management structure
*/
struct dp_timelog_t{
	//private member
	const char *delimiter; /*! delimiter */
	pthread_mutex_t *lock; 

	unsigned long current_num; /*! numnber of current stored log */
	unsigned long maxstoresize; /*! max size of log_list */
	dp_timelog_data_t * log_list; /*! log list */
	size_t maxloglen;
	char * tmpbuf;/*! tmp buffer for using store log */
};

#define DPTLOG_LOCK(mng) DPUTIL_LOCK(mng->lock)
#define DPTLOG_UNLOCK DPUTIL_UNLOCK;

static void dp_timelog_show(DPTimeLog mng);

/*************
 * implement
*************/
/**for dp_timelog_data_t**/
static inline void dp_timelog_data_store(DPTimeLog mng) {
	//set time
	dp_timelog_data_t * data = &mng->log_list[mng->current_num++];
	clock_gettime(CLOCK_REALTIME, &data->time);
	memcpy(data->buf, mng->tmpbuf, mng->maxloglen);
}

/*interface*/
static void dp_timelog_show(DPTimeLog mng) {
	unsigned long i=0;
	for(i = 0; i < mng->current_num; i ++) {
		//show 1 data
		fprintf(stderr, "%u.%09lu%s%s", (unsigned int)mng->log_list[i].time.tv_sec, mng->log_list[i].time.tv_nsec, mng->delimiter, mng->log_list[i].buf);
	}
}

/*************
 * public interface API implement
*************/
DPTimeLog dp_timelog_init(const char *delimiter, size_t maxloglen, unsigned long maxstoresize, int is_threadsafe) {
	DPTimeLog mng=NULL;
	size_t size = sizeof(*mng);
	void * current_p=NULL;
	/*add buffer data size*/

	/*add dp_timelog_data_t size*/
	size += sizeof(dp_timelog_data_t) * maxstoresize;

	/*add buffer size + tmp buffer size*/
	size += (maxloglen + 1) * (maxstoresize + 1);

	mng=malloc(size);
	if(!mng) {
		DEBUG_ERRPRINT("calloc mng error:%s\n", strerror(errno));
		return NULL;
	}
	memset(mng, 0, size);

	current_p = (mng+1);

	/*set delimiter*/
	if(delimiter) {
		mng->delimiter = delimiter;
	} else {
		mng->delimiter = DPTLOG_DELIMITER_DEFAULT;
	}

	/*set lock*/
	if(is_threadsafe) {
		mng->lock = malloc(sizeof(pthread_mutex_t));
		pthread_mutex_init(mng->lock, NULL);
	}

	mng->maxstoresize = maxstoresize;

	/*set loglist*/
	unsigned long i;
	mng->log_list = (dp_timelog_data_t *)current_p;
	current_p = mng->log_list + maxstoresize;

	/*set buffer in log_list*/
	for( i = 0; i < maxstoresize; i++ ) {
		mng->log_list[i].buf = current_p;
		current_p = mng->log_list[i].buf + maxloglen;
	}

	/*set tmp buffer*/
	mng->maxloglen = maxloglen;
	mng->tmpbuf = current_p;

	return mng;
}

int dp_timelog_print(DPTimeLog mng, const char *format, ...) {
	//fail safe
	if(!mng || !format) {
		return DPTLOG_FAILED;
	}

DPTLOG_LOCK(mng)
	memset(mng->tmpbuf, 0, mng->maxloglen);
	va_list arg;
	va_start(arg, format);
	vsnprintf(mng->tmpbuf, mng->maxloglen, format, arg);
	va_end(arg);
	dp_timelog_data_store(mng);

	if(mng->maxstoresize <= mng->current_num) {
		dp_timelog_show(mng);
		mng->current_num=0;
	}
DPTLOG_UNLOCK

	return DPTLOG_SUCCESS;
}

void dp_timelog_exit(DPTimeLog mng) {
	//fail safe
	if(!mng) {
		return ;
	}

	pthread_mutex_t *lock=NULL;
DPTLOG_LOCK(mng)
	//show log
	dp_timelog_show(mng);
	lock=mng->lock;
	free(mng);
DPTLOG_UNLOCK
	free(lock);
}

