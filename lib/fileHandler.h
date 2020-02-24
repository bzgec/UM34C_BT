#ifndef __fileHandler_h
#define __fileHandler_h

#include "um34c.h"

#define UM34C_ADDR_FILE_NAME "um34c_addr.txt"

#define UM34C_DATA_FILE_NAME        "um34c_data_%Y%m%d_%H%M%S.csv"  // "um34c_data_20200214_204518.csv"
#define UM34C_DATA_FILE_NAME_LEN     31
// #define UM34C_DATA_CSV_HEADERS "Time;Voltage;Current;\n"
// #define UM34C_DATA_CSV_HEADERS "Time;Voltage;Current;Power;Resistance;Temperature;mAh;mWh;\n"
#define UM34C_DATA_CSV_HEADERS "Time;Voltage;Current;Temperature;mAh;mWh;\n"  // Power and resistance can be calculated from Voltage and current

#define BYTES_TO_kB(bytes)  ((bytes)/1000.0)
#define BYTES_TO_MB(bytes)  ((bytes)/(1000*1000.0))
#define BYTES_TO_GB(bytes)  ((bytes)/(1000*1000*1000.0))


typedef struct  {
    char szCSVfileName[UM34C_DATA_FILE_NAME_LEN];
    uint32_t dwNumbOfAppends;
    uint8_t bFileCreated_CSV;
} fileHandler_config_S;

typedef enum {
    fileHandler_info_OK = 0,
    fileHandler_info_NULL = 1,
    fileHandler_info_FAILED_TO_OPEN_FILE = 2,
    fileHandler_info_FAILED_TO_GET_FILE_SIZE = 3,
    fileHandler_info_FILE_TOO_BIG = 4,
    fileHandler_info_FILE_CREATED = 5,

} fileHandler_info_E;

uint8_t bReadDevAddrFromFile(char *pszDevAddr);
void storeDevAddrToFile(char *pszDevAddr);
uint8_t bMakeNewCSVfile(fileHandler_config_S *pSConfig);
fileHandler_info_E byAppendToCSVfile(fileHandler_config_S *pSConfig, um34c_data_S *pSData);
long int getFileSize(char *pszFileName);
fileHandler_info_E fileHandler_checkIfTooBig(char *pszFileName, uint32_t dwFileSizeB);


#endif  // __fileHandler_h