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
#include "task_pub_sub.h"

#include "mqtt_communication.h"

q_msg_t gw_task_pub_sub_mailbox;

void* gw_task_pub_sub_entry(void*) {
	task_mask_started();
	wait_all_tasks_started();

	APP_DBG("[STARTED] gw_task_pub_sub_entry\n");

	//	mqtt_add_gateway* mqttaddgateway = NULL;
	mqtt_communication* mqttcomm = NULL;

	/*Init Gateway config*/
	APP_PRINT("Init Gateway config\n");
	strcpy(g_config_parameters.mqtt_host, "broker.hivemq.com");
	strcpy(g_config_parameters.mqtt_username, "fanning");
	strcpy(g_config_parameters.mqtt_topic, "temperature/fanning");
	strcpy(g_config_parameters.mqtt_password, "whydoesitrain?");
	g_config_parameters.mqtt_port = 1883;

	/*Add gateway mqtt*/
	string client_id_communication = string("iot1");
	mqttcomm = new mqtt_communication((const char*) client_id_communication.data(), (char*) g_config_parameters.mqtt_host, g_config_parameters.mqtt_port, (char*) g_config_parameters.mqtt_username, (char*) g_config_parameters.mqtt_password);

	string comm_topic_publish = string((char*) g_config_parameters.mqtt_topic);
	mqttcomm->set_topic_publish(comm_topic_publish.data());

	while (1) {
		while (msg_available(GW_TASK_PUB_SUB_ID)) {
			/* get messge */
			ak_msg_t* msg = rev_msg(GW_TASK_PUB_SUB_ID);

			switch (msg->header->sig) {

			case 1: {

				APP_PRINT("information: \n");
				APP_PRINT(" user name: %s \n", g_config_parameters.mqtt_username);
				APP_PRINT(" password:%s \n", g_config_parameters.mqtt_password);
				APP_PRINT(" host: %s \n", g_config_parameters.mqtt_host);
				APP_PRINT(" port: %d \n", g_config_parameters.mqtt_port);
				APP_PRINT(" key: %s \n", g_config_parameters.security_key);

				const char* data = "Why ?";
				uint32_t data_len = strlen(data);

				mqttcomm->communication_public(data, data_len, true);
			}
				break;

			case 2: {
				/*Init Gateway config*/
				APP_PRINT("Init Gateway config\n");
				strcpy(g_config_parameters.mqtt_host, "broker.hivemq.com");
				strcpy(g_config_parameters.mqtt_username, "fanning");
				strcpy(g_config_parameters.mqtt_topic, "temperature/fanning");
				strcpy(g_config_parameters.mqtt_password, "whydoesitrain?");
				g_config_parameters.mqtt_port = 1883;

				/*Add gateway mqtt*/
				string client_id_communication = string("iot1");
				mqttcomm = new mqtt_communication((const char*) client_id_communication.data(), (char*) g_config_parameters.mqtt_host, g_config_parameters.mqtt_port, (char*) g_config_parameters.mqtt_username, (char*) g_config_parameters.mqtt_password);

				string comm_topic_publish = string((char*) g_config_parameters.mqtt_topic);
				mqttcomm->set_topic_publish(comm_topic_publish.data());

				//mosquitto_sub -d -p 1883 -t temperature/fanning -h broker.hivemq.com

			}
				break;

			case 3: {
				/*Init Gateway config*/
				APP_PRINT("Init Gateway config\n");
				strcpy(g_config_parameters.mqtt_host, "broker.hivemq.com");
				strcpy(g_config_parameters.mqtt_username, "fanning");
				strcpy(g_config_parameters.mqtt_topic, "temperature/fanning");
				strcpy(g_config_parameters.mqtt_password, "whydoesitrain?");
				g_config_parameters.mqtt_port = 1883;

				/*Add gateway mqtt*/
				string client_id_communication = string("iot2");
				mqttcomm = new mqtt_communication((const char*) client_id_communication.data(), (char*) g_config_parameters.mqtt_host, g_config_parameters.mqtt_port, (char*) g_config_parameters.mqtt_username, (char*) g_config_parameters.mqtt_password);

				string comm_topic_sub = string((char*) g_config_parameters.mqtt_topic);
				mqttcomm->set_topic_subcribe(comm_topic_sub.data());

				//mosquitto_sub -d -p 1883 -t temperature/fanning -h broker.hivemq.com

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
