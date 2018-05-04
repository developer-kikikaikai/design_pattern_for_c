#ifndef PUBLISH_MSG_
#define PUBLISH_MSG_

#define PUBLISH_ID (1)
#define PUB_MSG_PUBLISH_NEW_BOOK (0x01<<0)
#define PUB_MSG_PUBLISH_DISCOUNT_BOOK (0x01<<1)
#define PUB_MSG_PUBLISH_STOP_PRODUCTION (0x01<<2)

typedef struct publish_msg_detail {
	char writername[32];
} publish_msg_detail_t;

#endif
