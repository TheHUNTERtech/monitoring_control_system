#ifndef __ZNP_SERIAL_H
#define __ZNP_SERIAL_H

/** Convert two unsigned chars to an unsigned int, LSB first*/
#define CONVERT_TO_INT(lsb,msb) ((lsb) + 0x0100*(msb))   // ((lsb) + (((unsigned int) (msb)) << 8))

// Get the Least Significant Byte (LSB) of an uint16_t
#define LSB(num) ((num) & 0xFF)

// Get the Most Significant Byte (MSB) of an uint16_t
#define MSB(num) ((num) >> 8)

// moduleResult_t setCallbacks(uint8_t cb);
#define CALLBACKS_DISABLED              0
#define CALLBACKS_ENABLED               1

//channel
#define CHANNEL_TRUE              		1
#define CHANNEL_FALSE               	0

#define CHANNEL_MASK_11                     0x800
#define CHANNEL_MASK_12                    0x1000
#define CHANNEL_MASK_13                    0x2000
#define CHANNEL_MASK_14                    0x4000
#define CHANNEL_MASK_15                    0x8000
#define CHANNEL_MASK_16                   0x10000
#define CHANNEL_MASK_17                   0x20000
#define CHANNEL_MASK_18                   0x40000
#define CHANNEL_MASK_19                   0x80000
#define CHANNEL_MASK_20                  0x100000
#define CHANNEL_MASK_21                  0x200000
#define CHANNEL_MASK_22                  0x400000
#define CHANNEL_MASK_23                  0x800000
#define CHANNEL_MASK_24                 0x1000000
#define CHANNEL_MASK_25                 0x2000000
#define CHANNEL_MASK_26                 0x4000000

#define STARTOPT_CLEAR_CONFIG           0x01
#define STARTOPT_CLEAR_STATE            0x02
#define STARTOPT_AUTO                   0x04

#define DEFAULT_STARTUP_OPTIONS         (STARTOPT_CLEAR_CONFIG + STARTOPT_CLEAR_STATE)
#define DEFAULT_CHANNEL_MASK            CHANNEL_MASK_11 //(CHANNEL_MASK_11 | CHANNEL_MASK_14 | CHANNEL_MASK_17 | CHANNEL_MASK_20 | CHANNEL_MASK_23)
#define ANY_PAN                         0xFFFF

//default values used when creating applicationConfigurations in Simple API or AFZDO

#define DEFAULT_ENDPOINT        	0x1

#define DEFAULT_PROFILE_ID      	0x0104		// HA Profile ID

#define DEVICE_ID               	0x0123		// HA Profile ID

#define DEVICE_VERSION          	0x89

//Values for latencyRequested field of struct applicationConfiguration. Not used in Simple API.
#define LATENCY_NORMAL          	0
#define LATENCY_FAST_BEACONS    	1
#define LATENCY_SLOW_BEACONS   		2

#define DEFAULT_TX_POWER		0xE5
#define ZNP_SOF				0xFE

#define NORMAL_CMD_WAIT			150000
#define START_APP_CMD_WAIT		3500000

#define PAN_ID					0x00F1

//LOGICAL TYPES
#define COORDINATOR                     0x00
#define ROUTER                          0x01
#define END_DEVICE                      0x02

//WRITE CONFIGURATION OPTIONS
#define ZCD_NV_STARTUP_OPTION           0x03
#define ZCD_NV_STARTUP_OPTION_LEN       1
#define ZCD_NV_LOGICAL_TYPE             0x87
#define ZCD_NV_LOGICAL_TYPE_LEN         1
#define ZCD_NV_ZDO_DIRECT_CB            0x8F
#define ZCD_NV_ZDO_DIRECT_CB_LEN        1

//NETWORK SPECIFIC CONFIGURATION PARAMETERS
#define ZCD_NV_PANID                    0x83
#define ZCD_NV_PANID_LEN                2

//STARTUP OPTIONS
#define STARTOPT_CLEAR_CONFIG           0x01
#define STARTOPT_CLEAR_STATE            0x02
#define STARTOPT_AUTO                   0x04

//SYS Interface
#define SYS_SET_TX_POWER                0x2114
#define SYS_RESET_IND                   0x4180
#define SYS_RESET_REQ                   0x4100

//AF
#define AF_REGISTER 					0x2400
#define AF_DATA_REQUEST 				0x2401
#define AF_INCOMING_MSG					0x4481
//ZDO
#define ZDO_STARTUP_FROM_APP 			0x2540
#define ZDO_MGMT_LEAVE_REQ				0x2534
#define ZDO_MGMT_PERMIT_JOIN_REQ		0x2536
#define ZDO_IEEE_ADDR_REQ 				0x2501
//Util
#define  UTIL_GET_DEVICE_INFO 			0x2700

