/*
 * if_znp_rx.cpp
 *
 *  Created on: Apr 24, 2017
 *      Author: TuDT13
 */

#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/signal.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <semaphore.h>

#include "../ak/ak.h"

#include "../sys/sys_dbg.h"

#include "app.h"
#include "app_if.h"
#include "app_dbg.h"
#include "app_data.h"

#include "task_list.h"
#include "task_list_if.h"
#include "if_znp.h"
#include "znp_serial.h"
#include "../common/global_parameters.h"
// Start of frame character value
#define SOF_CHAR			0xFE

//COMPORT name: ttyACM0
#define DEVICE_PATH			"/dev/ttyACM0"

#define RX_BUFFER_SIZE		256


q_msg_t mt_task_zigbee_mng_mailbox;
void* mt_task_zigbee_mng_entry(void*);

int8_t flagAddDevice = 0; /*bit 0: 0x45CA, bit 1: 0x45C1*/
uint16_t g_short_address_new_device = 0;
int if_znp_fd;
struct termios options;
static int if_znp_rx_opentty(const char* devpath);
static void if_znp_rx_closetty();
static uint8_t if_znp_rx_calcfcs(uint8_t len, uint8_t cmd0, uint8_t cmd1, uint8_t *data_ptr);
static uint8_t if_znp_tx_calcfcs(uint8_t len, uint8_t *data_ptr);
static pthread_t if_znp_rx_thread;
static void* if_znp_rx_thread_handler(void*);

void permitJoinAddDevice(if_parseinfo_frame_t parseinfo_data);
void leaveDeviceResponse(if_parseinfo_frame_t parseinfo_data);

#define SOP_STATE      0x00
#define LEN_STATE      0x01
#define CMD0_STATE     0x02
#define CMD1_STATE     0x03
#define DATA_STATE     0x04
#define FCS_STATE      0x05

//static uint8_t rx_frame_state = SOP_STATE;

#define RX_FRAME_PARSER_FAILED			(-1)
#define RX_FRAME_PARSER_SUCCESS			(0)
#define RX_FRAME_PARSER_rx_remain		(1)

static if_parseinfo_frame_t if_parseinfo_frame;
static void rx_frame_parser(uint8_t* data, uint8_t len);
static int tx_frame_post(uint8_t* data, uint8_t len);

static uint8_t tx_buffer[1024];

