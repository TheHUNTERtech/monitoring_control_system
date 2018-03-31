#include <sys/types.h>
#include <sys/stat.h>

#include "../ak/ak.h"

#include "app.h"
#include "app_dbg.h"

app_config gateway_configure;
config_parameter_t g_config_parameters;
//app_config_parameter_t gateway_configure_parameter;
app_zigbee_device_list_t zigbee_sensor_list;
void task_init() {
	struct stat st;

	/* create app root path on DISK */
	if (stat(APP_ROOT_PATH_DISK, &st) == -1) {
		mkdir(APP_ROOT_PATH_DISK, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	}

	/* create app root path on RAM */
	if (stat(APP_ROOT_PATH_RAM, &st) == -1) {
		mkdir(APP_ROOT_PATH_RAM, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	}
}
