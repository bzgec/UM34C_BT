#ifndef __logger_h
#define __logger_h


#define LOG_DFLT_DATE_TIME_STRING_FORMAT "%H:%M:%S %d/%m/%Y"
#define LOG_STR_ARG_MAX_SIZE  300  // size of the string which can be made in 'logger()' function 

typedef enum {
    log_lvl_debug    = 1,
    log_lvl_info     = 2,
    log_lvl_warning  = 3,
    log_lvl_error    = 4,
    log_lvl_critical = 5,
} log_lvl_E;


void init_logger(const char *pszFileName, const char *pszDateTime, log_lvl_E ELogLvl);
void logger(log_lvl_E ELogLvl, const char* tag, const char *format, ...);


#endif  // __logger_h