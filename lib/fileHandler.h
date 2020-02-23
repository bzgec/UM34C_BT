#ifndef __fileHandler_h
#define __fileHandler_h

#include "um34c.h"

#define UM34C_ADDR_FILE_NAME "um34c_addr.txt"

#define UM34C_DATA_FILE_NAME        "um34c_data_%Y%m%d_%H%M%S.csv"  // "um34c_data_20200214_204518.csv"
#define UM34C_DATA_FILE_NAME_LEN     31
// #define UM34C_DATA_CSV_HEADERS "Time;Voltage;Current;\n"
// #define UM34C_DATA_CSV_HEADERS "Time;Voltage;Current;Power;Resistance;Temperature;mAh;mWh;\n"
#define UM34C_DATA_CSV_HEADERS "Time;Voltage;Current;Temperature;mAh;mWh;\n"  // Power and resistance can be calculated from Voltage and current

#define BYTES_TO_kB  (1000.0)
#define BYTES_TO_MB  (1000.0*1000.0)
#define BYTES_TO_GB  (1000.0*1000.0*1000.0)

typedef struct  {
    char szCSVfileName[UM34C_DATA_FILE_NAME_LEN];
    uint32_t dwNumbOfAppends;
    uint8_t bFileCreated_CSV;
} fileHandler_config_S;

typedef enum {
    fileHandler_size_B,
    fileHandler_size_kB,
    fileHandler_size_MB,
    fileHandler_size_GB,

    fileHandler_size_count
} fileHandler_size_E;


uint8_t bReadDevAddrFromFile(char *pszDevAddr);
void storeDevAddrToFile(char *pszDevAddr);
uint8_t bMakeNewCSVfile(fileHandler_config_S *pSConfig);
uint8_t byAppendToCSVfile(fileHandler_config_S *pSConfig, um34c_data_S *pSData);
float getFileSize(char *pszFileName, fileHandler_size_E EReturnIn);


#endif  // __fileHandler_h