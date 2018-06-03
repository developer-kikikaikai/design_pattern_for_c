/**
 * @file dp_list.h
 *    @brief  Utility list API for design pattern
**/
#ifndef DPUTIL_LIST_H_
#define DPUTIL_LIST_H_

/*! @name List Structure */
/* @{ */
/** @brief Typedef class DPUtilListData, member is defined in struct dputil_list_data_t*/
typedef struct dputil_list_data_t *DPUtilListData;
/**! @struct dputil_list_data_t
 * @brief This structure will override to use, when you use it, please define own next/prev pointer at the top
*/
struct dputil_list_data_t {
	DPUtilListData next;
	DPUtilListData prev;
};
/**! @struct DPUtilList
 * @brief This structure will override to use, when you use it, please define own head/tail pointer at the top
*/
typedef struct dputil_list_t *DPUtilList;
struct dputil_list_t {
	DPUtilListData head;
	DPUtilListData tail;
};
/* @} */

/*! @name List API */
/* @{ */
/*! list push */
void dputil_list_push(DPUtilList this, DPUtilListData data);
/*! list insert to point */
void dputil_list_insert(DPUtilList this, DPUtilListData prev, DPUtilListData data);
/*! list pull */
void dputil_list_pull(DPUtilList this, DPUtilListData data);
/*! list pop */
DPUtilListData dputil_list_pop(DPUtilList this);
/* @} */

#endif
