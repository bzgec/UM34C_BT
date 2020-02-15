#ifndef __umc34c_h
#define __umc34c_h

#define _GNU_SOURCE
#include "dataTypes.h"
#include <bluetooth/bluetooth.h>


#define UM34C_ADDR "00:15:A3:00:2D:BF"
#define UM34C_ADDR_LEN 18
#define UM34C_MSG_SIZE 130
#define UM34C_INTERVAL_DEFAULT ( 500*1000)
#define UM34C_INTERVAL_MIN     ( 100*1000)
#define UM34C_INTERVAL_MAX     (2000*1000)

#define DATE_TIME_STRING_FORMAT "%H:%M:%S %d/%m/%Y"
#define DATE_TIME_STRING_SIZE   20





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
    char szTimeDate[DATE_TIME_STRING_SIZE];  // "20:11:18 14/02/2020"
} um34c_data_S;









uint8_t bConnectToBtAddapter(int *pnSocketHandle, bdaddr_t *pabyDestDevAddr, int *pnStatus);
uint8_t bGetDestDevAddr(char *szUM34CAddress);
void decodeData(uint8_t *buf, um34c_data_S *data);
uint8_t * createTimer(uint32_t dwInterval);
void timer_handler (int signum);








#endif  // __umc34c_h