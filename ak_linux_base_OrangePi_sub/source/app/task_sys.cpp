#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../ak/ak.h"
#include "../ak/timer.h"

#include "app.h"
#include "app_if.h"
#include "app_dbg.h"
#include "app_data.h"

#include "task_list.h"
#include "task_sys.h"

q_msg_t gw_task_sys_mailbox;

void* gw_task_sys_entry(void*) {
	task_mask_started();
	wait_all_tasks_started();

	APP_DBG("[STARTED] gw_task_sys_entry\n");

	while (1) {
		while (msg_available(GW_TASK_SYS_ID)) {
			/* get messge */
			ak_msg_t* msg = rev_msg(GW_TASK_SYS_ID);

			switch (msg->header->sig) {

			case GW_SYS_WATCH_DOG_REPORT_REQ: {
				APP_DBG("GW_SYS_WATCH_DOG_REPORT_REQ\n");
			}
				break;

			default:
				break;
			}

			/* free message */
			free_msg(msg);
		}
	}

	return (void*)0;
}
