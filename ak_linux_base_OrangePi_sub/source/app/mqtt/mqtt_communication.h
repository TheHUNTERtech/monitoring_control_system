#ifndef __MQTT_COMMUNICATION_H__
#define __MQTT_COMMUNICATION_H__

#include <stdint.h>
#include <string>
#include <iostream>

#include <mosquittopp.h>
//#include "task_mqtt.h"

using namespace std;

class mqtt_communication: public mosqpp::mosquittopp {
public:
	mqtt_communication(const char *id, const char *host, int port, char* username, char* password);
	~mqtt_communication();

	void set_topic_subcribe(const char* topic);
	void set_topic_publish(const char* topic);
	void communication_public(const char* msg, uint32_t len, bool retain);

	/* call back functions */
	void on_connect(int rc);
	void on_publish(int mid);
	void on_subscribe(int mid, int qos_count, const int *granted_qos);
	void on_message(const struct mosquitto_message *message);
	bool check_message_duplicate(const struct mosquitto_message *message);

private:
//	string m_topic_subscribe;
	string m_topic_publish;
	char m_connect_ok_flag;
	int m_mid;
};

#endif //__MQTT_COMMUNICATION_H__
