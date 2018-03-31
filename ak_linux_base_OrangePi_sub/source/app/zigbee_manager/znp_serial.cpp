#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>

#include <sys/select.h>
#include <termios.h>

#include "znp_serial.h"
#include "if_znp.h"
// For options field of afSendData()
#define AF_MAC_ACK              		0x00		//Require Acknowledgement from next device on route
#define DEFAULT_RADIUS                  0x0F    //Maximum number of hops to get to destination

#define MAX_LEN_ZNP                  	150    //Maximum length znp
uint8_t sequenceNumber = 0;
uint8_t pre_zigbee_key[16] = { '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1' };

uint8_t znpBuf[MAX_LEN_ZNP];

int8_t setTCRequireKeyExchange(int fd, uint8_t bdb_TrustCenterRequireKeyExchange);

uint8_t waitingForMessage(int fd, uint16_t cmd);

uint8_t waitingForStatus(int fd, uint16_t cmd, uint8_t ustatus);

uint8_t calcFCS(uint8_t *pMsg, int8_t len) {
	uint8_t result = 0;
	while (len--) {
		result ^= *pMsg++;
	}
	return result;
}

#define APP_CNF_SET_ALLOWREJOIN_TC_POLICY_PAYLOAD_LEN	0x01
uint8_t app_cnf_set_allowrejoin_tc_policy(int fd, uint8_t mode) {
	int8_t i = 0;
	znpBuf[i] = ZNP_SOF;
	i++;
	znpBuf[i] = APP_CNF_SET_ALLOWREJOIN_TC_POLICY_PAYLOAD_LEN;
	i++;
	znpBuf[i] = MSB(APP_CNF_SET_ALLOWREJOIN_TC_POLICY);
	i++;
	znpBuf[i] = LSB(APP_CNF_SET_ALLOWREJOIN_TC_POLICY);
	i++;
	znpBuf[i] = mode;
	i++;
	znpBuf[i] = calcFCS((uint8_t *) &znpBuf[1], (i - 1));
	i++;
	if (write(fd, znpBuf, i) < 0) {
		printf("ERROR: write");
		return ZNP_NOT_SUCCESS;
	}

	return waitingForMessage(fd, (APP_CNF_SET_ALLOWREJOIN_TC_POLICY | 0x6000));

}
#define APP_CNF_BDB_START_COMMISSIONING_PAYLOAD_LEN	0x01
int8_t app_cnf_bdb_start_commissioning(int fd, uint8_t mode_config, uint8_t mode_receiving, uint8_t flagWaiting) {
	int8_t i = 0;
	znpBuf[i] = ZNP_SOF;
	i++;
	znpBuf[i] = APP_CNF_BDB_START_COMMISSIONING_PAYLOAD_LEN;
	i++;
	znpBuf[i] = MSB(APP_CNF_BDB_START_COMMISSIONING);
	i++;
	znpBuf[i] = LSB(APP_CNF_BDB_START_COMMISSIONING);
	i++;
	znpBuf[i] = mode_config;
	i++;
	znpBuf[i] = calcFCS((uint8_t *) &znpBuf[1], (i - 1));
	i++;
	if (write(fd, znpBuf, i) < 0) {
		printf("ERROR: write");
		return ZNP_NOT_SUCCESS;
	}

	if (flagWaiting == 0) {
		return waitingForStatus(fd, APP_CNF_BDB_COMMISSIONING_NOTIFICATION, ZNP_SUCCESS);
	} else {
		return ZNP_SUCCESS;
	}
}

// 0xfe 0x03 0x4f 0x80 0x00 0x02 success commissioning mode 2 (formation)
//void wait4_commissioning_notification(int fd, uint8_t status, uint8_t mode) {
//	unsigned char buf2[16], buf3[256];
//	int n_read, i1, i2, i3;
//
//	i2 = i3 = 0;
//	while (!i3) {
//		usleep(NORMAL_CMD_WAIT);
//		n_read = read(fd, buf2, 16);
//		if (n_read > 0) {
//			for (i1 = 0; i1 < n_read; i1++) {
//				if (i2 < sizeof(buf3)) {
//					buf3[i2++] = buf2[i1];
//				}
//			}
//
//			//printf("buf3 %d: ", i2);
//			//for (i1 = 0; i1 < i2; i1 ++) {
//			//printf("0x%02x ", buf3[i1] & 0xFF);
//			//}
//			//printf("\n");
//			//fflush(stdout);
//
//			for (i1 = 0; i1 < i2 - 6; i1++) {
//				if (buf3[i1] == 0xFE && buf3[i1 + 1] == 3 && buf3[i1 + 2] == 0x4F && buf3[i1 + 3] == 0x80 && buf3[i1 + 4] == status && buf3[i1 + 5] == mode) {
//					printf(" got APP_CNF_BDB_COMMISIONING_NOTIFICATION message\n");
//					i3 = 1;
//					break;
//				}
//			}
//		}
//	}
//	printf("exit wait4_commissioning_notification()\n");
//}
//
//void wait4_coordinator_up_ok(int fd, uint8_t status, uint8_t device_type, uint8_t device_state) {
//	unsigned char buf2[16], buf3[256];
//	int n_read, i1, i2, i3;
//
//	i2 = i3 = 0;
//	while (!i3) {
//		usleep(NORMAL_CMD_WAIT);
//		n_read = read(fd, buf2, 16);
//		if (n_read > 0) {
//			for (i1 = 0; i1 < n_read; i1++) {
//				if (i2 < sizeof(buf3)) {
//					buf3[i2++] = buf2[i1];
//				}
//			}
//
//			//printf("buf3 %d: ", i2);
//			//for (i1 = 0; i1 < i2; i1 ++) {
//			//printf("0x%02x ", buf3[i1] & 0xFF);
//			//}
//			//printf("\n");
//			//fflush(stdout);
//
//			for (i1 = 0; i1 < i2 - 17; i1++) {
//				if (buf3[i1] == 0xFE && buf3[i1 + 2] == 0x67 && buf3[i1 + 3] == 0 && buf3[i1 + 4] == status && buf3[i1 + 15] == device_type && buf3[i1 + 16] == device_state) {
//					printf(" got DEVICE INFO UP OK message\n");
//					i3 = 1;
//					break;
//				}
//			}
//		}
//	}
//	printf("exit wait4_commissioning_notification()\n");
//}