void* mt_task_zigbee_mng_entry(void*) {

	int bSuccess;
	task_mask_started();
	wait_all_tasks_started();


	APP_DBG("[STARTED] gw_task_if_znp_entry\n");    //b1.1: Cấu hình cổng COM

	while (1) {
		if (if_znp_rx_opentty(DEVICE_PATH) < 0) {
			APP_DBG("Cannot open %s !\n", DEVICE_PATH);
		} else {
			APP_DBG("Opened %s success !\n", DEVICE_PATH);
			// initialize UART receive thread related variables
			// TODO: Start coordinator here
			printf("Start coordinator here !\n");

			bSuccess = startZigbeeCoordinator_test(if_znp_fd);          /* bắt đầu gửi khung truyền */
			if (bSuccess == 0) {
				printf("Start coordinator successfully !\n");
				break;
			}
		}
		if_znp_rx_closetty();
		usleep(100);
	}

	APP_DBG("Opened %s success !\n", DEVICE_PATH);
	pthread_create(&if_znp_rx_thread, NULL, if_znp_rx_thread_handler, NULL);

	while (1) {
		while (msg_available(MT_TASK_ZIGBEE_MNG_ID)) {
			/* get message */
			ak_msg_t* msg = rev_msg(MT_TASK_ZIGBEE_MNG_ID);

			switch (msg->header->sig) {
			case ZB_PERMIT_JOINING_REQUEST: {   //Cho phep them Device

				printf("if_znp: add new device!\n");

				if (setPermitJoiningReq(if_znp_fd, SHORT_ADDRESS_COORDINATOR_ADD_ROUTER, *(uint8_t *)msg->header->payload, 1) != ZNP_SUCCESS) {
					printf("ERROR: request add new device!\n");
				}
			}
				break;
			case ZDO_MGMT_LEAVE_REQ: {
				printf("ZDO_MGMT_LEAVE_REQ\n");
				zdo_mgmt_leave_req_t *zdoMgmtLeaveReq;
				uint8_t* data = (uint8_t *) msg->header->payload;
				zdoMgmtLeaveReq = (zdo_mgmt_leave_req_t *) data;
				//zdoMgmtLeaveReq->short_address = (uint16_t)0xAA5D;
				//				memcpy(zdoMgmtLeaveReq->device_address, data, 8);
				//				zdoMgmtLeaveReq->flags = 0x01;

				if (zdo_mgmt_leave_req(if_znp_fd, zdoMgmtLeaveReq->short_address, zdoMgmtLeaveReq->device_address, zdoMgmtLeaveReq->flags) == ZNP_SUCCESS) {
					printf("SUCCESS!\n");
				} else {
					printf("ERROR: zdo_mgmt_leave_req!\n");
				}

			}
				break;
			case UTIL_GET_DEVICE_INFO: {
				util_get_device_info(if_znp_fd);		                                                          // get short addr list
			}
				break;
			case AF_INCOMING_MSG: {
				printf("AF_INCOMING_MSG 0x4481 \n\n");
				if_parseinfo_frame_t parseinfo_data;
				get_data_dynamic_msg(msg, (uint8_t*) &parseinfo_data, sizeof(if_parseinfo_frame_t));

				ak_msg_t* s_msg = get_dynamic_msg();
				set_msg_sig(s_msg, HANDLE_MESSAGE_DATA_INCOMING);
				set_data_dynamic_msg(s_msg, (uint8_t*) &parseinfo_data.data[0], msg->header->len);

				af_incoming_msg_t *pkt;
				pkt = (af_incoming_msg_t *) &parseinfo_data.data[0];

				if (zcl_ProcessMessageMSG(pkt) == ZCL_PROC_SUCCESS) {
					printf("TODO: parse data to json structure and send to MQTT\n");
				} else {
					printf("It is not ZCL data\n");
				}

			}
				break;
			case ZDO_MGMT_LEAVE_RSP: {
				printf("ZDO_MGMT_LEAVE_RSP 0x45B4 \n\n");
				if_parseinfo_frame_t parseinfo_data;
				get_data_dynamic_msg(msg, (uint8_t*) &parseinfo_data, sizeof(if_parseinfo_frame_t));
				leaveDeviceResponse(parseinfo_data);
			}
				break;
			case AF_DATA_REQUEST: {
				printf("AF_DATA_REQUEST\n");

				uint8_t *data = (uint8_t*) msg->header->payload;
				af_data_request_t afDataRequest = *(af_data_request_t*) data;

				for (int j = 0; j < afDataRequest.len; j++) {
					printf(" %02X", *(afDataRequest.data + j));
				}
				printf("\n");
				send_af_data_request(if_znp_fd, afDataRequest);
			}
				break;
			case ZDO_END_DEVICE_ANNCE_IND: {
				printf("ZDO_END_DEVICE_ANNCE_IND 0x45C1\n");

				if_parseinfo_frame_t parseinfo_data;
				get_data_dynamic_msg(msg, (uint8_t*) &parseinfo_data, sizeof(if_parseinfo_frame_t));

				for (int i = 0; i < parseinfo_data.tempDataLen; i++) {
					printf(" %02X ", parseinfo_data.data[i]);
				}

				printf("\n");
				flagAddDevice = (flagAddDevice | 0x02);

				if (flagAddDevice == 0x03) {
					for (int i = 0; i < 10; i++) {
						parseinfo_data.data[i] = parseinfo_data.data[i + 2];
					}
					parseinfo_data.tempDataLen = 10;
					permitJoinAddDevice(parseinfo_data);
				}
				g_short_address_new_device = *(uint16_t*) &parseinfo_data.data[0];
			}
				break;

			case ZDO_TC_DEV_IND: {
				printf("ZDO_TC_DEV_IND 0x45CA \n\n");
				if_parseinfo_frame_t parseinfo_data;
				get_data_dynamic_msg(msg, (uint8_t*) &parseinfo_data, sizeof(if_parseinfo_frame_t));
				printf("Dia chi MAC\t");
				for (int i = 0; i < parseinfo_data.tempDataLen; i++) {
					printf(" %02X ", parseinfo_data.data[i]);
				}
				printf("\n");
				/*when receiving this signal meaning the new advice join network, need send MAC address to mqtt.
				 * Save MAC address to binding table*/
				flagAddDevice = (flagAddDevice | 0x01);
				parseinfo_data.tempDataLen = 10;
				permitJoinAddDevice(parseinfo_data);
				g_short_address_new_device = *(uint16_t*) &parseinfo_data.data[0];
				//printf("Short Address = %02X\n",g_short_address_new_device);
			}
				break;
			case UTIL_GET_DEVICE_INFO_RESPONSE: {
				printf("UTIL_GET_DEVICE_INFO_RESPONSE 0x6700 \n\n");
				/*when receiving this signal meaning the new advice join network, need send MAC address to mqtt.
				 * Save MAC address to binding table*/
				if_parseinfo_frame_t parseinfo_data;
				get_data_dynamic_msg(msg, (uint8_t*) &parseinfo_data, sizeof(if_parseinfo_frame_t));
				//				ak_msg_t* s_msg = get_dymanic_msg();
				//				set_msg_sig(s_msg, MT_CONSOLE_GET_LIST_ZED_RESPONSE);
				//				set_data_dynamic_msg(s_msg, (uint8_t*) &parseinfo_data, sizeof(if_parseinfo_frame_t));

				//				set_msg_src_task_id(s_msg, MT_TASK_HANDLE_MSG_ID);
				//				task_post(MT_TASK_DOOR_MNG_ID, s_msg);
			}
				break;
			case ZDO_IEEE_ADDR_RSP: {
				printf("ZDO_IEEE_ADDR_RSP  0x4581 \n\n");
				if_parseinfo_frame_t parseinfo_data;
				get_data_dynamic_msg(msg, (uint8_t*) &parseinfo_data, sizeof(if_parseinfo_frame_t));
				/*when receiving this signal meaning the new advice join network, need send MAC address to mqtt.
				 * Save MAC address to binding table*/
				if (parseinfo_data.data[0] == ZNP_SUCCESS) {
					bytestoHexChars((uint8_t*) &parseinfo_data.data[1], 8, g_strTokenFactory);
					g_strTokenFactory[16] = '\0';
					//printf("MAC address = %s\n", g_strTokenFactory);
				}
			}
				break;
			default: {
				if_parseinfo_frame_t parseinfo_data;
				get_data_dynamic_msg(msg, (uint8_t*) &parseinfo_data, sizeof(if_parseinfo_frame_t));
				if (((msg->header->sig & 0xE000/*RPC_CMD_TYPE_MASK*/) == 0x6000 /*RPC_CMD_SRSP*/) || ((msg->header->sig & 0x1F00/*RPC_SUBSYSTEM_MASK*/) == 0x0D00 /*RPC_SYS_BOOT*/)) {

					if (parseinfo_data.data[0] == 0) {
						APP_DBG("SUCCESSFUL!\n");
					} else {
						APP_DBG("FAIL: %02X\n", parseinfo_data.data[0]);
					}
				} else {

					printf("cmd: %04X -data:", msg->header->sig);
					for (int i = 0; i < parseinfo_data.tempDataLen; i++) {
						printf(" %02X", parseinfo_data.data[i]);
					}
					printf("\n");
				}
			}
				break;
			}
			/* free message */
			free_msg(msg);
		}

		usleep(1000);
	}

	return (void*) 0;
}

