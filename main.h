#ifndef __main_h
#define __main_h


#include "um34c.h"
#include "fileHandler.h"

typedef struct {
    int nSocketHandle;
    int nStatus;
    uint32_t dwTimerInterval;
    um34c_data_S SCurrentData;
    uint8_t *pbRequestData;
    char szDestDevAddr[UM34C_ADDR_LEN];
    bdaddr_t abyDestDevAddr;
    char szCSVfileName[UM34C_DATA_FILE_NAME_LEN];
    uint8_t bSaveToCSVfile;

} um34c_config_S;

#endif  // __main_h