void print_receiving_bytes(int iNum, unsigned char *buf2) {
	int i1;
	printf(" %d - ", iNum);

	for (i1 = 0; i1 < iNum; i1++) {
		printf("0x%02x ", buf2[i1] & 0xFF);
	}
	printf("\n");
}

void zb_write_config(int fd, unsigned char config_id, unsigned char config_len, unsigned char *config_data) {
	unsigned char buf1[32], buf2[32];
	int n_read, i1;

	buf1[0] = 0xFE;
	buf1[1] = 2 + config_len;
	buf1[2] = 0x26;
	buf1[3] = 0x05;
	buf1[4] = config_id;
	buf1[5] = config_len;
	for (i1 = 0; i1 < config_len; i1++) {
		buf1[6 + i1] = *config_data++;
	}
	buf1[6 + i1] = calcFCS((unsigned char *) &buf1[1], 5 + config_len);
	if (write(fd, buf1, 7 + config_len) < 0) {
		printf("ERROR: write");
	}

	usleep(NORMAL_CMD_WAIT);

	printf("zb_write_config() return ...");
	n_read = read(fd, buf2, 24);
	print_receiving_bytes(n_read, buf2);
}
#define RX_BUFFER_SIZE		256
int8_t zbReadConfiguration(int fd, uint8_t config_id, uint8_t* rx_buffer, int32_t* len) {
	;
	int8_t i = 0;
	znpBuf[i] = ZNP_SOF;
	i++;
	znpBuf[i] = 0;
	i++;
	znpBuf[i] = MSB(ZB_READ_CONFIGURATION);
	i++;
	znpBuf[i] = LSB(ZB_READ_CONFIGURATION);
	i++;
	znpBuf[i] = config_id;
	i++;
	znpBuf[1] = i - 4;
	printf("I = %d\n",i);
	for (int f = 0; f < 5; f++) {
		printf(" %02X ", znpBuf[f]);
	}
	printf(" NINH");
	printf("\n");
	return 0;
	znpBuf[i] = calcFCS((uint8_t *) &znpBuf[1], (i - 1));
	i++;


	if (write(fd, znpBuf, i) < 0) {
		printf("ERROR: write");
		return ZNP_NOT_SUCCESS;
	}
	usleep(NORMAL_CMD_WAIT);

//	uint8_t rx_buffer[RX_BUFFER_SIZE];
//	int32_t rx_read_len;
	*len = read(fd, rx_buffer, RX_BUFFER_SIZE);
	for (i = 0; i < *len; i++) {
		printf(" %02X ", rx_buffer[i]);   //xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
	}
	printf(" NINH");
	printf("\n");
	return rx_buffer[4]; //return bit status of ZB_READ_CONFIGURATION respone !
}

uint8_t waitingForMessage(int fd, uint16_t cmd) {
	uint8_t rx_buffer[RX_BUFFER_SIZE];
	int32_t rx_read_len, i;
//	printf("waiting message SPSP\n");
	usleep(NORMAL_CMD_WAIT);
	i = 400;
	while (i > 0) {
		rx_read_len = read(fd, rx_buffer, RX_BUFFER_SIZE);
		for (int j = 0; j < rx_read_len; j++) {
			printf(" %02X", rx_buffer[j]);
		}
		printf("\n");
		if (rx_read_len > 0) {
			int j = 0;
			while (j < rx_read_len) {
				if (rx_buffer[j] == ZNP_SOF) {
					if ( CONVERT_TO_INT(rx_buffer[j + 3], rx_buffer[j + 2]) == cmd) {
						return rx_buffer[j + 4];
					}
				}
				j++;
			}

		} else {
			i--;
			usleep(NORMAL_CMD_WAIT);
			printf("i = %d \n", i);
		}
	}

	printf("ZNP_NOT_SUCCESS\n");
	return ZNP_NOT_SUCCESS;
}

uint8_t waitingForStatus(int fd, uint16_t cmd, uint8_t ustatus) {
	uint8_t rx_buffer[RX_BUFFER_SIZE];
	int32_t rx_read_len, i;
//	printf("waiting message SPSP\n");
	usleep(NORMAL_CMD_WAIT);
	i = 400;
	while (i > 0) {
		rx_read_len = read(fd, rx_buffer, RX_BUFFER_SIZE);
		for (int j = 0; j < rx_read_len; j++) {
			printf(" %02X", rx_buffer[j]);
		}
		printf("\n");
		if (rx_read_len > 0) {
			int j = 0;
			while (j < rx_read_len) {
				if (rx_buffer[j] == ZNP_SOF) {
					if ( CONVERT_TO_INT(rx_buffer[j + 3], rx_buffer[j + 2]) == cmd && rx_buffer[j + 4] == ustatus) {
						return rx_buffer[j + 4];
					}
				}
				j++;
			}

		}
		i--;
		usleep(NORMAL_CMD_WAIT);
		printf("i = %d \n", i);
	}

	printf("ZNP_NOT_SUCCESS\n");
	return ZNP_NOT_SUCCESS;
}

/*
 * znpSoftReset*/
#define SYS_RESET_PAYLOAD_LEN	0x01
#define HARD_RESET				0x00
#define SOFT_RESET				0x01
#define WAIT_ONE_SECOND			usleep(1000000)
uint8_t znpSoftReset(int fd) {
	int8_t i = 0;
	znpBuf[i] = ZNP_SOF;
	i++;
	znpBuf[i] = SYS_RESET_PAYLOAD_LEN;
	i++;
	znpBuf[i] = MSB(SYS_RESET_REQ);
	i++;
	znpBuf[i] = LSB(SYS_RESET_REQ);
	i++;
	znpBuf[i] = SOFT_RESET;
	i++;
	znpBuf[i] = calcFCS((uint8_t *) &znpBuf[1], (i - 1));
	i++;

	if (write(fd, znpBuf, i) < 0) {
		printf("ERROR: write");
		return ZNP_NOT_SUCCESS;
	}
	return waitingForMessage(fd, SYS_RESET_IND);
}

