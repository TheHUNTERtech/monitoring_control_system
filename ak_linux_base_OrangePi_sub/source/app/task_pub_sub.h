#ifndef __TASK_PUB_SUB_H__
#define __TASK_PUB_SUB_H__

#include "../ak/message.h"

extern q_msg_t gw_task_pub_sub_mailbox;
extern void* gw_task_pub_sub_entry(void*);

#endif // __TASK_PUB_SUB_H__
