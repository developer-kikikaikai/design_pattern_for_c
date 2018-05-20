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
	chain_func func;/*! fnction pointer */
};

/*! @brief new ChainElementPart */
static ChainElementPart chain_element_part_new(chain_func func);
/*! @brief free ChainElementPart */
static void chain_element_part_free(ChainElementPart this);
/* @} */

 /*! @struct chain_element
  * @brief ChainElement class instance definition
*/
struct chain_element_t {
	ChainElementPart head;/*! list of ChainElementPart*/
	ChainElementPart tail;
	pthread_mutex_t lock;/*! lock */
};

#define CHAIN_ELEMENT_LOCK(instance) DPUTIL_LOCK(&instance->lock)
#define CHAIN_ELEMENT_UNLOCK DPUTIL_UNLOCK

/*************
 * for ChainElementPart class API definition
*************/
static ChainElementPart chain_element_part_new(chain_func func) {
	ChainElementPart element = calloc(1, sizeof(*element));
	if(!element) {
		return NULL;
	}

	element->func = func;
	return element;
}

static void chain_element_part_free(ChainElementPart this) {
	free(this);
}

ChainElement chain_element_new(void) {
	ChainElement element = calloc(1, sizeof(*element));
	if(!element) {
		return NULL;
	}

	pthread_mutex_init(&element->lock, NULL);
	return element;
}

/*************
 * public API definition
*************/
int chain_element_add_function(ChainElement this, chain_func func) {
	int ret = COR_FAILED;
CHAIN_ELEMENT_LOCK(this)
	ChainElementPart part=chain_element_part_new(func);
	if(part) {
		dputil_list_push((DPUtilList)this, (DPUtilListData)part);
		ret = COR_SUCCESS;
	}
CHAIN_ELEMENT_UNLOCK
	return ret;
}

void chain_element_remove_function(ChainElement this, chain_func func) {
CHAIN_ELEMENT_LOCK(this)
	ChainElementPart part = this->head;
	while(part) {
		if(part->func == func) {
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

void chain_element_call(ChainElement this, void *arg) {
CHAIN_ELEMENT_LOCK(this)
	ChainElementPart part = this->head;
	while(part) {
		if((part->func) && (part->func(arg) == CoR_RETURN)) {
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
