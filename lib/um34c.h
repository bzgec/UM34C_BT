#ifndef __umc34c_h
#define __umc34c_h

// #define _GNU_SOURCE
#include "dataTypes.h"
#include <bluetooth/bluetooth.h>


#define UM34C_ADDR "00:15:A3:00:2D:BF"
#define UM34C_ADDR_LEN 18
#define UM34C_MSG_SIZE 130
#define UM34C_INTERVAL_DEFAULT ( 500*1000)  // [us]
#define UM34C_INTERVAL_MIN     ( 400*1000)  // [us]
#define UM34C_INTERVAL_MAX    (10000*1000)  // [us]

#define DATE_TIME_STRING_FORMAT "%H:%M:%S %d/%m/%Y"
#define DATE_TIME_STRING_SIZE   20
#define DATE_TIME_STRING_INIT   "no data yet"

#define UM34C_SEND_CMD_SIZE 1

// if x times in a row there is error on sending data exit application
// there is probably error on connection with UM34C
#define UM34C_MAX_SEND_ERR_NUMB  10




/* Found commands:
	* 		a0-9: Select group 0-9
	* 		b0-ce: Set record current
	* 		d0-5: Set brightness to 0-5
	* 		e0-9: Set timeout 0-9 minutes
	* 		f0: Get Data
	* 		f1: Next
	* 		f2: Rotate Clockwise
	* 		f3: Prev
	* 		f4: Reset Group
	*/

typedef enum {
    um34c_cmd_groupSel0 =        0xA0,
    um34c_cmd_groupSel1 =        0xA1,
    um34c_cmd_groupSel2 =        0xA2,
    um34c_cmd_groupSel3 =        0xA3,
    um34c_cmd_groupSel4 =        0xA4,
    um34c_cmd_groupSel5 =        0xA5,
    um34c_cmd_groupSel6 =        0xA6,
    um34c_cmd_groupSel7 =        0xA7,
    um34c_cmd_groupSel8 =        0xA8,
    um34c_cmd_groupSel9 =        0xA9,
    um34c_cmd_setRecordCurrent = 0xB0,
    um34c_cmd_setBrightness0 =   0xD0,
    um34c_cmd_setBrightness1 =   0xD1,
    um34c_cmd_setBrightness2 =   0xD2,
    um34c_cmd_setBrightness3 =   0xD3,
    um34c_cmd_setBrightness4 =   0xD4,
    um34c_cmd_setBrightness5 =   0xD5,
    um34c_cmd_setTimeout0 =      0xE0,  // 0 min
    um34c_cmd_setTimeout1 =      0xE1,  // 1 min
    um34c_cmd_setTimeout2 =      0xE2,
    um34c_cmd_setTimeout3 =      0xE3,
    um34c_cmd_setTimeout4 =      0xE4,
    um34c_cmd_setTimeout5 =      0xE5,
    um34c_cmd_setTimeout6 =      0xE6,
    um34c_cmd_setTimeout7 =      0xE7,
    um34c_cmd_setTimeout8 =      0xE8,
    um34c_cmd_setTimeout9 =      0xE9,
    um34c_cmd_getData =          0xF0,
    um34c_cmd_next =             0xF1,
    um34c_cmd_rotateClockwise =  0xF2,
    um34c_cmd_prev =             0xF3,
    um34c_cmd_resetGroup =       0xF4,
} um34c_cmd_E;

typedef struct {
    float fVoltage;
    float fCurrent;
    float fPower;
    uint8_t byTemperatureC;
    uint8_t bySelectedGroup;
    uint16_t wCapacity_mAh[10];
    uint16_t wCapacity_mWh[10];
    float fResistance;
    uint8_t byCurrentScreen;
    uint8_t byBrightness;
    uint8_t byScreenTimeout;
    float fUsbDataPlus;
    float fUsbDataMinus;
    uint8_t byChargingMode;
    char szTimeDate[DATE_TIME_STRING_SIZE];  // "20:11:18 14/02/2020"
} um34c_data_S;

typedef struct {
    uint8_t bInitialized;
    int nSocketHandle;
    int nStatus;
    uint32_t dwTimerInterval;
    um34c_data_S SCurrentData;
    uint8_t *pbRequestData;  // Needed only if timer is used
    char szDestDevAddr[UM34C_ADDR_LEN];
    bdaddr_t abyDestDevAddr;
    uint8_t byErrSend;
} um34c_config_S;






void UM34C_init(um34c_config_S *pSUM34C_config);
void UM34C_deinit(um34c_config_S *pSUM34C_config);
uint8_t UM34C_bGetDestDevAddr(char *szUM34CAddress);
uint8_t UM34C_bConnectToBtAddapter(um34c_config_S *pSUM34C_config);
void createTimer(um34c_config_S *pSUM34C_config);
void timer_handler (int signum);
uint8_t UM34C_bGetData(um34c_config_S *pSConfig);
void UM34C_sendCmd(int nSocketHandle, uint8_t byBuffSend, int *pnStatus);
void UM34C_readCmd(int nSocketHandle, uint8_t *pabyBuff, size_t size, int *pnStatus);
void UM34C_prettyPrintData(um34c_data_S *pSData, uint8_t bUseNcurses);
void UM34C_decodeData(uint8_t *buf, um34c_data_S *pSData);





#endif  // __umc34c_h