#include "logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdarg.h>

#define LOG_DATE_TIME_STRING_SIZE 40

typedef struct {
    const char *pszFileName;
    const char *pszDateTimeFormat;
    char szDateTimeStr[LOG_DATE_TIME_STRING_SIZE];
    log_lvl_E ELogLvl;
} loggerConfig_S;


static loggerConfig_S fs_loggerConfig;

// Check https://www.tutorialspoint.com/c_standard_library/c_function_strftime.htm for formatting dateTime
void init_logger(const char *pszFileName, const char *pszDateTime, log_lvl_E ELogLvl) {
    fs_loggerConfig.pszFileName = pszFileName;
    fs_loggerConfig.pszDateTimeFormat = pszDateTime;
    fs_loggerConfig.ELogLvl = ELogLvl;
}

void logger(log_lvl_E ELogLvl, const char* tag, const char *format, ...) {
    FILE * fp;
    time_t currentTime;
    struct tm *tm;
    va_list args;  
    char szBuff[LOG_STR_ARG_MAX_SIZE];
    char *pszLoggerMode[] = {
        "DEBUG",
        "INFO",
        "WARNING",
        "ERROR",
        "CRITICAL"
    };

    currentTime = time(NULL);
    tm = localtime(&currentTime);
    strftime(fs_loggerConfig.szDateTimeStr, LOG_DATE_TIME_STRING_SIZE, fs_loggerConfig.pszDateTimeFormat, tm);

    // Convert passed arguments to specified string (with format)
    va_start(args, format);
    vsnprintf(szBuff, LOG_STR_ARG_MAX_SIZE, format, args);

    if(ELogLvl > log_lvl_critical || ELogLvl < log_lvl_debug) {
        fp = fopen (fs_loggerConfig.pszFileName, "a");
        if(fp != NULL) {
            fprintf(fp, "%s\t%s\t%s\t%s\n", fs_loggerConfig.szDateTimeStr, tag, "ERROR\tNO SUCH LOGGER LEVEL\t", szBuff);
            fclose(fp);
        }
    } else if(ELogLvl >= fs_loggerConfig.ELogLvl) {
        fp = fopen (fs_loggerConfig.pszFileName, "a");
        if(fp != NULL) {
            fprintf(fp, "%s\t%s\t%s\t%s\n", fs_loggerConfig.szDateTimeStr, tag, pszLoggerMode[ELogLvl-1], szBuff);
            fclose(fp);
        }
    }

    va_end(args);
}