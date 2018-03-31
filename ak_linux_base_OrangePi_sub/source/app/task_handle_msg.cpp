/*
 * task_handle_msg.cpp
 *
 *  Created on: Apr 24, 2017
 *      Author: TuDT13
 */

#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "app.h"
#include "app_if.h"
#include "app_data.h"
#include "app_dbg.h"
#include "app_config.h"

#include "task_list.h"
#include "task_list_if.h"

#include "task_handle_msg.h"

#include "zigbee_manager/if_znp.h"
#include "zigbee_manager/zcl.h"
#include "zigbee_manager/znp_serial.h"
#include "../common/global_parameters.h"

static char cmd_buffer[STR_BUFFER_SIZE];
static char* str_list[STR_LIST_MAX_SIZE];
static uint8_t str_list_len;

q_msg_t mt_task_handle_msg_mailbox;

//app_zigbee_device_list_t zigbee_sensor_list;
//app_zigbee_device_data_t zigbee_sensor_data;
int i=0;

void sensorDatatest(zclOutgoingMsg_t outgoingMsg);

void* mt_task_handle_msg_entry(void*) {
	task_mask_started();
	wait_all_tasks_started();

	APP_DBG("mt_task_handle_msg_entry\n");

	while (1) {
		while (msg_available(MT_TASK_HANDLE_MSG_ID)) {
			/* get messge */
			ak_msg_t* msg = rev_msg(MT_TASK_HANDLE_MSG_ID);

			/* handler message */
			switch (msg->header->sig) {
			case HANDLE_MESSAGE_DATA_INCOMING: {
				printf("HANDLE_MESSAGE_DATA_INCOMING\n");
				zclOutgoingMsg_t outgoingMsg; //data send to mqtt
				uint8_t* data = (uint8_t *) msg->header->payload;
				outgoingMsg = *(zclOutgoingMsg_t *) data;
				data += sizeof(zclOutgoingMsg_t);
				outgoingMsg.data = data;
				sensorDatatest(outgoingMsg);
			}
				break;
			case MT_ZIGBEE_SENSOR_JOIN: {
				printf("NEW ZIGBEE DEVICE JOIN\n");

				uint8_t* data = (uint8_t*)msg->header->payload;
				printf("data: %s\n", data);
				for (unsigned int i = 0; i < msg->header->len; i++) {
					printf("%c", data[i]);
				}
				printf("\n");
				str_parser((char*)data);

			}
				break;
			case MT_ZIGBEE_SENSOR_DATA: {
				printf("DATA FORM SENSOR\n");
				uint8_t *data = (uint8_t*) msg->header->payload;
				for (unsigned int i = 0; i < msg->header->len; i++) {
					printf("%c", data[i]);
				}
				printf("\n");

				str_parser((char*)data);
				char *short_addr = str_parser_get_attr(3);
				char *type  = str_parser_get_attr(4);
				char *sensor_data = str_parser_get_attr(5);




				{
					ak_msg_t* s_msg = get_pure_msg();

					set_msg_sig(s_msg, 1);
					set_msg_src_task_id(s_msg, MT_TASK_HANDLE_MSG_ID);
					task_post(GW_TASK_PUB_SUB_ID, s_msg);
				}
			}
				break;

			default:
				printf("unknown signal handle message!");
				break;
			}
			/* free message */
			free_msg(msg);
		}

		usleep(1000);
	}

	return (void*) 0;
}

