/*
 * if_znp.h
 *
 *  Created on: Apr 24, 2017
 *      Author: TuDT13
 */

#ifndef SOURCE_APP_INTERFACES_IF_ZNP_H_
#define SOURCE_APP_INTERFACES_IF_ZNP_H_

#include <stdint.h>

#include "../ak/message.h"

#include "app.h"
#include "app_data.h"
#include "app_dbg.h"

#define FRAME_DATA_SIZE		256

typedef struct {
	uint8_t state;
	uint8_t len;
	uint8_t cmd0;
	uint8_t cmd1;
	uint8_t tempDataLen;
	uint8_t data[FRAME_DATA_SIZE];
	uint8_t frame_fcs;
} if_parseinfo_frame_t; //if_parseinfo_frame_t : cấu trúc phân tích thông tin

extern int if_znp_fd;
//extern if_parseinfo_frame_t if_parseinfo_frame;
extern q_msg_t mt_task_zigbee_mng_mailbox;
extern void* mt_task_zigbee_mng_entry(void*);



#endif /* SOURCE_APP_INTERFACES_IF_ZNP_H_ */
