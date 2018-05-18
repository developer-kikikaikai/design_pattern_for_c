/**
 * @file dp_util.c
 *    @brief      design pattern libraries utility API
**/
#include "dp_util.h"
/*! lock */
void dputil_lock(void *handle) {
	if(!handle) {
		return;
	}
        pthread_mutex_t * lock=(pthread_mutex_t *)handle;
        pthread_mutex_lock(lock);
}
/*! unlock */
void dputil_unlock(void *handle) {
	if(!handle) {
		return;
	}
	pthread_mutex_t * lock=(pthread_mutex_t *)handle;
        pthread_mutex_unlock(lock);
}

void dputil_list_push(DPUtilList this, DPUtilListData data) {
        /* add to tail */
        data->prev = this->tail;
        //slide tail
        if(this->tail) {
                this->tail->next = data;
        }
        this->tail = data;

        /* if head is null, set to head */
        if(!this->head) {
                this->head = data;
        }
}

void dputil_list_pull(DPUtilList this, DPUtilListData data) {
        if(!data) {
                return;
        }

        /* update content */
        if(this->head == data) {
                this->head = data->next;
        } else {
                /* else case, account is not head. So there is a prev. */
                data->prev->next = data->next;
        }

        if(this->tail == data) {
                this->tail = data->prev;
        } else {
                /* else case, account is not tail. So there is a next. */
                data->next->prev = data->prev;
        }
}

DPUtilListData dputil_list_pop(DPUtilList this) {
        DPUtilListData data = this->head;
	dputil_list_pull(this, data);
	return data;
}

