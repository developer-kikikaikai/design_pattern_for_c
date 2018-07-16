/**
 * @file chain_element.c
 *    @brief      Implement of ChainElement class
 **/

#include "dp_util.h"
#include "chain_element.h"

/*! @name ChainElementPart class */
/* @{ */

/*! @struct ChainElementPart
 * @brief chain function element part class instance definition
*/
typedef struct chain_element_part * ChainElementPart;
struct chain_element_part {
	ChainElementPart next;
	ChainElementPart prev;
	chain_element_data_t data;
};

/*! @brief new ChainElementPart */
static ChainElementPart chain_element_part_new(chain_element_data_t * element_data);
/*! @brief free ChainElementPart */
static void chain_element_part_free(ChainElementPart this);
/* @} */

 /*! @struct chain_element
  * @brief ChainElement class instance definition
*/
struct chain_element_t {
	ChainElementPart head;/*! list of ChainElementPart*/
	ChainElementPart tail;
	pthread_mutex_t *lock;/*! lock */
};

#define CHAIN_ELEMENT_LOCK(instance) DPUTIL_LOCK(instance->lock)
#define CHAIN_ELEMENT_UNLOCK DPUTIL_UNLOCK

/*************
 * for ChainElementPart class API definition
*************/
static ChainElementPart chain_element_part_new(chain_element_data_t * element_data) {
	ChainElementPart element = calloc(1, sizeof(*element));
	if(!element) {
		return NULL;
	}

	memcpy(&element->data, element_data, sizeof(element->data));
	return element;
}

static void chain_element_part_free(ChainElementPart this) {
	free(this);
}

ChainElement chain_element_new(int is_threadsafe) {
	ChainElement element = calloc(1, sizeof(*element) + (is_threadsafe * sizeof(pthread_mutex_t)));
	if(!element) {
		return NULL;
	}

	if(is_threadsafe) {
		element->lock = (pthread_mutex_t *)(element + 1);
		pthread_mutex_init(element->lock, NULL);
	}
	return element;
}

/*************
 * public API definition
*************/
int chain_element_add_function(ChainElement this, chain_element_req_t * elemnt_data) {
	int ret = COR_FAILED;
CHAIN_ELEMENT_LOCK(this)
	ChainElementPart part=chain_element_part_new(&elemnt_data->element_data);
	if(part) {
		dputil_list_push((DPUtilList)this, (DPUtilListData)part);
		elemnt_data->result_element_part = part;
		ret = COR_SUCCESS;
	}
CHAIN_ELEMENT_UNLOCK
	return ret;
}

void chain_element_remove_function(ChainElement this, chain_func func) {
CHAIN_ELEMENT_LOCK(this)
	ChainElementPart part = this->head;
	while(part) {
		if(part->data.func == func) {
			//free ChainElementPart
			ChainElementPart free_part = part;
			part=part->next;
			dputil_list_pull((DPUtilList)this, (DPUtilListData)free_part);
			chain_element_part_free(free_part);
		} else {
			part=part->next;
		}
	}
CHAIN_ELEMENT_UNLOCK
}

void chain_element_remove_element_part(ChainElement this, ChainElementPart element) {
CHAIN_ELEMENT_LOCK(this)
	dputil_list_pull((DPUtilList)this, (DPUtilListData)element);
	chain_element_part_free(element);
CHAIN_ELEMENT_UNLOCK
}

void chain_element_call(ChainElement this, void *arg) {
CHAIN_ELEMENT_LOCK(this)
	ChainElementPart part = this->head;
	while(part) {
		if((part->data.func) && (part->data.func(arg, part->data.ctx) == CoR_RETURN)) {
			/*exit*/
			break;
		}
		part=part->next;
	}
CHAIN_ELEMENT_UNLOCK
}

void chain_element_delete(ChainElement this) {
CHAIN_ELEMENT_LOCK(this)
	ChainElementPart part;
	do {
		part = (ChainElementPart) dputil_list_pop((DPUtilList) this);
		chain_element_part_free(part);
	} while(part);
CHAIN_ELEMENT_UNLOCK
	free(this);
}
