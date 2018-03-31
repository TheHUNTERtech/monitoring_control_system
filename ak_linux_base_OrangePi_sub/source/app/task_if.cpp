#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../ak/ak.h"

#include "../sys/sys_dbg.h"

#include "app.h"
#include "app_if.h"
#include "app_data.h"
#include "app_dbg.h"

#include "task_list.h"
#include "task_list_if.h"
#include "task_snmp.h"
#include "if_rf24.h"

#include "task_if.h"

q_msg_t gw_task_if_mailbox;

#if (IF_RF24_ENABLE == 1)
static void rf24_if_forward_msg(ak_msg_t* msg);
#endif

#if (IF_APP_ENABLE==1)
static void app_if_forward_msg(ak_msg_t* msg);
#endif

#if (IF_CPU_SERIAL_ENABLE==1)
static void cpu_serial_if_forward_msg(ak_msg_t* msg);
#endif

#if (IF_USB_RF24_ENABLE == 1)
static void usb_stick_rf24_if_forward_msg(ak_msg_t* msg);
#endif

void* gw_task_if_entry(void*) {
	task_mask_started();
	wait_all_tasks_started();

	APP_DBG("[STARTED] gw_task_if_entry\n");

	while (1) {
		while (msg_available(GW_TASK_IF_ID)) {
			/* get messge */
			ak_msg_t* msg = rev_msg(GW_TASK_IF_ID);

			/* handler message */
			if (msg->header->if_des_type == IF_TYPE_RF24_GW ||
					msg->header->if_des_type == IF_TYPE_RF24_AC) {
#if (IF_RF24_ENABLE == 1)
				rf24_if_forward_msg(msg);
#endif

#if (IF_USB_RF24_ENABLE == 1)
				usb_stick_rf24_if_forward_msg(msg);
#endif
			}
			else if (msg->header->if_des_type == IF_TYPE_APP_GMNG ||
					 msg->header->if_des_type == IF_TYPE_APP_GW) {
#if (IF_APP_ENABLE == 1)
				app_if_forward_msg(msg);
#endif
			}
			else if (msg->header->if_des_type == IF_TYPE_UART_GW ||
					 msg->header->if_des_type == IF_TYPE_UART_AC) {
#if (IF_CPU_SERIAL_ENABLE == 1)
				cpu_serial_if_forward_msg(msg);
#endif
			}

			/* free message */
			free_msg(msg);
		}
	}

	return (void*)0;
}

#if (IF_RF24_ENABLE==1)
void rf24_if_forward_msg(ak_msg_t* msg) {
	switch (msg->header->sig) {
	case GW_IF_PURE_MSG_IN: {
		msg_inc_ref_count(msg);

		set_msg_sig(msg, msg->header->if_sig);
		set_msg_src_task_id(msg, msg->header->if_src_task_id);
		task_post(msg->header->if_des_task_id, msg);
	}
		break;

	case GW_IF_COMMON_MSG_IN: {
		msg_inc_ref_count(msg);

		set_msg_sig(msg, msg->header->if_sig);
		set_msg_src_task_id(msg, msg->header->if_src_task_id);
		task_post(msg->header->if_des_task_id, msg);
	}
		break;

	case GW_IF_PURE_MSG_OUT: {
		msg_inc_ref_count(msg);

		set_msg_sig(msg, GW_RF24_IF_PURE_MSG_OUT);
		task_post(GW_TASK_IF_RF24_ID, msg);
	}
		break;

	case GW_IF_COMMON_MSG_OUT: {
		msg_inc_ref_count(msg);

		set_msg_sig(msg, GW_RF24_IF_COMMON_MSG_OUT);
		task_post(GW_TASK_IF_RF24_ID, msg);
	}
		break;

	default:
		break;
	}
}
#endif


#if (IF_APP_ENABLE==1)
void app_if_forward_msg(ak_msg_t* msg) {
	switch (msg->header->sig) {
	case GW_IF_PURE_MSG_IN:	{
		msg_inc_ref_count(msg);

		set_msg_sig(msg, msg->header->if_sig);
		set_msg_src_task_id(msg, msg->header->if_src_task_id);
		task_post(msg->header->if_des_task_id, msg);
	}
		break;

	case GW_IF_COMMON_MSG_IN: {
		msg_inc_ref_count(msg);

		set_msg_sig(msg, msg->header->if_sig);
		set_msg_src_task_id(msg, msg->header->if_src_task_id);
		task_post(msg->header->if_des_task_id, msg);
	}
		break;

	case GW_IF_PURE_MSG_OUT: {
		msg_inc_ref_count(msg);
		task_post(GW_TASK_IF_APP_ID, msg);
	}
		break;

	case GW_IF_COMMON_MSG_OUT: {
		msg_inc_ref_count(msg);
		task_post(GW_TASK_IF_APP_ID, msg);
	}
		break;

	default:
		break;
	}
}
#endif