void* if_znp_rx_thread_handler(void*) {
	APP_DBG("if_znp_rx_thread_handler entry successfully!\n");
	uint8_t rx_buffer[RX_BUFFER_SIZE];
	int32_t rx_read_len;

	if_parseinfo_frame.state = SOP_STATE;
	while (1) {

		rx_read_len = read(if_znp_fd, rx_buffer, RX_BUFFER_SIZE);
		if (rx_read_len > 0) {

			printf("go to if_znp_rx_thread_handler Test\n");
			//            printf("raw data:%X : \n",rx_buffer[18]);

			//			printf("len: =  %02X \n",rx_read_len);
			for (int i = 0; i < rx_read_len; i++) {
				printf(" %02X ", rx_buffer[i]);
			}
			printf("\n");
			printf("go to if_znp_rx_thread_handler Test\n");

			rx_frame_parser(rx_buffer, rx_read_len);
		}
		usleep(100);
	}

	return (void*) 0;
}

int if_znp_rx_opentty(const char* devpath) {

	APP_DBG("[IF znp_rx][if_znp_rx_opentty] devpath: %s\n", devpath);

	if_znp_fd = open(devpath, O_RDWR | O_NOCTTY | O_NDELAY);
	if (if_znp_fd < 0) {
		return if_znp_fd;
	} else {
		fcntl(if_znp_fd, F_SETFL, 0);

		/* get current status */
		tcgetattr(if_znp_fd, &options);

		cfsetispeed(&options, B115200);
		cfsetospeed(&options, B115200);

		//		cfsetispeed(&options, B38400);
		//		cfsetospeed(&options, B38400);

		/* No parity (8N1) */
		options.c_cflag &= ~PARENB;
		options.c_cflag &= ~CSTOPB;
		options.c_cflag &= ~CSIZE;
		options.c_cflag |= CS8;

		options.c_cflag |= (CLOCAL | CREAD);
		options.c_cflag &= ~CRTSCTS;
		options.c_cc[VMIN] = 1;		// read doesn't block
		options.c_cc[VTIME] = 5;		// 0.5 seconds read timeout
		cfmakeraw(&options);

		tcflush(if_znp_fd, TCIFLUSH);
		if (tcsetattr(if_znp_fd, TCSANOW, &options) != 0) {
			SYS_DBG("error in tcsetattr()\n");
			return -1;
		}
	}

#ifdef mqtt
	struct termios tty;
	struct termios tty_old;
	int r = 1;

	memset(&tty, 0, sizeof(tty));

	printf("Serial device [%s]\n", devpath);
	if_znp_fd = open(devpath, O_RDWR | O_NOCTTY | O_NONBLOCK);

	if (tcgetattr(if_znp_fd, &tty) != 0) {
		printf("Error in tcgetattr()\n");
		return -1;
	}
	if (r == 1) {
		// Save old tty parameters
		tty_old = tty;

		// Set Baud Rate
		cfsetospeed(&tty, B115200);
		cfsetispeed(&tty, B115200);

		// Setting other Port Stuff
		tty.c_cflag &= ~PARENB;// Make 8n1
		tty.c_cflag &= ~CSTOPB;
		tty.c_cflag &= ~CSIZE;
		tty.c_cflag |= CS8;

		tty.c_cflag &= ~CRTSCTS;// no flow control
		tty.c_cc[VMIN] = 1;// read doesn't block
		tty.c_cc[VTIME] = 5;// 0.5 seconds read timeout
		tty.c_cflag |= CREAD | CLOCAL;// turn on READ & ignore ctrl lines

		// Make raw
		cfmakeraw(&tty);

		// Flush Port, then applies attributes
		tcflush(if_znp_fd, TCIFLUSH);
		if (tcsetattr(if_znp_fd, TCSANOW, &tty) != 0) {
			printf("Error in tcsetattr()\n");
			return -1;
		}
	}
#endif
	return 0;
}

