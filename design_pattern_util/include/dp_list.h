#ifndef DPUTIL_LIST_
#define DPUTIL_LIST_
/**
 *    @brief  Utility list API for design pattern
**/

/*! @name List API */
/* @{ */
//when you use it, please define own next/prev pointer at the top
typedef struct dputil_list_data_t *DPUtilListData;
struct dputil_list_data_t {
	DPUtilListData next;
	DPUtilListData prev;
};

//when you use it, please define own head/tail pointer at the top
typedef struct dputil_list_t *DPUtilList;
struct dputil_list_t {
	DPUtilListData head;
	DPUtilListData tail;
};

void dputil_list_push(DPUtilList this, DPUtilListData data);
void dputil_list_pull(DPUtilList this, DPUtilListData data);
DPUtilListData dputil_list_pop(DPUtilList this);
/* }@ */

#endif