/*sensor data*/
void sensorDatatest(zclOutgoingMsg_t outgoingMsg) {

	//	APP_DBG("sensor data begin\n");
	uint8_t client_message[MAX_MESSAGE_LEN];
	for (int var = 0; var < MAX_MESSAGE_LEN; ++var) {
		client_message[var] = 0;
	}
	int32_t i = 0;
	int8_t flag_unknown_cluster = 0;
	//add token communicate
	client_message[i] = SEPARATE_CHAR;
	printf("%s",&client_message[i]);
	i++;
	memcpy((uint8_t*) &client_message[i], g_strTokenCommunicate.c_str(), g_strTokenCommunicate.size());
	printf("%s",&client_message[i],i);
	i += g_strTokenCommunicate.size();
	//add time
	client_message[i] = SEPARATE_CHAR;
	printf("%s",&client_message[i]);
	i++;
	time_t rawtime = time(NULL);
	bytestoHexChars((uint8_t*) &rawtime, sizeof(time_t), (uint8_t*) &client_message[i]);
	printf("%s", &client_message[i]);
	i += sizeof(time_t) * 2;

	//add command sensor data
	client_message[i] = SEPARATE_CHAR;
	printf("%s",&client_message[i]);
	i++;
	memcpy((uint8_t*) &client_message[i], g_strCommand[sensordata].c_str(), g_strCommand[sensordata].size());
	printf("%s",&client_message[i]);
	i += g_strCommand[sensordata].size();
	//add device id 2bytes
	client_message[i] = SEPARATE_CHAR;
	printf("%s",&client_message[i]);
	i++;
	bytestoHexChars((uint8_t*) &outgoingMsg.short_addr, sizeof(outgoingMsg.short_addr), (uint8_t*) &client_message[i]);
	printf("%02X",outgoingMsg.short_addr);
	i += sizeof(outgoingMsg.short_addr) * 2;

	//	APP_DBG("add tokencommunicate, time , device id success!\n");
	//add sensor type
	client_message[i] = SEPARATE_CHAR;
	i++;
	switch (outgoingMsg.cluster_id) {
	case ZCL_CLUSTER_ID_MS_RELATIVE_HUMIDITY: {	//humidity
		memcpy((uint8_t*) &client_message[i], (uint8_t*) g_strSensorType[RELATIVE_HUMIDITY].c_str(), g_strSensorType[RELATIVE_HUMIDITY].size());
		i += g_strSensorType[RELATIVE_HUMIDITY].size();

		//		//add sensordata
		client_message[i] = SEPARATE_CHAR;
		printf("%s",&client_message[i]);
		i++;
		uint16_t tempSig = (uint16_t) ((outgoingMsg.data[0]) + 0x0100 * (outgoingMsg.data[1]));
		printf("\n\t\tByte 1: %d\n",outgoingMsg.data[0]);
		printf("\t\tByte 2: %d\n",outgoingMsg.data[1]);
		//printf("Humidity = %d,%d  | %d	\n", tempSig / 100, tempSig % 100, tempSig);
		char data[10];
		snprintf(data, 10, "%d,%d", tempSig / 100, tempSig % 100);
		memcpy((uint8_t*) &client_message[i], data, strlen((const char*) data));
		printf("%s\n",&client_message[i]);
		i += strlen((const char*) data);

	}
		break;
	case ZCL_CLUSTER_ID_MS_TEMPERATURE_MEASUREMENT: {	//temperature
		memcpy((uint8_t*) &client_message[i], g_strSensorType[TEMPERATURE_MEASUREMENT].c_str(), g_strSensorType[TEMPERATURE_MEASUREMENT].size());
		i += g_strSensorType[TEMPERATURE_MEASUREMENT].size();
		printf("%S",&client_message[i]);
		//		//add sensordata
		client_message[i] = SEPARATE_CHAR;
		printf("%s",&client_message[i]);
		i++;
		uint16_t tempSig = (uint16_t) ((outgoingMsg.data[0]) + 0x0100 * (outgoingMsg.data[1]));
		//printf("Temperature = %d,%d  | %d	\n", tempSig / 100, tempSig % 100, tempSig);
		char data[10];
		snprintf(data, 10, "%d,%d", tempSig / 100, tempSig % 100);
		memcpy((uint8_t*) &client_message[i], data, strlen((const char*) data));
		printf("%s\n",&client_message[i]);
		i += strlen((const char*) data);
	}
		break;
	case ZCL_CLUSTER_ID_MS_OCCUPANCY_SENSING: {	//motion
		printf("\n\nZCL_CLUSTER_ID_MS_OCCUPANCY_SENSING\n\n");
		memcpy((uint8_t*) &client_message[i], g_strSensorType[OCCUPANCY_SENSING].c_str(), g_strSensorType[OCCUPANCY_SENSING].size());
		i += g_strSensorType[OCCUPANCY_SENSING].size();
		//		//add sensordata
		//		client_message[i] = SEPARATE_CHAR;
		//		i++;

		//		bytestoHexChars((uint8_t*) &outgoingMsg.data[0], outgoingMsg.dataLen, (uint8_t*) &client_message[i]);

		//		i += outgoingMsg.dataLen * 2;
	}
		break;
	case ZCL_CLUSTER_ID_GEN_ON_OFF: {	//on-off door
		//printf("CAM BIEN CUA\n");
		//printf("Door: %d\n",outgoingMsg.data[0]);

		memcpy((uint8_t*) &client_message[i], (uint8_t*) g_strSensorType[ON_OFF].c_str(), g_strSensorType[ON_OFF].size());
		printf("%s",&client_message[i]);

		i += g_strSensorType[ON_OFF].size();
		//add sensordata
		client_message[i] = SEPARATE_CHAR;
		printf("%s",&client_message[i]);
		i++;

		bytestoHexChars((uint8_t*) &outgoingMsg.data[0], outgoingMsg.dataLen, (uint8_t*) &client_message[i]);
		printf("%02X\n",&client_message[i]);
		i += outgoingMsg.dataLen * 2;
	}
		break;
	default:
		APP_DBG("unknown data\n");
		char data[10];
		snprintf(data, 10, "0x%X", outgoingMsg.cluster_id);
		memcpy((uint8_t*) &client_message[i], data, strlen((const char*) data));

		i += strlen((const char*) data);
		flag_unknown_cluster = 1;
		break;
	}

	//add sensordata
	client_message[i] = SEPARATE_CHAR;
	i++;
	printf(":");
	bytestoHexChars((uint8_t*) &outgoingMsg.data[0], outgoingMsg.dataLen, (uint8_t*) &client_message[i]);
	i += outgoingMsg.dataLen * 2;

	//end message
	client_message[i] = '\0';
	i++;



	if (flag_unknown_cluster == 0) {

		{
			/*Prepare and signal next task*/
			uint8_t *data = (uint8_t*) client_message;
			ak_msg_t* s_msg = get_dynamic_msg();
			set_msg_sig(s_msg, MT_ZIGBEE_SENSOR_DATA);
			set_data_dynamic_msg(s_msg, data, i);
			set_msg_src_task_id(s_msg, MT_TASK_HANDLE_MSG_ID);
			task_post(MT_TASK_HANDLE_MSG_ID, s_msg);
			APP_DBG("------------------------------------------>");
		}
	} else {
		printf("\n unknown data = %s\n", client_message);
	}
}


uint8_t str_parser(char* str) {
	strcpy(cmd_buffer, str);
	str_list_len = 0;

	uint8_t i = 0;
	uint8_t str_list_index = 0;
	uint8_t flag_insert_str = 1;

	while (cmd_buffer[i] != 0 && cmd_buffer[i] != '\n' && cmd_buffer[i] != '\r') {
		if (cmd_buffer[i] == ':') {
			cmd_buffer[i] = 0;
			flag_insert_str = 1;
		}
		else if (flag_insert_str) {
			str_list[str_list_index++] = &cmd_buffer[i];
			flag_insert_str = 0;
		}
		i++;
	}

	cmd_buffer[i] = 0;

	str_list_len = str_list_index;
	return str_list_len;
}

char* str_parser_get_attr(uint8_t index) {
	if (index < str_list_len) {
		return str_list[index];
	}

	return NULL;
}


uint8_t str_com(char *s1, char *s2){
	while(*s1 != NULL && *s1 == *s2){
		s1++;
		s2++;
	}
	return *s1 - *s2;
}