/* Close open device */
static void if_znp_rx_closetty(void) {
	/* revert to old settings */
	tcsetattr(if_znp_fd, TCSANOW, &options);

	/* close the device */
	close(if_znp_fd);
}
/* Calculate if znp receiving frame FCS */
static uint8_t if_znp_rx_calcfcs(uint8_t len, uint8_t cmd0, uint8_t cmd1, uint8_t *data_ptr) {
	uint8_t x;
	uint8_t xorResult;

	xorResult = len ^ cmd0 ^ cmd1;

	for (x = 0; x < len; x++, data_ptr++)
		xorResult = xorResult ^ *data_ptr;

	return (xorResult);
}
void rx_frame_parser(uint8_t* data, uint8_t len) {
	uint8_t ch;
	int rx_remain;

	while (len) {
		ch = *data++;
		len--;

		switch (if_parseinfo_frame.state) {
		case SOP_STATE:
			if (SOF_CHAR == ch) {
				if_parseinfo_frame.state = LEN_STATE;   // 1
			}
			break;

		case LEN_STATE:
			if_parseinfo_frame.len = ch;
			if_parseinfo_frame.tempDataLen = 0;
			/* Fill up what we can */
			if_parseinfo_frame.state = CMD0_STATE;
			break;

		case CMD0_STATE:
			if_parseinfo_frame.cmd0 = ch;
			if_parseinfo_frame.state = CMD1_STATE;

			break;
		case CMD1_STATE:
			if_parseinfo_frame.cmd1 = ch;
			/* If there is no data, skip to FCS state */
			if (if_parseinfo_frame.len) {
				if_parseinfo_frame.state = DATA_STATE;
			} else {
				if_parseinfo_frame.state = FCS_STATE;
			}
			break;

		case DATA_STATE: {
			if_parseinfo_frame.data[if_parseinfo_frame.tempDataLen++] = ch;

			rx_remain = if_parseinfo_frame.len - if_parseinfo_frame.tempDataLen;

			if (len >= rx_remain) {
				memcpy((uint8_t*) (if_parseinfo_frame.data + if_parseinfo_frame.tempDataLen), data, rx_remain);
				if_parseinfo_frame.tempDataLen += rx_remain;
				len -= rx_remain;
				data += rx_remain;
			} else {
				memcpy((uint8_t*) (if_parseinfo_frame.data + if_parseinfo_frame.tempDataLen), data, len);
				if_parseinfo_frame.tempDataLen += len;
				len = 0;
			}

			if (if_parseinfo_frame.len == if_parseinfo_frame.tempDataLen) {
				if_parseinfo_frame.state = FCS_STATE;
			}
		}
			break;

		case FCS_STATE: {
			if_parseinfo_frame.state = SOP_STATE;

			if_parseinfo_frame.frame_fcs = ch;

			if (if_parseinfo_frame.frame_fcs == if_znp_rx_calcfcs(if_parseinfo_frame.len, if_parseinfo_frame.cmd0, if_parseinfo_frame.cmd1, if_parseinfo_frame.data)) {

				uint16_t tempSig = (uint16_t)((if_parseinfo_frame.cmd1) + 0x0100 * (if_parseinfo_frame.cmd0));
				ak_msg_t* s_msg = get_dynamic_msg();
				printf("Show temp Signal = %X\n",tempSig);
				set_msg_sig(s_msg, tempSig);
				set_data_dynamic_msg(s_msg, (uint8_t*) &if_parseinfo_frame, sizeof(if_parseinfo_frame_t));

				set_msg_src_task_id(s_msg, MT_TASK_ZIGBEE_MNG_ID);
				task_post(MT_TASK_ZIGBEE_MNG_ID, s_msg);

			} else {
				/* TODO: handle checksum incorrect */
				APP_DBG("ERROR: checksum incorrect!\n");
			}
		}
			break;

		default:
			break;
		}
	}
}