/*SRSP*/
#define  UTIL_GET_DEVICE_INFO_RESPONSE 	0x6700

//ZDO RSP
#define ZDO_IEEE_ADDR_RSP 				0x4581
#define ZDO_MGMT_LEAVE_RSP 				0x45B4
#define ZDO_MGMT_PERMIT_JOIN_RSP 		0x45B6
#define ZDO_LEAVE_IND 					0x45C9
#define ZDO_END_DEVICE_ANNCE_IND		0x45C1
#define ZDO_TC_DEV_IND					0x45CA

//Simple API REQ
#define ZB_START_REQUEST 				0x2600
#define ZB_READ_CONFIGURATION 			0x2604
#define ZB_WRITE_CONFIGURATION          0x2605
#define ZB_PERMIT_JOINING_REQUEST 		0x2608

//Simple API SPSP
#define ZB_READ_CONFIGURATION_RSP 		0x6604

//Simple API RESPONSE
#define ZB_START_CONFIRM 				0x4680
#define ZDO_MGMT_PERMIT_JOIN_RSP 		0x45B6

//App configuration
#define APP_CNF_SET_ALLOWREJOIN_TC_POLICY		0x2F03
#define	APP_CNF_BDB_START_COMMISSIONING 		0x2F05
#define APP_CNF_BDB_SET_CHANNEL 				0x2F08
#define APP_CNF_BDB_SET_TC_REQUIRE_KEY_EXCHANGE 0x2F09

//Application configuration SPSP
#define APP_CNF_BDB_SET_TC_REQUIRE_KEY_EXCHANGE_SRSP	0x6F09

//App configuration SPSP message
#define APP_CNF_BDB_COMMISSIONING_NOTIFICATION	0x4F80

#define	COMMISSIONING_MODE_STEERING			0x02
#define	COMMISSIONING_MODE_INFORMATION		0x04

//Received in SRSP message
#define ZNP_SUCCESS                     0x00
#define ZNP_NOT_SUCCESS					0x01

//permit join req
/*
address to broadcast address to all devices                         0xFFFF
address to broadcast address to all RxOnWhenIdle devices            0xFFFD
address to Broadcast to all routers and coordinator                 0xFFFC
*/
#define	DISABLE_PERMIT_JOIN                         0x00
#define	TIME_ENABLE_PERMIT_JOIN                     60 //..<=> 0x3C is 60 seconds

#define SHORT_ADDRESS_COORDINATOR                   0x0000
#define SHORT_ADDRESS_COORDINATOR_ADD_ROUTER		0xFFFC

//uint8_t pre_zigbee_key[16] = {'1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1'};

uint8_t startZigbeeCoordinator(int serial_fd);
uint8_t startZigbeeCoordinator_test(int serial_fd);
int8_t app_cnf_bdb_start_commissioning(int fd, uint8_t mode_config, uint8_t mode_receiving, uint8_t flagWaiting = 0);
void util_get_device_info(int fd);

/// frame format for a ZDO Management Leave Request
typedef struct __attribute__((__packed__)) zdo_mgmt_leave_req_t {
	uint16_t short_address;
	uint8_t device_address[8];
	uint8_t flags;
#define ZDO_MGMT_LEAVE_REQ_FLAG_NONE					0x00
#define ZDO_MGMT_LEAVE_REQ_FLAG_REJOIN					0x01
#define ZDO_MGMT_LEAVE_REQ_FLAG_REMOVE_CHILDREN		0x02
} zdo_mgmt_leave_req_t;

// struct af_data_request

//Byte: 1 1 1 2 1
//Length = 0x0A-0x8A Cmd0 = 0x24 Cmd1 = 0x01 DstAddr DstEndpoint
//Byte: 1 2 1 1 1 1 0-128
//SrcEndpoint ClusterId TransId Options Radius Len Data

typedef struct __attribute__((__packed__)) af_data_request_t {
	uint16_t dst_address;
	uint8_t dst_endpoint;
	uint8_t src_endpoint;
	uint16_t cluster_id;
	uint8_t trans_id;
	uint8_t options;
	uint8_t radius;
	uint8_t len;
	uint8_t *data;
} af_data_request_t;

int8_t zdo_mgmt_leave_req(int fd, uint16_t short_add, uint8_t ieee_addr[8], uint8_t flags);
int8_t setPermitJoiningReq(int fd, uint16_t short_addr, uint8_t Timeout, uint8_t flagWaiting = 0);
int8_t getMACAddressReq(int fd, uint16_t short_addr, uint8_t req_type, uint8_t start_index);
int8_t send_af_data_request(int fd, af_data_request_t afDataRequest);
int8_t zb_Start_Request(int fd, uint8_t flagWaiting);
#endif