/** Configures startup options on the ZNP. These will reset various parameters back to their factory defaults.
 The radio supports two types of clearing state, and both are supported:
 - STARTOPT_CLEAR_CONFIG restores all settings to factory defaults. Must restart the ZNP after using this option.
 - STARTOPT_CLEAR_STATE only clears network settings (PAN ID, channel, etc.)
 @note all ZB_WRITE_CONFIGURATION commands take approx. 3.5mSec between SREQ & SRSP; presumably to write to flash inside the CC2530ZNP.
 @param option which options to set. Must be zero, STARTOPT_CLEAR_CONFIG, or STARTOPT_CLEAR_STATE.
 @post znpResult contains the error code, or ZNP_SUCCESS if success.
*/

#define ZB_WRITE_CONFIGURATION_LEN 		2  //excluding payload length

int8_t setStartupOptions(int fd, uint8_t option) {
	int8_t i = 0;
	if (option > (STARTOPT_CLEAR_CONFIG + STARTOPT_CLEAR_STATE)) {
		printf("ERROR: set option value! \n");
		return ZNP_NOT_SUCCESS;
	}

	znpBuf[i] = ZNP_SOF;
	i++;
	znpBuf[i] = ZB_WRITE_CONFIGURATION_LEN + ZCD_NV_STARTUP_OPTION_LEN;
	i++;
	znpBuf[i] = MSB(ZB_WRITE_CONFIGURATION);
	i++;
	znpBuf[i] = LSB(ZB_WRITE_CONFIGURATION);
	i++;

	znpBuf[i] = ZCD_NV_STARTUP_OPTION;
	i++;
	znpBuf[i] = ZCD_NV_STARTUP_OPTION_LEN;
	i++;
	znpBuf[i] = option;
	i++;
	znpBuf[i] = calcFCS((uint8_t *) &znpBuf[1], (i - 1));
	i++;

	if (write(fd, znpBuf, i) < 0) {
		printf("ERROR: write");
		return ZNP_NOT_SUCCESS;
	}

	return waitingForMessage(fd, (ZB_WRITE_CONFIGURATION | 0x6000));
}

/** Configures the ZNP to only join a network with the given panId.
 If panId = ANY_PAN then the radio will join any network.
 @param panId the PANID to join, or ANY_PAN to join any PAN
 @post znpResult contains the error code, or ZNP_SUCCESS if success.
 */
uint8_t setPanId(int fd, uint16_t panId) {

	int8_t i = 0;
	znpBuf[i] = ZNP_SOF;
	i++;
	znpBuf[i] = ZB_WRITE_CONFIGURATION_LEN + ZCD_NV_PANID_LEN;
	i++;
	znpBuf[i] = MSB(ZB_WRITE_CONFIGURATION);
	i++;
	znpBuf[i] = LSB(ZB_WRITE_CONFIGURATION);
	i++;
	znpBuf[i] = ZCD_NV_PANID;
	i++;
	znpBuf[i] = ZCD_NV_PANID_LEN;
	i++;
	znpBuf[i] = LSB(panId);
	i++;
	znpBuf[i] = MSB(panId);
	i++;
	znpBuf[i] = calcFCS((uint8_t *) &znpBuf[1], (i - 1));
	i++;
	if (write(fd, znpBuf, i) < 0) {
		printf("ERROR: write");
		return ZNP_NOT_SUCCESS;
	}
	return waitingForMessage(fd, (ZB_WRITE_CONFIGURATION | 0x6000));
}

/** Sets the Zigbee Device Type for the ZNP.
 @param deviceType the type of Zigbee device. Must be COORDINATOR, ROUTER, or END_DEVICE
 @post znpResult contains the error code, or ZNP_SUCCESS if success.
 */
uint8_t setZigbeeDeviceType(int fd, uint8_t deviceType) {
	int8_t i = 0;
	if (deviceType > END_DEVICE) {
		return ZNP_NOT_SUCCESS;
	}
	znpBuf[i] = ZNP_SOF;
	i++;
	znpBuf[i] = ZB_WRITE_CONFIGURATION_LEN + ZCD_NV_LOGICAL_TYPE_LEN;
	i++;
	znpBuf[i] = MSB(ZB_WRITE_CONFIGURATION);
	i++;
	znpBuf[i] = LSB(ZB_WRITE_CONFIGURATION);
	i++;

	znpBuf[i] = ZCD_NV_LOGICAL_TYPE;
	i++;
	znpBuf[i] = ZCD_NV_LOGICAL_TYPE_LEN;
	i++;
	znpBuf[i] = deviceType;
	i++;

	znpBuf[i] = calcFCS((uint8_t *) &znpBuf[1], (i - 1));
	i++;
	if (write(fd, znpBuf, i) < 0) {
		printf("ERROR: write");
		return ZNP_NOT_SUCCESS;
	}

	return waitingForMessage(fd, (ZB_WRITE_CONFIGURATION | 0x6000));
}

/** Sets the RF Output power.
 @param Tx Output Power, in dB. e.g. -10 = -10dB, 3 = +3dB etc.
 */
#define SYS_TX_PAYLOAD_LEN 1
uint8_t setTransmitPower(int fd, int8_t txPowerDb) {

	int8_t i = 0;
	znpBuf[i] = ZNP_SOF;
	i++;
	znpBuf[i] = SYS_TX_PAYLOAD_LEN;
	i++;
	znpBuf[i] = MSB(SYS_SET_TX_POWER);
	i++;
	znpBuf[i] = LSB(SYS_SET_TX_POWER);
	i++;
	znpBuf[i] = txPowerDb;
	i++;

	znpBuf[i] = calcFCS((uint8_t *) &znpBuf[1], (i - 1));
	i++;
	if (write(fd, znpBuf, i) < 0) {
		printf("ERROR: write");
		return ZNP_NOT_SUCCESS;
	}

	int8_t znpResult = waitingForMessage(fd, (SYS_SET_TX_POWER | 0x6000));
	if (znpResult == txPowerDb) {
		return ZNP_SUCCESS;
	}
	return ZNP_NOT_SUCCESS;
}

