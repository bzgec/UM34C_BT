#ifndef __fileHandler_h
#define __fileHandler_h

#include "um34c.h"

#define UM34C_ADDR_FILE_NAME "um34c_addr.txt"

#define UM34C_DATA_FILE_NAME        "um34c_data_%Y%m%d_%H%M%S.csv"  // "um34c_data_20200214_204518.csv"
#define UM34C_DATA_FILE_NAME_LEN     31
// #define UM34C_DATA_CSV_HEADERS "Time;Voltage;Current;\n"
#define UM34C_DATA_CSV_HEADERS "Time;Voltage;Current;Power;Resistance;Temperature;mAh;mWh;\n"

uint8_t bReadDevAddrFromFile(char *pszDevAddr);
void storeDevAddrToFile(char *pszDevAddr);
void makeNewCSVfile(char *pszCSVfileName);
void appendToCSVfile(char *pszCSVfileName, um34c_data_S *pSData);


#endif  // __fileHandler_h