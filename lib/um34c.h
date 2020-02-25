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

// Record current threshold values in centi Ampers (like cm but actully cA)
#define UM34C_SRCT_MIN       0
#define UM34C_SRCT_MAX      30  // 0.3 A
#define UM34C_SRCT_DEFAULT  10  // 0.1 A



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
// https://sigrok.org/wiki/RDTech_UM_series
typedef enum {
    um34c_cmd_SDG_0 =        0xA0,  // Set Data Group - 0
    um34c_cmd_SDG_1 =        0xA1,  // Set Data Group - 1
    um34c_cmd_SDG_2 =        0xA2,  // Set Data Group - 2
    um34c_cmd_SDG_3 =        0xA3,  // Set Data Group - 3
    um34c_cmd_SDG_4 =        0xA4,  // Set Data Group - 4
    um34c_cmd_SDG_5 =        0xA5,  // Set Data Group - 5
    um34c_cmd_SDG_6 =        0xA6,  // Set Data Group - 6
    um34c_cmd_SDG_7 =        0xA7,  // Set Data Group - 7
    um34c_cmd_SDG_8 =        0xA8,  // Set Data Group - 8
    um34c_cmd_SDG_9 =        0xA9,  // Set Data Group - 9
    um34c_cmd_SRCT_Min = 0xB0,  // Set Record Current Threshold - 0.00 A
    um34c_cmd_SRCT_Max = 0xCE,  // Set Record Current Threshold - 0.30 A
    um34c_cmd_SB_Min =   0xD0,  // Set Brightness - Min
    um34c_cmd_SB_Max =   0xD5,  // Set Brightness - Max
    um34c_cmd_ST_OFF =      0xE0,  // Set Timeout - 0 min == OFF
    um34c_cmd_ST_Min =      0xE1,  // Set Timeout - 1 min
    um34c_cmd_ST_Max =      0xE9,  // Set Timeout - 9 min
    um34c_cmd_getData =          0xF0,
    um34c_cmd_nextScreen =             0xF1,
    um34c_cmd_rotateClockwise =  0xF2,
    um34c_cmd_prevScreen =             0xF3,
    um34c_cmd_resetDataGroup =       0xF4,
} um34c_cmd_E;

typedef enum {
    UM24C = 0x0963,
    UM25C = 0x09c9,
    UM34C = 0x0d4c
} UM_C_devices_E;

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
    const char *pszChargingMode;
    const char *pszModelID;
    uint16_t wThreshold_mAh;  // mAh from threshold-based recording 
    uint16_t wThreshold_mWh;  // mWh from threshold-based recording
    uint8_t byThreshold_cA;  // Currently configured threshold for recording (in centiamps, divide by 100 to get A) 
    uint8_t bThreshold_active;  // Threshold recording active (1 if recording, 0 if not) 
    uint16_t wThreshold_duration;  //  Duration of threshold recording, in cumulative seconds 

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
uint8_t UM34C_bMatchID(uint16_t wModeID);
uint8_t UM34C_bGetData(um34c_config_S *pSConfig);
void UM34C_sendCmd(int nSocketHandle, uint8_t byBuffSend, int *pnStatus);
void UM34C_readCmd(int nSocketHandle, uint8_t *pabyBuff, size_t size, int *pnStatus);
const char *UM34C_getModelName(UM_C_devices_E EDevice);
const char *UM34C_getChargingMode(uint8_t byChargingModeNumb);
void UM34C_prettyPrintData(uint16_t *wY, uint16_t *wX, um34c_data_S *pSData, uint8_t bUseNcurses);
void UM34C_prettyPrintSettings(uint16_t *wY, uint16_t *wX, um34c_data_S *pSData, uint8_t bUseNcurses);
void UM34C_decodeData(uint8_t *buf, um34c_data_S *pSData);





#endif  // __umc34c_h