#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>
#include <sys/stat.h>

#include <time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>

#include "../ak/ak.h"
#include "../ak/timer.h"

#include "../common/cmd_line.h"

#include "app.h"
#include "app_if.h"
#include "app_dbg.h"
#include "app_config.h"
#include "app_cmd.h"

#include "task_list.h"
#include "task_list_if.h"

//usr
#include "znp_serial.h"
//usr end

static int32_t i_shell_ver(uint8_t* argv);
static int32_t i_shell_help(uint8_t* argv);
static int32_t i_shell_cfg(uint8_t* argv);
static int32_t i_shell_dbg(uint8_t* argv);
static int32_t i_shell_fw(uint8_t* argv);
static int32_t i_shell_pub(uint8_t* argv);
//usr
static int32_t i_shell_per(uint8_t* argv);
static int32_t i_shell_add(uint8_t* argv);
//usr end

cmd_line_t lgn_cmd_table[] = {
	{(const int8_t*)"ver",		i_shell_ver,			(const int8_t*)"get kernel version"},
	{(const int8_t*)"help",		i_shell_help,			(const int8_t*)"help command info"},
	{(const int8_t*)"cfg",		i_shell_cfg,			(const int8_t*)"config"},
	{(const int8_t*)"dbg",		i_shell_dbg,			(const int8_t*)"debug"},
	{(const int8_t*)"fw",		i_shell_fw,				(const int8_t*)"firmware update"},
	{(const int8_t*)"sen",		i_shell_pub,			(const int8_t*)"mqtt_pub"},
	//usr
	{(const int8_t*)"per",		i_shell_per,			(const int8_t*)"Permission Joining Request"},
	{(const int8_t*)"add",		i_shell_add,			(const int8_t*)"Permission Joining Request"},

	/* End Of Table */
	{(const int8_t*)0,(pf_cmd_func)0,(const int8_t*)0}
};

int32_t i_shell_ver(uint8_t* argv) {
	(void)argv;
	APP_PRINT("version: %s\n", AK_VERSION);
	return 0;
}

int32_t i_shell_help(uint8_t* argv) {
	uint32_t idx = 0;
	switch (*(argv + 4)) {
	default:
		APP_PRINT("\nCOMMANDS INFORMATION:\n\n");
		while(lgn_cmd_table[idx].cmd != (const int8_t*)0) {
			APP_PRINT("%s\n\t%s\n\n", lgn_cmd_table[idx].cmd, lgn_cmd_table[idx].info);
			idx++;
		}
		break;
	}
	return 0;
}

int32_t i_shell_cfg(uint8_t* argv) {
	switch (*(argv + 4)) {
	case '0': {
	}
		break;

	default:
		break;
	}

	return 0;
}

int32_t i_shell_dbg(uint8_t* argv) {
	switch (*(argv + 4)) {
	case '1': {
		ak_msg_t* s_msg = get_dynamic_msg();
		set_if_des_type(s_msg, IF_TYPE_UART_GW);
		set_if_src_type(s_msg, IF_TYPE_UART_AC);
		set_if_des_task_id(s_msg, AC_TASK_DBG_ID);
		set_if_sig(s_msg, AC_DBG_TEST_3);
		uint8_t* send_data = (uint8_t*)malloc(150);
		for (uint8_t i = 0; i < 150; i++) {
			*(send_data + i) = i;
		}
		set_if_data_dynamic_msg(s_msg, send_data, 150);

		set_msg_sig(s_msg, GW_IF_DYNAMIC_MSG_OUT);
		task_post(GW_TASK_IF_ID, s_msg);
	}
		break;

	default:
		break;
	}
	return 0;
}

int32_t i_shell_pub(uint8_t* argv) {
	switch (*(argv + 4)) {
	case 'i': {
		APP_PRINT("init mqtt\n");

		ak_msg_t* s_msg = get_pure_msg();

		set_msg_sig(s_msg, 2);
		set_msg_src_task_id(s_msg, GW_TASK_CONSOLE_ID);
		task_post(GW_TASK_PUB_SUB_ID, s_msg);
	}
		break;

	case '1': {
		APP_PRINT("pub mqtt\n");
		ak_msg_t* s_msg = get_pure_msg();

		set_msg_sig(s_msg, 1);
		set_msg_src_task_id(s_msg, GW_TASK_CONSOLE_ID);
		task_post(GW_TASK_PUB_SUB_ID, s_msg);
	}
		break;

	case '3': {
		APP_PRINT("sub mqtt\n");
		ak_msg_t* s_msg = get_pure_msg();

		set_msg_sig(s_msg, 3);
		set_msg_src_task_id(s_msg, GW_TASK_CONSOLE_ID);
		task_post(GW_TASK_PUB_SUB_ID, s_msg);
	}
		break;

	default:
		break;
	}
	return 0;
}

