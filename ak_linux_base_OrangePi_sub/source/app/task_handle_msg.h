/*
 * task_handle_msg.h
 *
 *  Created on: Apr 24, 2017
 *      Author: TuDT13
 */

#ifndef SOURCE_APP_TASK_HANDLE_MSG_H_
#define SOURCE_APP_TASK_HANDLE_MSG_H_

#include "../ak/message.h"

#define STR_LIST_MAX_SIZE			(10)
#define STR_BUFFER_SIZE				(128)

typedef struct  __attribute__((__packed__))
{
  uint16_t group_id;
  uint16_t cluster_id;
  uint16_t src_addr;
  uint8_t src_endpoint;
  uint8_t dst_endpoint;
  uint8_t was_broadcast;
  uint8_t link_quality;
  uint8_t security_use;
  uint32_t time_stamp;
  uint8_t trans_seq_num;
  uint8_t len;
  uint8_t payload[128];
} af_incoming_msg_t;

extern q_msg_t mt_task_handle_msg_mailbox;
extern void* mt_task_handle_msg_entry(void*);
extern uint8_t str_parser(char* str);
char* str_parser_get_attr(uint8_t index);
extern uint8_t str_com(char *s1, char *s2);
#endif /* SOURCE_APP_TASK_HANDLE_MSG_H_ */
