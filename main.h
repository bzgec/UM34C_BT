#ifndef __main_h
#define __main_h


#include "um34c.h"
#include "fileHandler.h"

#define RUNNING_TIME_STRING "%H:%M:%S"
#define RUNNING_TIME_STRING_SIZE 10

#define INTREVAL_CHECK_FOR_CHAR (20*1000)  // [us]
#define INTREVAL_UPDATE_DISPLAY (50*1000)  // [us]

typedef struct {
    um34c_config_S *pSUM34C_config;
    fileHandler_config_S *pSFileHandler_config;
    uint8_t bSaveToCSVfile;
    uint8_t bReadData;
    time_t startTime;
    int nCmdChar;
    int nCmdChar_prev;
    uint8_t byDeviceBrightness;
    uint8_t byCurrentScreenRotation;
} mainConfig_S;

#endif  // __main_h