#ifndef __TASK_SM_H__
#define __TASK_SM_H__

#include "../ak/ak.h"
#include "../ak/message.h"

extern q_msg_t gw_task_sm_mailbox;
extern void* gw_task_sm_entry(void*);

#endif //__TASK_SM_H__