#if (IF_CPU_SERIAL_ENABLE==1)
void cpu_serial_if_forward_msg(ak_msg_t* msg) {
	switch (msg->header->sig) {
	case GW_IF_PURE_MSG_IN:	{
		APP_DBG("GW_IF_PURE_MSG_IN\n");
		msg_inc_ref_count(msg);

		set_msg_sig(msg, msg->header->if_sig);
		set_msg_src_task_id(msg, msg->header->if_src_task_id);
		task_post(msg->header->if_des_task_id, msg);
	}
		break;

	case GW_IF_COMMON_MSG_IN: {
		APP_DBG("GW_IF_COMMON_MSG_IN\n");
		msg_inc_ref_count(msg);

		set_msg_sig(msg, msg->header->if_sig);
		set_msg_src_task_id(msg, msg->header->if_src_task_id);
		task_post(msg->header->if_des_task_id, msg);
	}
		break;

	case GW_IF_DYNAMIC_MSG_IN: {
		APP_DBG("GW_IF_DYNAMIC_MSG_IN\n");
		msg_inc_ref_count(msg);

		set_msg_sig(msg, msg->header->if_sig);
		set_msg_src_task_id(msg, msg->header->if_src_task_id);
		task_post(msg->header->if_des_task_id, msg);
	}
		break;

	case GW_IF_PURE_MSG_OUT: {
		APP_DBG("GW_IF_PURE_MSG_OUT\n");
		msg_inc_ref_count(msg);
		task_post(GW_TASK_IF_CPU_SERIAL_ID, msg);
	}
		break;

	case GW_IF_COMMON_MSG_OUT: {
		APP_DBG("GW_IF_COMMON_MSG_OUT\n");
		msg_inc_ref_count(msg);
		task_post(GW_TASK_IF_CPU_SERIAL_ID, msg);
	}
		break;

	case GW_IF_DYNAMIC_MSG_OUT: {
		APP_DBG("GW_IF_DYNAMIC_MSG_OUT\n");
		msg_inc_ref_count(msg);
		task_post(GW_TASK_IF_CPU_SERIAL_ID, msg);
	}
		break;

	default:
		break;
	}
}
#endif

#if (IF_USB_RF24_ENABLE == 1)
void usb_stick_rf24_if_forward_msg(ak_msg_t* msg) {
	switch (msg->header->sig) {
	case GW_IF_PURE_MSG_IN:	{
		APP_DBG("GW_IF_PURE_MSG_IN\n");
		msg_inc_ref_count(msg);

		set_msg_sig(msg, msg->header->if_sig);
		set_msg_src_task_id(msg, msg->header->if_src_task_id);
		task_post(msg->header->if_des_task_id, msg);
	}
		break;

	case GW_IF_COMMON_MSG_IN: {
		APP_DBG("GW_IF_COMMON_MSG_IN\n");
		msg_inc_ref_count(msg);

		set_msg_sig(msg, msg->header->if_sig);
		set_msg_src_task_id(msg, msg->header->if_src_task_id);
		task_post(msg->header->if_des_task_id, msg);
	}
		break;

	case GW_IF_DYNAMIC_MSG_IN: {
		APP_DBG("GW_IF_DYNAMIC_MSG_IN\n");
		msg_inc_ref_count(msg);

		set_msg_sig(msg, msg->header->if_sig);
		set_msg_src_task_id(msg, msg->header->if_src_task_id);
		task_post(msg->header->if_des_task_id, msg);
	}
		break;

	case GW_IF_PURE_MSG_OUT: {
		APP_DBG("GW_IF_PURE_MSG_OUT\n");
		msg_inc_ref_count(msg);
		task_post(GW_TASK_IF_USB_STICK_RF24_ID, msg);
	}
		break;

	case GW_IF_COMMON_MSG_OUT: {
		APP_DBG("GW_IF_COMMON_MSG_OUT\n");
		msg_inc_ref_count(msg);
		task_post(GW_TASK_IF_USB_STICK_RF24_ID, msg);
	}
		break;

	case GW_IF_DYNAMIC_MSG_OUT: {
		APP_DBG("GW_IF_DYNAMIC_MSG_OUT\n");
		msg_inc_ref_count(msg);
		task_post(GW_TASK_IF_USB_STICK_RF24_ID, msg);
	}
		break;

	default:
		break;
	}
}
#endif