//void set_TX_Power(int fd, unsigned char power) {
//	unsigned char buf1[8], buf2[16];
//	int n_read;
//
//	buf1[1] = 0x02;
//	buf1[2] = 0x21;
//	buf1[3] = 0x14;
//	buf1[4] = 0x00;
//	buf1[5] = power;
//	buf1[6] = calcFCS((unsigned char *) &buf1[1], 5);
//
//	if (write(fd, buf1, 7) < 0) {
//		printf("ERROR: write");
//	}
//	usleep(NORMAL_CMD_WAIT);
//
//	printf("set_TX_Power() return ...");
//	n_read = read(fd, buf2, 16);
//	print_receiving_bytes(n_read, buf2);
//}

#define APP_CNF_BDB_SET_CHANNEL_LEN_PAYLOAD 5
/*setChannelMask*/
uint8_t setChannelMask(int fd, uint8_t primary, uint32_t channelMask) {

	int8_t i = 0;
	znpBuf[i] = ZNP_SOF;
	i++;
	znpBuf[i] = APP_CNF_BDB_SET_CHANNEL_LEN_PAYLOAD;
	i++;
	znpBuf[i] = MSB(APP_CNF_BDB_SET_CHANNEL);
	i++;
	znpBuf[i] = LSB(APP_CNF_BDB_SET_CHANNEL);
	i++;

	znpBuf[i] = primary;
	i++;
	znpBuf[i] = LSB(channelMask);
	i++;
	znpBuf[i] = (channelMask & 0xFF00) >> 8;
	i++;
	znpBuf[i] = (channelMask & 0xFF0000) >> 16;
	i++;
	znpBuf[i] = channelMask >> 24;
	i++;
	znpBuf[i] = calcFCS((uint8_t *) &znpBuf[1], (i - 1));
	i++;

	if (write(fd, znpBuf, i) < 0) {
		printf("ERROR: write");
		return ZNP_NOT_SUCCESS;
	}

	return waitingForMessage(fd, (APP_CNF_BDB_SET_CHANNEL | 0x6000));
}

#define AF_REGISTER_PAYLOAD_LEN         9
uint8_t afRegisterGenericApplication(int fd) {
	int8_t i = 0;
	znpBuf[i] = ZNP_SOF;
	i++;
	znpBuf[i] = AF_REGISTER_PAYLOAD_LEN;
	i++;
	znpBuf[i] = MSB(AF_REGISTER);
	i++;
	znpBuf[i] = LSB(AF_REGISTER);
	i++;
	znpBuf[i] = DEFAULT_ENDPOINT;				//0x1
	i++;
	znpBuf[i] = LSB(DEFAULT_PROFILE_ID);		//0x0104
	i++;
	znpBuf[i] = MSB(DEFAULT_PROFILE_ID);		//0x0104
	i++;
	znpBuf[i] = LSB(DEVICE_ID);					//0x0123
	i++;
	znpBuf[i] = MSB(DEVICE_ID);					//0x0123
	i++;
	znpBuf[i] = DEVICE_VERSION;
	i++;
	znpBuf[i] = LATENCY_NORMAL;
	i++;
	znpBuf[i] = 0;
	i++;                // number of binding input clusters
	znpBuf[i] = 0;
	i++;            // number of binding output clusters
	znpBuf[i] = calcFCS((uint8_t *) &znpBuf[1], (i - 1));
	i++;

	if (write(fd, znpBuf, i) < 0) {
		printf("ERROR: write");
		return ZNP_NOT_SUCCESS;
	}

	return waitingForMessage(fd, (AF_REGISTER | 0x6000));
//	unsigned char buf1[16], buf2[16];
//	int n_read;
//
//	buf1[0] = 0xFE;
//	buf1[1] = AF_REGISTER_PAYLOAD_LEN;
//	buf1[2] = 0x24;
//	buf1[3] = 0;
//	buf1[4] = DEFAULT_ENDPOINT;
//	buf1[5] = LSB(DEFAULT_PROFILE_ID);
//	buf1[6] = MSB(DEFAULT_PROFILE_ID);
//	buf1[7] = LSB(DEVICE_ID);
//	buf1[8] = MSB(DEVICE_ID);
//	buf1[9] = DEVICE_VERSION;
//	buf1[10] = LATENCY_NORMAL;
//	buf1[11] = 0;                 // number of binding input clusters
//	buf1[12] = 0;                 // number of binding output clusters
//	buf1[13] = calcFCS((unsigned char *) &buf1[1], 12);
//
//	if (write(fd, buf1, 14) < 0) {
//		printf("ERROR: write");
//	}
//
//	printf("afRegisterGenericApplication ...");
//
//	usleep(NORMAL_CMD_WAIT);
//
//	printf("afRegisterGenericApplication() return ...");
//	n_read = read(fd, buf2, 16);
//	print_receiving_bytes(n_read, buf2);
//	return ZNP_SUCCESS;
}
#define ZDO_STARTUP_FROM_APP_LEN_PAYLOAD 0x01
uint8_t zdoStartApplication(int fd) {

	int8_t i = 0;
	znpBuf[i] = ZNP_SOF;
	i++;
	znpBuf[i] = ZDO_STARTUP_FROM_APP_LEN_PAYLOAD;
	i++;
	znpBuf[i] = MSB(ZDO_STARTUP_FROM_APP);
	i++;
	znpBuf[i] = LSB(ZDO_STARTUP_FROM_APP);
	i++;
	znpBuf[i] = 0;
	i++;
	znpBuf[i] = calcFCS((uint8_t *) &znpBuf[1], (i - 1));
	i++;

	if (write(fd, znpBuf, i) < 0) {
		printf("ERROR: write");
		return ZNP_NOT_SUCCESS;
	}

	return waitingForMessage(fd, ( ZDO_STARTUP_FROM_APP | 0x6000));
//	return waitingForMessage(fd, ( ZB_START_CONFIRM));
}

