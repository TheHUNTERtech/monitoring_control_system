#ifndef __APP_CONFIG_H__
#define __APP_CONFIG_H__

#include <stdint.h>
#include <string>

#include "app.h"
#include "app_data.h"
#include "../common/global_parameters.h"


using namespace std;

#define MAX_MESSAGE_LEN		2000
const char g_app_version[] = "1.0";

typedef struct {
	/* parameter for gateway*/
	//	uint8_t gateway_name[CONFIGURE_PARAMETER_BUFFER_SIZE];
	uint32_t broadcast_udp_port;
	char mqtt_host[CONFIGURE_PARAMETER_BUFFER_SIZE];
	uint32_t mqtt_port;

	char mqtt_username[CONFIGURE_PARAMETER_BUFFER_SIZE];
	char mqtt_password[CONFIGURE_PARAMETER_BUFFER_SIZE];
	char mqtt_topic[CONFIGURE_PARAMETER_BUFFER_SIZE];
	char token_communicate[CONFIGURE_PARAMETER_BUFFER_SIZE];
	char security_key[CONFIGURE_PARAMETER_BUFFER_SIZE];
	uint8_t start_up_success;

} config_parameter_t;

class app_config {
public:
	app_config();
	void initializer(char*);
	void set_config_path_file(char*);
	void get_config_path_file(char*);
	int parser_config_file(void*);
	int write_config_data(void*);

private:
	char m_config_path[256];
};

#endif //__APP_CONFIG_H__