/* Calculate IF_ZNP TX frame FCS */
uint8_t if_znp_tx_calcfcs(uint8_t len, uint8_t *data_ptr) {
	uint8_t xor_result;
	xor_result = len;

	for (int i = 0; i < len; i++, data_ptr++) {
		xor_result = xor_result ^ *data_ptr;
	}

	return xor_result;
}

int tx_frame_post(uint8_t* data, uint8_t len) {
	tx_buffer[0] = SOF_CHAR;
	tx_buffer[1] = len;
	memcpy(&tx_buffer[2], data, len);
	tx_buffer[2 + len] = if_znp_tx_calcfcs(len, data);
	return write(if_znp_fd, tx_buffer, (len + 3));
}

void permitJoinAddDevice(if_parseinfo_frame_t parseinfo_data) {
	//if (flagAddDevice == 0x03) {
	//		if (g_short_address_new_device != *(uint16_t*) &parseinfo_data.data[0]) {
	//			flagAddDevice == 0x00;
	//			return;
	//		}
	setPermitJoiningReq(if_znp_fd, SHORT_ADDRESS_COORDINATOR, DISABLE_PERMIT_JOIN, 1);

	uint8_t client_message[MAX_MESSAGE_LEN];
	int32_t i = 0;
	uint8_t d = 0;
	//add token communicate
	client_message[i] = SEPARATE_CHAR;
	i++;
	memcpy((uint8_t*) &client_message[i], g_strTokenCommunicate.c_str(), g_strTokenCommunicate.size());
	//printf("1 str add-----------------> %d  ----->",client_message[i]);
	//printf(" %s\n",g_strTokenCommunicate.c_str());
	i += g_strTokenCommunicate.size();

	//add time
	client_message[i] = SEPARATE_CHAR;
	i++;
	time_t rawtime = time(NULL);
	bytestoHexChars((uint8_t*) &rawtime, sizeof(time_t), (uint8_t*) &client_message[i]);
	//printf("2 str add-----------------> %02X  ----->",client_message[i]);
	//printf(" %s\n",g_strTokenCommunicate.c_str());
	i += sizeof(time_t) * 2;

	//add command adddevice
	client_message[i] = SEPARATE_CHAR;
	i++;
	memcpy((uint8_t*) &client_message[i], g_strCommand[adddevice].c_str(), g_strCommand[adddevice].size());
	i += g_strCommand[adddevice].size();
	//add data
	client_message[i] = SEPARATE_CHAR;
	i++;

	//device id 2 bytes short address
	bytestoHexChars((uint8_t*) parseinfo_data.data, 2, (uint8_t*) &client_message[i]);
	printf("SHORT ADDRESS: ");
	printf("%02X ",(uint8_t*) parseinfo_data.data[0]);
	printf("%02X \n",(uint8_t*) parseinfo_data.data[1]);
	i += 2 * 2;
	client_message[i] = SEPARATE_CHAR;
	i++;
	printf("MAC ADRESS: ");
	for (int x = 2;x<=9;x++) {
		printf("%02X ",(uint8_t*) parseinfo_data.data[x]);
	}
	printf("\n");
	bytestoHexChars((uint8_t*) &parseinfo_data.data[2], 8, (uint8_t*) &client_message[i]);
	i += 2 * 8;
	client_message[i] = '\0';
	i++;

	{
		uint8_t *data = (uint8_t*) client_message;
		ak_msg_t* s_msg = get_dynamic_msg();
		set_msg_sig(s_msg, MT_ZIGBEE_SENSOR_JOIN);
		set_data_dynamic_msg(s_msg, data, i);
		set_msg_src_task_id(s_msg, MT_TASK_ZIGBEE_MNG_ID);
		task_post(MT_TASK_HANDLE_MSG_ID, s_msg);
	}

	//flagAddDevice = 0x00;
}

