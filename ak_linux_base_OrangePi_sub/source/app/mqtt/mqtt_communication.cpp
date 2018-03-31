#include <string.h>

#include "../ak/ak.h"

#include "mqtt_communication.h"

#include "app.h"
#include "app_dbg.h"
#include "app_data.h"


#include "task_list.h"

uint8_t* lastSensorControlMessage;
uint32_t len_lastControl = 0;
int8_t flag_update_time = 1;
/*clean section = true: mean every reconnect or start need to subcribe all topics*/
mqtt_communication::mqtt_communication(const char *id, const char *host, int port, char* username, char* password) :
	mosquittopp(id, true) {
	/* init private data */
	m_connect_ok_flag = -1;
	m_mid = 1;

	/* init mqtt */
	mosqpp::lib_init();

//	username_pw_set(username, password);

	string comm_topic_status = string((char*) g_config_parameters.mqtt_topic) + string("/status/");
//	APP_PRINT(" comm_topic_status: %s \n", comm_topic_status.data());

	uint8_t *offline = (uint8_t *) malloc(MAX_MESSAGE_LEN * sizeof(uint8_t));
	int i = 0;
	offline[i] = SEPARATE_CHAR;
	i++;
	memcpy((uint8_t*) &offline[i], "offline", strlen("offline"));
	i += strlen("offline");
	offline[i] = SEPARATE_CHAR;
	i++;
	memcpy((uint8_t*) &offline[i], (uint8_t*) g_app_version, strlen(g_app_version));
	i += strlen(g_app_version);
	offline[i] = '\0';
	//i++;

	//setKey((unsigned char*) g_config_parameters.security_key, 32);
	//int encrypted_offline = iot_final_encrypt((uint8_t*) offline, i, g_config_parameters.security_key);

	will_set(comm_topic_status.data(), i, offline, 1, true);
	free(offline);

	/* connect */
	connect_async(host, port, 60);
	loop_start();
}

mqtt_communication::~mqtt_communication() {
	loop_stop();
	mosqpp::lib_cleanup();
}

void mqtt_communication::set_topic_subcribe(const char* topic) {
	//	m_topic_subscribe = static_cast<string>(topic);
	subscribe(NULL, topic, 1);
}

void mqtt_communication::set_topic_publish(const char* topic) {
	m_topic_publish = static_cast<string>(topic);
}
void mqtt_communication::on_connect(int rc) {

}

void mqtt_communication::communication_public(const char* msg, uint32_t len, bool retain) {
	APP_DBG("[mqtt_communication][public] msg:%s len:%d\n", msg, len);
	if (publish(&m_mid, m_topic_publish.data(), len, msg, 1, true) != MOSQ_ERR_SUCCESS) {
		printf("public error"); //qos = 1
	}
}

void mqtt_communication::on_publish(int mid) {
	APP_DBG("[mqtt_communication][on_publish] mid: %d\n", mid);
}

void mqtt_communication::on_subscribe(int mid, int qos_count, const int *granted_qos) {
	(void) granted_qos;

	APP_DBG("[mqtt_communication][on_subscribe] mid:%d\tqos_count:%d\n", mid, qos_count);
}

void mqtt_communication::on_message(const struct mosquitto_message *message) {
	//	if (!strcmp(message->topic, m_topic_subscribe.data())) {

	if (message->payloadlen > 0) {

			APP_DBG("[mqtt_communication][on _message] topic:%s\tpayloadlen:%d\n", message->topic, message->payloadlen);
			char *p = (char*) message->payload;
			for (int i = 0; i < message->payloadlen; i++) {
					APP_DBG("%c", p[i]);
				}
			APP_DBG("\n");
			if (check_message_duplicate(message) == true) {

//					int len = message->payloadlen;

					/* post message to mqtt task */
//					ak_msg_t* s_msg = get_dymanic_msg();
//					set_msg_sig(s_msg, MQTT_DATA_COMMUNICATION);
//					set_data_dynamic_msg(s_msg, (uint8_t*) message->payload, len);

//					set_msg_src_task_id(s_msg, ID_TASK_MQTT);
//					task_post(ID_TASK_MQTT, s_msg);

				} else {
					printf("\n Duplicate message! \n");
				}
			if (message->retain == true) {
					publish(&m_mid, message->topic, 0, NULL, 1, true);
				}
		}
}
bool mqtt_communication::check_message_duplicate(const struct mosquitto_message *message) {
	if (lastSensorControlMessage == NULL) {

		}
	return true;
}