void zb_write_config(int fd, unsigned char config_id1, unsigned char config_id2, unsigned char offset, unsigned char config_len, unsigned char *config_data) {
	unsigned char buf1[32], buf2[32];
	int n_written, n_read, i1;

	buf1[0] = 0xFE;
	buf1[1] = 4 + config_len;
	buf1[2] = 0x21;
	buf1[3] = 0x09;
	buf1[4] = config_id2;
	buf1[5] = config_id1;
	buf1[6] = offset;
	buf1[7] = config_len;
	for (i1 = 0; i1 < config_len; i1++) {
		buf1[8 + i1] = *config_data++;
	}
	buf1[8 + i1] = calcFCS((unsigned char *) &buf1[1], 7 + config_len);
	n_written = write(fd, buf1, 9 + config_len);

	usleep(NORMAL_CMD_WAIT);

	printf("zb_write_config() return ...");
	n_read = read(fd, buf2, 24);
	printf(" %d - ", n_read);

	for (i1 = 0; i1 < n_read; i1++) {
		printf("0x%02x ", buf2[i1] & 0xFF);
	}
	printf(" done !\n");
}

void util_get_device_info(int fd) {

	int8_t i = 0;
	znpBuf[i] = ZNP_SOF;
	i++;
	znpBuf[i] = 0;
	i++;
	znpBuf[i] = MSB(UTIL_GET_DEVICE_INFO);
	i++;
	znpBuf[i] = LSB(UTIL_GET_DEVICE_INFO);
	i++;
	znpBuf[i] = calcFCS((uint8_t *) &znpBuf[1], (i - 1));
	i++;

	if (write(fd, znpBuf, i) < 0) {
		printf("ERROR: write");
	}

	printf("util_get_device_info\n");
}

/** Enable/Disabless callbacks on the ZNP.
 @param cb must be either CALLBACKS_ENABLED or CALLBACKS_DISABLED
 @see section ZCD_NV_ZDO_DIRECT_CB in ZNP Interface Specification
 @post znpResult contains the error code, or ZNP_SUCCESS if success.
 */
uint8_t setCallbacks(int fd, uint8_t cb) {
	if ((cb != CALLBACKS_ENABLED) && (cb != CALLBACKS_DISABLED)) {
		return ZNP_NOT_SUCCESS;
	}
	int8_t i = 0;
	znpBuf[i] = ZNP_SOF;
	i++;
	znpBuf[i] = 3;
	i++;
	znpBuf[i] = MSB(ZB_WRITE_CONFIGURATION);
	i++;
	znpBuf[i] = LSB(ZB_WRITE_CONFIGURATION);
	i++;
	znpBuf[i] = ZCD_NV_ZDO_DIRECT_CB;
	i++;
	znpBuf[i] = ZCD_NV_ZDO_DIRECT_CB_LEN;
	i++;
	znpBuf[i] = cb;
	i++;
	znpBuf[i] = calcFCS((uint8_t *) &znpBuf[1], (i - 1));
	i++;

	if (write(fd, znpBuf, i) < 0) {
		printf("ERROR: write");
		return ZNP_NOT_SUCCESS;
	}
	return waitingForMessage(fd, (ZB_WRITE_CONFIGURATION | 0x6000));

}

uint8_t startZigbeeCoordinator_test(int serial_fd) {

    uint8_t startup_option = 0;
    uint8_t k1;
    uint8_t znpResult;
    uint8_t rx_buffer[RX_BUFFER_SIZE];
    int32_t rx_read_len = 0;

//    printf("reset ZNP\n");
//    znpResult = znpSoftReset(serial_fd);
//    if (znpResult != ZNP_SUCCESS) {
//        printf("ERROR: reset ZNP \n");
//        return znpResult;
//    }

    //check panid
    znpResult = zbReadConfiguration(serial_fd, ZCD_NV_PANID, rx_buffer, (int32_t*) &rx_read_len);

    if (znpResult != ZNP_SUCCESS) {
        return znpResult;
    }

	if (rx_buffer[7] == 0xFF && rx_buffer[8] == 0xFF) {
		startup_option = 1;
	}
    if (startup_option == 0) {
        printf("Skipping startup option !\n");
    } else {
        znpResult = setStartupOptions(serial_fd, DEFAULT_STARTUP_OPTIONS);


        if (znpResult != ZNP_SUCCESS) {
            printf("ERROR: startup option. \n");
            return znpResult;
        }

        znpResult = znpSoftReset(serial_fd);
        if (znpResult != ZNP_SUCCESS) {
            printf("ERROR: reset ZNP \n");
            return znpResult;
        }

//	// Set security ON
//		printf("Set security ON ...");
//		k1 = 1;
//		zb_write_config(serial_fd, 0x64, 1, &k1);
//		printf("\n");

//	//	// Set ZCD_NV_PRECFGKEYS_ENABLE
//	//	printf("Set ZCD_NV_PRECFGKEYS_ENABLE ...");
//	//	k1 = 0;
//	//	zb_write_config(serial_fd, 0x63, 1, &k1);
//	//	printf("\n");

//	// Set ZCD_NV_PRECFGKEY
//	printf("Set ZCD_NV_PRECFGKEY ...");
//	zb_write_config(serial_fd, 0x62, 16, &pre_zigbee_key[0]);
//	printf("\n");

        // Set security ON
        printf("Set security ON ...");
        k1 = 1;
        zb_write_config(serial_fd, 0x64, 1, &k1);
        printf("\n");

        znpResult = setPanId(serial_fd, (uint16_t) PAN_ID);

        if (znpResult != ZNP_SUCCESS) {
            printf("ERROR: PAN ID \n");
            return znpResult;
        }

        znpResult = setZigbeeDeviceType(serial_fd, COORDINATOR);
        if (znpResult != ZNP_SUCCESS) {
            printf("ERROR: Device type \n");
            return znpResult;
        }

        //Set primary channel mask & disable secondary channel mask
        znpResult = setChannelMask(serial_fd, CHANNEL_TRUE, (uint32_t) DEFAULT_CHANNEL_MASK);
        if (znpResult != ZNP_SUCCESS) {
            printf("ERROR: set channel mask \n");
            return znpResult;
        }
        znpResult = setChannelMask(serial_fd, CHANNEL_FALSE, (uint32_t) 0);
        if (znpResult != ZNP_SUCCESS) {
            printf("ERROR: set channel mask \n");
            return znpResult;
        }
        // Start commissioning using network formation as parameter to start coordinator

        app_cnf_bdb_start_commissioning(serial_fd, COMMISSIONING_MODE_INFORMATION, 2);		// 0x04 is Network Formation
//		wait4_commissioning_notification(serial_fd, 0, 2);

//	if (startup_option > 0) {
//		util_get_device_info(serial_fd);
//		wait4_coordinator_up_ok(serial_fd, 0, 7, 9);
//	}

        znpResult = app_cnf_set_allowrejoin_tc_policy(serial_fd, 1);
        if (znpResult != ZNP_SUCCESS) {
            printf("ERROR: set allow join \n");
            return znpResult;
        }
//	set_TX_Power(serial_fd, DEFAULT_TX_POWER);
        znpResult = setTransmitPower(serial_fd, DEFAULT_TX_POWER);

//		if (znpResult != ZNP_SUCCESS) {
//			printf("ERROR: set transmit power \n");
//			return znpResult;
//		}
//		printf("STATUS: %d\n",znpResult);



        // Set ZCD_NV_ZDO_DIRECT_CB
        znpResult = setCallbacks(serial_fd, CALLBACKS_ENABLED);
        if (znpResult != ZNP_SUCCESS) {
            printf("ERROR: set callback \n");
            return znpResult;
        }

        znpResult = app_cnf_bdb_start_commissioning(serial_fd, COMMISSIONING_MODE_STEERING, 1);		// 0x02 is Network Steering
        if (znpResult != ZNP_SUCCESS) {
            printf("ERROR: Network Steering \n");
            return znpResult;
        }
    }



    //-------------------------------

    znpResult = znpSoftReset(serial_fd);
    if (znpResult != ZNP_SUCCESS) {
        printf("ERROR: reset ZNP \n");
        return znpResult;
    }

    znpResult = app_cnf_bdb_start_commissioning(serial_fd, COMMISSIONING_MODE_STEERING, 1);		// 0x02 is Network Steering
    if (znpResult != ZNP_SUCCESS) {
        printf("ERROR: Network Steering \n");
        return znpResult;
    }

    znpResult = setTCRequireKeyExchange(serial_fd, 0);
    if (znpResult != ZNP_SUCCESS) {
        printf("ERROR: set TC key exchange \n");
        return znpResult;
    }

    znpResult = afRegisterGenericApplication(serial_fd);
    if (znpResult != ZNP_SUCCESS) {
        printf("ERROR: af register \n");
        return znpResult;
    }
    znpResult = zdoStartApplication(serial_fd);
    if (znpResult != ZNP_SUCCESS) {
        printf("ERROR: ZDO start application \n");
        return znpResult;
    }

    znpResult = setPermitJoiningReq(serial_fd, SHORT_ADDRESS_COORDINATOR, DISABLE_PERMIT_JOIN, 1);



    if (znpResult != ZNP_SUCCESS) {
        printf("ERROR: disabled permit join \n");
        return znpResult;
    }

    /*get MAC address of coordinator*/
    getMACAddressReq(serial_fd, SHORT_ADDRESS_COORDINATOR, 0x00, 0x00);





    return ZNP_SUCCESS;
}

