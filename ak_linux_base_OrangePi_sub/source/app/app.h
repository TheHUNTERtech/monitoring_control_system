#ifndef __APP_H__
#define __APP_H__

#include <string>

#include "../ak/ak.h"

#include "app_config.h"

using namespace std;

extern app_config gateway_configure;
extern config_parameter_t g_config_parameters;
extern app_zigbee_device_list_t zigbee_sensor_list;
/*****************************************************************************/
/* task GW_SYS define.
 */
/*****************************************************************************/
/* define timer */
/* define signal */
enum {
    GW_SYS_WATCH_DOG_REPORT_REQ = AK_USER_DEFINE_SIG,
};

/*****************************************************************************/
/* task MT_HANDLE_MSG define.
 */
/*****************************************************************************/
/* define signal */
#define	HANDLE_MESSAGE_DATA_INCOMING				(1)
#define MT_ZIGBEE_SENSOR_JOIN						(2)
#define MT_ZIGBEE_SENSOR_DATA						(3)

/*****************************************************************************/
/*  task GW_RF24 define.
 */
/*****************************************************************************/
/* define timer */
#define GW_RF24_IF_TIMER_PACKET_DELAY_INTERVAL		(100)

/* define signal */
enum {
    GW_RF24_IF_PURE_MSG_OUT = AK_USER_DEFINE_SIG,
    GW_RF24_IF_COMMON_MSG_OUT,
    GW_RF24_IF_TIMER_PACKET_DELAY,
};

/*****************************************************************************/
/*  task GW_CONSOLE define
 */
/*****************************************************************************/
/* define timer */

/* define signal */
enum {
    GW_CONSOLE_INTERNAL_LOGIN_CMD = AK_USER_DEFINE_SIG,
};

/*****************************************************************************/
/* task GW_IF define
 */
/*****************************************************************************/
/* define timer */
/* define signal */
enum {
    GW_IF_PURE_MSG_IN = AK_USER_DEFINE_SIG,
    GW_IF_PURE_MSG_OUT,
    GW_IF_COMMON_MSG_IN,
    GW_IF_COMMON_MSG_OUT,
    GW_IF_DYNAMIC_MSG_IN,
    GW_IF_DYNAMIC_MSG_OUT,
};

/*****************************************************************************/
/* task GW_FW define
 */
/*****************************************************************************/
/* define timer */
#define GW_TIMER_FIRMWARE_PACKED_TIMEOUT_INTERVAL			(300)		/* 300ms */
#define GW_TIMER_FIRMWARE_GET_FIRMWARE_INFO_TIMEOUT			(2000)		/* 2s */
#define GW_TIMER_FIRMWARE_DEV_INTERNAL_UPDATE_TIMEOUT		(180000)	/* 3' */

/* define signal */
enum {
    GW_FW_OTA_REQ = AK_USER_DEFINE_SIG,
    GW_FW_CURRENT_INFO_RES,
    GW_FW_UPDATE_RES_OK,
    GW_FW_TRANFER_REQ,
    GW_FW_TRANSFER_RES_OK,
    GW_FW_TRANSFER_CHECKSUM_ERR,
    GW_FW_INTERNAL_UPDATE_REQ,
    GW_FW_UPDATE_BUSY,
    GW_FW_PACKED_TIMEOUT,
    GW_FW_UPDATE_COMPLETED,
    GW_FW_SM_UPDATE_RES_OK,
    GW_FW_SM_RELEASE_RES_OK,
    GW_FW_GET_FIRMWARE_INFO_TIMEOUT,
    GW_FW_DEV_INTERNAL_UPDATE_TIMEOUT,
};

/*****************************************************************************/
/*  task GW_TASK_DEBUG_MSG define
 */
/*****************************************************************************/
/* define timer */
/* define signal */
enum {
    GW_DEBUG_MSG_1 = AK_USER_DEFINE_SIG,
    GW_DEBUG_MSG_2
};

/*****************************************************************************/
/*  global define variable
 */
/*****************************************************************************/
#define APP_OK										(0x00)
#define APP_NG										(0x01)

#define APP_FLAG_OFF								(0x00)
#define APP_FLAG_ON									(0x01)

#define APP_ROOT_PATH_DISK							"/root/smarthome_gateway"
#define APP_ROOT_PATH_RAM							"/run/smarthome_gateway"

extern app_config gateway_configure;

#endif // __APP_H__
