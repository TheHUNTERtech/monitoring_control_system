#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string>

#include "app.h"
#include "app_data.h"
#include "app_dbg.h"

#include "task_list.h"
#include "task_if.h"
#include "task_snmp.h"

using namespace std;

q_msg_t gw_task_snmp_mailbox;

void* gw_task_snmp_entry(void*) {
	task_mask_started();
	wait_all_tasks_started();
	APP_DBG("[STARTED] gw_task_snmp_entry\n");

	while (1) {

		while (msg_available(GW_TASK_SNMP_ID)) {
			/* get messge */
			ak_msg_t* msg = rev_msg(GW_TASK_SNMP_ID);

			/* handler message */
			switch (msg->header->sig) {

			default:
				break;
			}

			/* free message */
			free_msg(msg);
		}
	}

	return (void*)0;
}