int32_t i_shell_fw(uint8_t* argv) {
	switch (*(argv + 3)) {
	case 'b': {
		APP_PRINT("[i_shell_fw] update slave boot request\n");
		gateway_fw_dev_update_req_t gateway_fw_dev_update_req;
		memset(&gateway_fw_dev_update_req, 0, sizeof(gateway_fw_dev_update_req_t));
		strcpy(gateway_fw_dev_update_req.dev_bin_path, "/home/thannt/workspace/projects/thannt/arm_cortex_m3_base_source/boot/build_arm_cortex_m3_base_boot_stm32l/arm_cortex_m3_base_boot.bin");
		gateway_fw_dev_update_req.type_update   = TYPE_UPDATE_TARTGET_BOOT;
		gateway_fw_dev_update_req.source_if_type = IF_TYPE_UART_GW;
		gateway_fw_dev_update_req.target_task_id = AC_TASK_FW_ID;
		gateway_fw_dev_update_req.target_if_type = IF_TYPE_UART_AC;

		ak_msg_t* s_msg = get_dynamic_msg();
		set_msg_sig(s_msg, GW_FW_OTA_REQ);
		set_data_dynamic_msg(s_msg, (uint8_t*)&gateway_fw_dev_update_req, sizeof(gateway_fw_dev_update_req_t));
		set_msg_src_task_id(s_msg, GW_TASK_CONSOLE_ID);
		task_post(GW_TASK_FW_ID, s_msg);
	}
		break;

	case 'a': {
		APP_PRINT("[i_shell_fw] update slave app request\n");
		gateway_fw_dev_update_req_t gateway_fw_dev_update_req;
		memset(&gateway_fw_dev_update_req, 0, sizeof(gateway_fw_dev_update_req_t));
		strcpy(gateway_fw_dev_update_req.dev_bin_path, "/home/thannt/workspace/projects/thannt/arm_cortex_m3_base_source/application/build_arm_cortex_m3_base_application_stm32l/arm_cortex_m3_base_application.bin");
		gateway_fw_dev_update_req.type_update   = TYPE_UPDATE_TARTGET_APP;
		gateway_fw_dev_update_req.source_if_type = IF_TYPE_UART_GW;
		gateway_fw_dev_update_req.target_task_id = AC_TASK_FW_ID;
		gateway_fw_dev_update_req.target_if_type = IF_TYPE_UART_AC;

		ak_msg_t* s_msg = get_dynamic_msg();
		set_msg_sig(s_msg, GW_FW_OTA_REQ);
		set_data_dynamic_msg(s_msg, (uint8_t*)&gateway_fw_dev_update_req, sizeof(gateway_fw_dev_update_req_t));
		set_msg_src_task_id(s_msg, GW_TASK_CONSOLE_ID);
		task_post(GW_TASK_FW_ID, s_msg);
	}
		break;

	case 'r': {
		APP_PRINT("[i_shell_fw] update slave app request via rf24\n");
		gateway_fw_dev_update_req_t gateway_fw_dev_update_req;
		memset(&gateway_fw_dev_update_req, 0, sizeof(gateway_fw_dev_update_req_t));
		strcpy(gateway_fw_dev_update_req.dev_bin_path, "/home/thannt/workspace/projects/thannt/arm_cortex_m3_base_source/application/build_arm_cortex_m3_base_application_stm32l/arm_cortex_m3_base_application.bin");
		gateway_fw_dev_update_req.type_update   = TYPE_UPDATE_TARTGET_APP;
		gateway_fw_dev_update_req.source_if_type = IF_TYPE_RF24_GW;
		gateway_fw_dev_update_req.target_task_id = AC_TASK_FW_ID;
		gateway_fw_dev_update_req.target_if_type = IF_TYPE_RF24_AC;

		ak_msg_t* s_msg = get_dynamic_msg();
		set_msg_sig(s_msg, GW_FW_OTA_REQ);
		set_data_dynamic_msg(s_msg, (uint8_t*)&gateway_fw_dev_update_req, sizeof(gateway_fw_dev_update_req_t));
		set_msg_src_task_id(s_msg, GW_TASK_CONSOLE_ID);
		task_post(GW_TASK_FW_ID, s_msg);
	}
		break;

	default:
		break;
	}
	return 0;
}

//usr
int32_t i_shell_per(uint8_t* argv) {
	(void)argv;
	APP_PRINT("Permission Joining Request\n");
	ak_msg_t* s_msg = get_common_msg();

	uint8_t Timeout = 60;

	set_data_common_msg(s_msg, (uint8_t*) &Timeout, sizeof(uint8_t));
	set_msg_sig(s_msg, 0x2608);
	set_msg_src_task_id(s_msg, MT_TASK_ZIGBEE_MNG_ID);
	task_post(MT_TASK_ZIGBEE_MNG_ID, s_msg);

	APP_PRINT("End Permission Joining Request\n");

	return 0;
}

int32_t i_shell_add(uint8_t* argv) {
	(void)argv;
	APP_PRINT("Permission Joining Request bMys\n");
	{
		uint8_t Time_out = TIME_ENABLE_PERMIT_JOIN;
		ak_msg_t* s_msg = get_dynamic_msg();

		set_msg_sig(s_msg, ZB_PERMIT_JOINING_REQUEST);
		set_data_dynamic_msg(s_msg, (uint8_t*) &Time_out, 1);
		set_msg_src_task_id(s_msg, GW_TASK_CONSOLE_ID);
		task_post(MT_TASK_ZIGBEE_MNG_ID, s_msg);
	}

	return 0;
}

//end usr