void leaveDeviceResponse(if_parseinfo_frame_t parseinfo_data) {

	uint8_t client_message[MAX_MESSAGE_LEN];
	int32_t i = 0;
	//add token communicate
	client_message[i] = SEPARATE_CHAR;
	i++;
	memcpy((uint8_t*) &client_message[i], g_strTokenCommunicate.c_str(), g_strTokenCommunicate.size());
	i += g_strTokenCommunicate.size();

	//add time
	client_message[i] = SEPARATE_CHAR;
	i++;
	time_t rawtime = time(NULL);
	bytestoHexChars((uint8_t*) &rawtime, sizeof(time_t), (uint8_t*) &client_message[i]);
	i += sizeof(time_t) * 2;

	//add command remove device
	client_message[i] = SEPARATE_CHAR;
	i++;
	memcpy((uint8_t*) &client_message[i], g_strCommand[removedevice].c_str(), g_strCommand[removedevice].size());
	i += g_strCommand[removedevice].size();
	//add data (device id 2bytes)
	client_message[i] = SEPARATE_CHAR;
	i++;
	bytestoHexChars((uint8_t*) parseinfo_data.data, parseinfo_data.tempDataLen, (uint8_t*) &client_message[i]);
	i += parseinfo_data.tempDataLen * 2;
	client_message[i] = '\0';
	i++;
	{
		//		uint8_t *data = (uint8_t*) client_message;
		//		ak_msg_t* s_msg = get_dynamic_msg();
		//		set_msg_sig(s_msg, IF_SOCKET_LEAVE_DEVICE_RESPONSE);
		//		set_data_dynamic_msg(s_msg, data, i);

		//		set_msg_src_task_id(s_msg, MT_TASK_ZIGBEE_MNG_ID);
		//		task_post(MT_TASK_IF_SOCKET_ID, s_msg);
	}

	{
		uint8_t *data = (uint8_t*) client_message;
		ak_msg_t* s_msg = get_dynamic_msg();
		//set_msg_sig(s_msg, LOG_SENSOR_DATA);
		set_data_dynamic_msg(s_msg, data, i);

		//set_msg_src_task_id(s_msg, ID_TASK_IF_ZNP);
		//task_post(ID_TASK_LOG, s_msg);
	}
}