uint8_t startZigbeeCoordinator(int serial_fd) {

    uint8_t startup_option = 0;
	uint8_t k1;
	uint8_t znpResult;
	uint8_t rx_buffer[RX_BUFFER_SIZE];
	int32_t rx_read_len = 0;

//	printf("reset ZNP\n");
//	znpResult = znpSoftReset(serial_fd);
//	if (znpResult != ZNP_SUCCESS) {
//		printf("ERROR: reset ZNP \n");
//		return znpResult;
//	}

	//check panid
	znpResult = zbReadConfiguration(serial_fd, ZCD_NV_PANID, rx_buffer, (int32_t*) &rx_read_len);

	if (znpResult != ZNP_SUCCESS) {
		return znpResult;
	}

	if (rx_buffer[7] == 0xFF && rx_buffer[8] == 0xFF) {
		startup_option = 1;
	}
	if (startup_option == 0) {
		printf("Skipping startup option !\n");
	} else {
		znpResult = setStartupOptions(serial_fd, DEFAULT_STARTUP_OPTIONS);


		if (znpResult != ZNP_SUCCESS) {
			printf("ERROR: startup option. \n");
			return znpResult;
		}

		znpResult = znpSoftReset(serial_fd);
		if (znpResult != ZNP_SUCCESS) {
			printf("ERROR: reset ZNP \n");
			return znpResult;
		}

//	// Set security ON
//		printf("Set security ON ...");
//		k1 = 1;
//		zb_write_config(serial_fd, 0x64, 1, &k1);
//		printf("\n");

//	//	// Set ZCD_NV_PRECFGKEYS_ENABLE
//	//	printf("Set ZCD_NV_PRECFGKEYS_ENABLE ...");
//	//	k1 = 0;
//	//	zb_write_config(serial_fd, 0x63, 1, &k1);
//	//	printf("\n");

//	// Set ZCD_NV_PRECFGKEY
//	printf("Set ZCD_NV_PRECFGKEY ...");
//	zb_write_config(serial_fd, 0x62, 16, &pre_zigbee_key[0]);
//	printf("\n");

		// Set security ON
		printf("Set security ON ...");
		k1 = 1;
		zb_write_config(serial_fd, 0x64, 1, &k1);
		printf("\n");

		znpResult = setPanId(serial_fd, (uint16_t) PAN_ID);

		if (znpResult != ZNP_SUCCESS) {
			printf("ERROR: PAN ID \n");
			return znpResult;
		}


		znpResult = setZigbeeDeviceType(serial_fd, COORDINATOR);
		if (znpResult != ZNP_SUCCESS) {
			printf("ERROR: Device type \n");
			return znpResult;
		}

		//Set primary channel mask & disable secondary channel mask
		znpResult = setChannelMask(serial_fd, CHANNEL_TRUE, (uint32_t) DEFAULT_CHANNEL_MASK);
		if (znpResult != ZNP_SUCCESS) {
			printf("ERROR: set channel mask \n");
			return znpResult;
		}
		znpResult = setChannelMask(serial_fd, CHANNEL_FALSE, (uint32_t) 0);
		if (znpResult != ZNP_SUCCESS) {
			printf("ERROR: set channel mask \n");
			return znpResult;
		}
		// Start commissioning using network formation as parameter to start coordinator

		app_cnf_bdb_start_commissioning(serial_fd, COMMISSIONING_MODE_INFORMATION, 2);		// 0x04 is Network Formation
//		wait4_commissioning_notification(serial_fd, 0, 2);

//	if (startup_option > 0) {
//		util_get_device_info(serial_fd);
//		wait4_coordinator_up_ok(serial_fd, 0, 7, 9);
//	}

		znpResult = app_cnf_set_allowrejoin_tc_policy(serial_fd, 1);
		if (znpResult != ZNP_SUCCESS) {
			printf("ERROR: set allow join \n");
			return znpResult;
		}
//	set_TX_Power(serial_fd, DEFAULT_TX_POWER);
		znpResult = setTransmitPower(serial_fd, DEFAULT_TX_POWER);
//		if (znpResult != ZNP_SUCCESS) {
//			printf("ERROR: set transmit power \n");
//			return znpResult;
//		}
		// Set ZCD_NV_ZDO_DIRECT_CB
		znpResult = setCallbacks(serial_fd, CALLBACKS_ENABLED);
		if (znpResult != ZNP_SUCCESS) {
			printf("ERROR: set callback \n");
			return znpResult;
		}

		znpResult = app_cnf_bdb_start_commissioning(serial_fd, COMMISSIONING_MODE_STEERING, 1);		// 0x02 is Network Steering
		if (znpResult != ZNP_SUCCESS) {
			printf("ERROR: Network Steering \n");
			return znpResult;
		}
	}
	znpResult = znpSoftReset(serial_fd);
	if (znpResult != ZNP_SUCCESS) {
		printf("ERROR: reset ZNP \n");
		return znpResult;
	}

	znpResult = app_cnf_bdb_start_commissioning(serial_fd, COMMISSIONING_MODE_STEERING, 1);		// 0x02 is Network Steering
	if (znpResult != ZNP_SUCCESS) {
		printf("ERROR: Network Steering \n");
		return znpResult;
	}

	znpResult = setTCRequireKeyExchange(serial_fd, 0);
	if (znpResult != ZNP_SUCCESS) {
		printf("ERROR: set TC key exchange \n");
		return znpResult;
	}

	znpResult = afRegisterGenericApplication(serial_fd);
	if (znpResult != ZNP_SUCCESS) {
		printf("ERROR: af register \n");
		return znpResult;
	}
	znpResult = zdoStartApplication(serial_fd);
	if (znpResult != ZNP_SUCCESS) {
		printf("ERROR: ZDO start application \n");
		return znpResult;
	}

	znpResult = setPermitJoiningReq(serial_fd, SHORT_ADDRESS_COORDINATOR, DISABLE_PERMIT_JOIN, 1);



	if (znpResult != ZNP_SUCCESS) {
		printf("ERROR: disabled permit join \n");
		return znpResult;
	}

	/*get MAC address of coordinator*/
	getMACAddressReq(serial_fd, SHORT_ADDRESS_COORDINATOR, 0x00, 0x00);





	return ZNP_SUCCESS;
}

