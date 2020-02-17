#ifndef __fileHandler_h
#define __fileHandler_h

#include "um34c.h"

#define UM34C_ADDR_FILE_NAME "um34c_addr.txt"

#define UM34C_DATA_FILE_NAME        "um34c_data_%Y%m%d_%H%M%S.csv"  // "um34c_data_20200214_204518.csv"
#define UM34C_DATA_FILE_NAME_LEN     31
// #define UM34C_DATA_CSV_HEADERS "Time;Voltage;Current;\n"
#define UM34C_DATA_CSV_HEADERS "Time;Voltage;Current;Power;Resistance;Temperature;mAh;mWh;\n"

typedef struct  {
    char szCSVfileName[UM34C_DATA_FILE_NAME_LEN];
    uint32_t dwNumbOfAppends;
    uint8_t bFileCreated_CSV;
} fileHandler_config_S;


uint8_t bReadDevAddrFromFile(char *pszDevAddr);
void storeDevAddrToFile(char *pszDevAddr);
uint8_t bMakeNewCSVfile(fileHandler_config_S *pSConfig);
uint8_t byAppendToCSVfile(fileHandler_config_S *pSConfig, um34c_data_S *pSData);
long int getFileSize(char *pszFileName);


#endif  // __fileHandler_h