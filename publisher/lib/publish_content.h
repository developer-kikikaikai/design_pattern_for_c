/**
 * @file publish_content.h
 * @brief This is API for PublishContent class which managed in Publisher
**/
#ifndef PUBLISH_CONTENT_H_
#define PUBLISH_CONTENT_H_

#include "publisher.h"

/*! @struct publish_content
 * @brief PublishContent class instance definition, detail is in publish_content.c
*/
struct publish_content_t;
typedef struct publish_content_t publish_content_t, *PublishContent;

/*
 * @brief new
 * @param[in] none
 * @return !=NULL : PublishContent resource
 * @retval NULL : failed to subscribe
 */
PublishContent publish_content_new(void);

/*
 * @brief subscribe.
 * @param[in] publish_type type of pushlish related to publish. this ID use bitwise operation "OR". So if you want to receive notification from some publish type, please use "OR".
 * @param[in] notify notification interface. If subscriber set this IF and type, publisher notify when publish.
 * @retval !=NULL : SubscriberAccount account of this subscribe, if you want to manage unscribe/subscribe many time, please keep this accout information
 * @retval NULL : failed to subscribe
 */
SubscriberAccount publish_content_subscribe(PublishContent this, int publish_type, void (*notify)(int publish_type, void * detail));

/*
 * @brief unsubscribe. If you want to stop subscribe, please call it
 * @param[in] PublishContent
 * @param[in] account account returned at publisher_subscribe
 * @return none
 */
void publish_content_unsubscribe(PublishContent this, SubscriberAccount account);

/*
 * @brief publish. Publisher call subscriber's notify if type is same
 * @param[in] PublishContent
 * @param[in] publish_type publish type
 * @param[in] detail detail data of publish
 * @return none
 */
void publish_content_publish(PublishContent content, int publish_type, void * detail);

/*
 * @brief free
 * @param[in] PublishContent
 * @return none
 */
void publish_content_free(PublishContent content);
#endif