int8_t setTCRequireKeyExchange(int fd, uint8_t bdb_TrustCenterRequireKeyExchange) {

	int8_t i = 0;
	znpBuf[i] = ZNP_SOF;
	i++;
	znpBuf[i] = 0;
	i++;
	znpBuf[i] = MSB(APP_CNF_BDB_SET_TC_REQUIRE_KEY_EXCHANGE);
	i++;
	znpBuf[i] = LSB(APP_CNF_BDB_SET_TC_REQUIRE_KEY_EXCHANGE);
	i++;
	znpBuf[i] = bdb_TrustCenterRequireKeyExchange;
	i++;
	znpBuf[1] = i - 4;
	znpBuf[i] = calcFCS((uint8_t *) &znpBuf[1], (i - 1));
	i++;

	if (write(fd, znpBuf, i) < 0) {
		printf("ERROR: write");
		return ZNP_NOT_SUCCESS;
	}
	return waitingForMessage(fd, APP_CNF_BDB_SET_TC_REQUIRE_KEY_EXCHANGE_SRSP);
}


int8_t zb_Start_Request(int fd, uint8_t flagWaiting) {
    int8_t i = 0;
    znpBuf[i] = ZNP_SOF;
    i++;
    znpBuf[i] = 0;
    i++;
    znpBuf[i] = MSB(ZB_START_REQUEST);
    i++;
    znpBuf[i] = LSB(ZB_START_REQUEST);
    i++;
    znpBuf[i] = calcFCS((uint8_t *) &znpBuf[1], (i - 1));
    i++;

    if (write(fd, znpBuf, i) < 0) {
        printf("ERROR: write");
        return ZNP_NOT_SUCCESS;
    }

    if (flagWaiting == 0) {
        return waitingForMessage(fd, ZDO_MGMT_PERMIT_JOIN_RSP);
    } else {
        return ZNP_SUCCESS;
    }
}


int8_t setPermitJoiningReq(int fd, uint16_t short_addr, uint8_t Timeout, uint8_t flagWaiting) {
	int8_t i = 0;
	znpBuf[i] = ZNP_SOF;
	i++;
	znpBuf[i] = 0;
	i++;
	znpBuf[i] = MSB(ZB_PERMIT_JOINING_REQUEST);
	i++;
	znpBuf[i] = LSB(ZB_PERMIT_JOINING_REQUEST);
	i++;
	znpBuf[i] = LSB(short_addr);
	i++;
	znpBuf[i] = MSB(short_addr);
	i++;
	znpBuf[i] = Timeout;
	i++;
    znpBuf[1] = i - 4;      //? znpBuf[1] != znpBuf[i]
	znpBuf[i] = calcFCS((uint8_t *) &znpBuf[1], (i - 1));
	i++;

	if (write(fd, znpBuf, i) < 0) {
		printf("ERROR: write");
		return ZNP_NOT_SUCCESS;
	}

	if (flagWaiting == 0) {
		return waitingForMessage(fd, ZDO_MGMT_PERMIT_JOIN_RSP);
	} else {
		return ZNP_SUCCESS;
	}
}

