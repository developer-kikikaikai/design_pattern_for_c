/**
 * @file publisher.h
 * @brief This is API as Observer(Publish-Subscribe) design petten
**/
#ifndef PUBLISHER_H_
#define PUBLISHER_H_

#include "dp_define.h"
DP_H_BEGIN

#include <stddef.h>

#define PUBLISHER_SUCCESS (0)
#define PUBLISHER_FAILED (-1)

/*! @struct subscriber_account_t
 * @brief SubscriberAccount class member definition, to get by publisher_subscribe. This is defined in publisher.c.
*/
struct subscriber_account_t;

/** @brief SubscriberAccount class definition */
typedef struct subscriber_account_t subscriber_account_t, *SubscriberAccount;

/**
 * @brief new Publisher content, user can get notify to subscribe.
 * @param[in] contents_num max size of publish content
 * @retval PUBLISHER_SUCCESS success to create PublisherContents, you can select content by 1, 2, 3, ... ,contents_num
 * @retval PUBLISHER_FAILED Failed to create instance/already created
 */
int publisher_new(size_t contents_num);

/**
 * @brief free All publisher content
 * @return none
 */
void publisher_free(void);

/**
 * @brief subscribe
 * @param[in] content_id id of publish content you want to receive
 * @param[in] publish_type type of pushlish related to publish. this ID use bitwise operation "OR". So if you want to receive notification from some publish type, please use "OR". So, if you set 0, send notify to all
 * @param[in] notify notification interface. If subscriber set this IF and type, publisher notify when publish.
 * @param[in] ctx user definition ctx information
 * @retval !=NULL SubscriberAccount account of this subscribe, if you want to manage unscribe/subscribe many time, please keep this accout information
 * @retval NULL failed to subscribe
 */
SubscriberAccount publisher_subscribe(int content_id, int publish_type, void (*notify)(int publish_type, void * detail, void * ctx), void * ctx );

/**
 * @brief subscribe only oneshot
 * @param[in] content_id id of publish content you want to receive
 * @param[in] publish_type type of pushlish related to publish. this ID use bitwise operation "OR". So if you want to receive notification from some publish type, please use "OR". So, if you set 0, send notify to all
 * @param[in] notify notification interface. If subscriber set this IF and type, publisher notify when publish.
 * @param[in] ctx user definition ctx information
 * @return none
 * @note ctx is not free in this library
 */
void publisher_subscribe_oneshot(int content_id, int publish_type, void (*notify)(int publish_type, void * detail, void * ctx), void * ctx );
/**
 * @brief unsubscribe, if you want to stop subscribe, please call it
 * @param[in] content_id id of publish content
 * @param[in] account account returned at publisher_subscribe
 * @return none
 */
void publisher_unsubscribe(int content_id, SubscriberAccount account);

/**
 * @brief publish, Publisher call subscriber's notify if type is same
 * @param[in] content_id id
 * @param[in] publish_type publish type
 * @param[in] detail detail data of publish
 * @return none
 */
void publisher_publish(int content_id, int publish_type, void * detail);
DP_H_END
#endif
