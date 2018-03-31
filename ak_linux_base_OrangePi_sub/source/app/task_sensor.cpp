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
#include "task_sensor.h"

q_msg_t gw_task_sensor_mailbox;

void* gw_task_sensor_entry(void*) {
	task_mask_started();
	wait_all_tasks_started();

	APP_DBG("[STARTED] gw_task_sensor_entry\n");

	while (1) {
		while (msg_available(GW_TASK_SENSOR_ID)) {
			/* get messge */
			ak_msg_t* msg = rev_msg(GW_TASK_SENSOR_ID);

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