/**For example, Device A sends a ZDO_MGMT_LEAVE_REQ to device B.
 *Next, Device A's module will send a ZDO_MGMT_LEAVE_RSP (0x45B4) to host Then Device A will send a ZDO_LEAVE_IND with source address = Device B's short address
 *Device B's module will leave the network and send a ZDO_LEAVE_IND (0x45C9) to host, with source address = Device A's short address
 *Device B's module will then reset and send a reset indication to host
 *Device B is now off the network
 */
int8_t zdo_mgmt_leave_req(int fd, uint16_t short_add, uint8_t ieee_addr[8], uint8_t flags) {

//	uint8_t data[] = { 0xF4, 0xC2, 0xF6, 0x00, 0x00, 0x8D, 0x15, 0x00 }; /*sensor_motion*/
	//uint8_t data[] = { 0x5D, 0x35, 0x14, 0x01, 0x00, 0x8D, 0x15, 0x00 }; /*sensor_temperature*/
//						5D 35 14 01 00 8D 15 00
	//memcpy(ieee_addr, data, 8);
	//short_add = 0xAA5D;
	//flags = 0x01;
	int8_t i = 0;
	znpBuf[i] = ZNP_SOF;
	i++;
	znpBuf[i] = 0;
	i++;
	znpBuf[i] = MSB(ZDO_MGMT_LEAVE_REQ);
	i++;
	znpBuf[i] = LSB(ZDO_MGMT_LEAVE_REQ);
	i++;
	znpBuf[i] = LSB(short_add);
	i++;
	znpBuf[i] = MSB(short_add);
	i++;
	memcpy((uint8_t*) &znpBuf[i], ieee_addr, 8);
	i += 8;
	znpBuf[i] = flags;
	i++;

	znpBuf[1] = i - 4;
	znpBuf[i] = calcFCS((uint8_t *) &znpBuf[1], (i - 1));
	i++;

	for (int j = 0; j < i; j++) {
		printf("%02X ", znpBuf[j]);
	}
	printf("\n");

	if (write(fd, znpBuf, i) < 0) {
		printf("ERROR: write");
		return ZNP_NOT_SUCCESS;
	}

	return ZNP_SUCCESS;
//	return waitingForMessage(fd, (ZDO_MGMT_LEAVE_REQ | 0x6000));
}

/*Get MAC Address*/
int8_t getMACAddressReq(int fd, uint16_t short_addr, uint8_t req_type, uint8_t start_index) {
	int8_t i = 0;
	znpBuf[i] = ZNP_SOF;
	i++;
	znpBuf[i] = 0;
	i++;
	znpBuf[i] = MSB(ZDO_IEEE_ADDR_REQ);
	i++;
	znpBuf[i] = LSB(ZDO_IEEE_ADDR_REQ);
	i++;
	znpBuf[i] = LSB(short_addr);
	i++;
	znpBuf[i] = MSB(short_addr);
	i++;
	znpBuf[i] = req_type;
	i++;
	znpBuf[i] = start_index;
	i++;
	znpBuf[1] = i - 4;
	znpBuf[i] = calcFCS((uint8_t *) &znpBuf[1], (i - 1));
	i++;

	if (write(fd, znpBuf, i) < 0) {
		printf("ERROR: write");
		return ZNP_NOT_SUCCESS;
	}
	return ZNP_SUCCESS;
}

/*send af_data_request*/
int8_t send_af_data_request(int fd, af_data_request_t afDataRequest) {

	int32_t i = 0;
	znpBuf[i] = ZNP_SOF;
	i++;
	znpBuf[i] = 0;
	i++;
	znpBuf[i] = MSB(AF_DATA_REQUEST);
	i++;
	znpBuf[i] = LSB(AF_DATA_REQUEST);
	i++;
	znpBuf[i] = LSB(afDataRequest.dst_address);
	printf("dst_address:%02X\n",afDataRequest.dst_address);
	i++;
	znpBuf[i] = MSB(afDataRequest.dst_address);
	i++;
	znpBuf[i] = afDataRequest.dst_endpoint;
	printf("dst_endpoint:%02X\n",afDataRequest.dst_endpoint);
	i++;
	znpBuf[i] = afDataRequest.src_endpoint;
	printf("src_endpoint:%02X\n",afDataRequest.dst_endpoint);
	i++;
	znpBuf[i] = MSB(afDataRequest.cluster_id);
	printf("cluster_id:%02X",znpBuf[i]);
	i++;
	znpBuf[i] = LSB(afDataRequest.cluster_id);
	printf("%02X\n",znpBuf[i]);
	i++;
	znpBuf[i] = afDataRequest.trans_id;
	printf("trans_id:%02X\n",afDataRequest.trans_id);
	i++;
	znpBuf[i] = afDataRequest.options;
	printf("options:%02X\n",afDataRequest.options);
	i++;
	znpBuf[i] = afDataRequest.radius;
	printf("radius:%02X\n",afDataRequest.radius);
	i++;
	znpBuf[i] = afDataRequest.len;
	printf("len:%02X\n",afDataRequest.len);
	i++;
	printf("Show DATA1 : %02X\n",afDataRequest.data[0]);
	printf("Show DATA1 : %02X\n",afDataRequest.data[1]);
	printf("Show DATA1 : %02X\n",afDataRequest.data[2]);
	printf("Show DATA1 : %02X\n",afDataRequest.data[3]);
	printf("Show DATA1 : %02X\n",afDataRequest.data[4]);
	printf("Show DATA1 : %02X\n",afDataRequest.data[5]);
	printf("Show DATA1 : %02X\n",afDataRequest.data[6]);

	memcpy((uint8_t*) &znpBuf[i], afDataRequest.data, afDataRequest.len);
	for(uint8_t x = i;x<(i+7);x++) {
		printf("DATA3:%02X\n",znpBuf[x]);
	}
	i += afDataRequest.len;
	znpBuf[1] = i - 4;
	znpBuf[i] = calcFCS((uint8_t *) &znpBuf[1], (i - 1));
	i++;
	free(afDataRequest.data);
	if (write(fd, znpBuf, i) < 0) {
		printf("ERROR: write");
		return ZNP_NOT_SUCCESS;
	}

	return ZNP_SUCCESS;
}
