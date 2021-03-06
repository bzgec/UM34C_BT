/*
 * https://people.csail.mit.edu/albert/bluez-intro/c404.html
 * https://github.com/sebastianha/um34c
 * https://linux.die.net/man/2/setitimer
 * http://www.cs.ukzn.ac.za/~hughm/os/notes/ncurses.html
 * 
 * 00:15:A3:00:2D:BF  UM34C
 */


#include "main.h"
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>
#include <bluetooth/rfcomm.h>
#include <signal.h>
#include <sys/socket.h>
#include <time.h>
#include <pthread.h>
#include <ncurses.h>
#include "customPrint.h"
#include "config.h"
#include "logger.h"
#include "display.h"


void fs_sigintHandler(int nSigNum);
void *threadReadData_UM34C(void *arg);
void checkForCommand(void);
void tryToSetRCT();


mainConfig_S g_SConfig;
extern um34c_config_S g_SUM34C_config;
extern fileHandler_config_S g_SFileHandler_config;

void exitProgram(exitProgram_param_E EParam) {
    time_t currentTime;
    uint64_t second;
    uint8_t minute;
    uint8_t hour;

    currentTime = time(NULL);

    // Compute running time and store it in this variable
    currentTime -= g_SConfig.startTime;

    hour = currentTime/3600;
    second = currentTime % 3600;
    minute = second/60;
    second %= 60;

    endwin();  // dellocates memory and ends ncurses

    UM34C_deinit(g_SConfig.pSUM34C_config);

    if(EParam == exitProgram_param_SendingUM34C) {
        printf("Connection to UM34C is probably lost\n\r");
    }
    
    // print last data to terminal (if there was any data...)
    if(strcmp(g_SConfig.pSUM34C_config->SCurrentData.szTimeDate, DATE_TIME_STRING_INIT) != 0) {
        UM34C_prettyPrintSettings(0, 0, &g_SConfig.pSUM34C_config->SCurrentData, FALSE);
        UM34C_prettyPrintData(0, 0, &g_SConfig.pSUM34C_config->SCurrentData, FALSE);

        // If there were no readings than no data could be saved to file...
        if(g_SConfig.pSFileHandler_config->bFileCreated_CSV == TRUE) {
            printf("Number of appends to CSV file: %d\n\r", g_SConfig.pSFileHandler_config->dwNumbOfAppends);
            logger(log_lvl_info, "main", "Number of appends to CSV file: %d", g_SConfig.pSFileHandler_config->dwNumbOfAppends);
            printf("File size: %0.1f kB\n\r", BYTES_TO_kB(getFileSize(g_SConfig.pSFileHandler_config->szCSVfileName)));
            logger(log_lvl_info, "main", "File size: %0.1f kB", BYTES_TO_kB(getFileSize(g_SConfig.pSFileHandler_config->szCSVfileName)));
        }
    }

    printf("Running time: %02d:%02d:%02ld\n\r", hour, minute, second);
    logger(log_lvl_info, "main", "Running time: %02d:%02d:%02ld", hour, minute, second);

    exit(0);
}

// Catch CTRL+C
void fs_sigintHandler(int nSigNum) {
    exitProgram(exitProgram_param_CTRL_C);
}

void *threadReadData_UM34C(void *arg) {
    time_t currentTime;
    struct tm *tm;
    fileHandler_info_E EAppendRetVal;
    uint8_t bGotData;
    float fVoltage_tmp;
    float fCurrent_tmp;

    g_SConfig.pSFileHandler_config->dwNumbOfAppends = 0;

    while(1) {
        if(g_SConfig.bReadData) {
            g_SConfig.bUpdateDisp = TRUE;

            #ifndef UM34C_NOT_IN_RANGE

            // Try to get data from UM34C
            bGotData = UM34C_bGetData(g_SConfig.pSUM34C_config);

            #else  // #ifndef UM34C_NOT_IN_RANGE

            bGotData = TRUE;
            currentTime = time(NULL);
            tm = localtime(&currentTime);
            strftime(g_SConfig.pSUM34C_config->SCurrentData.szTimeDate, DATE_TIME_STRING_SIZE, DATE_TIME_STRING_FORMAT, tm);

            #endif  // UM34C_NOT_IN_RANGE

            if(bGotData == TRUE) {
                // Data from UM34C received successfully

                // Caluclate moving average (if used)
                if(g_SConfig.wMovAvgStrength != 0) {
                    movingAvg_calc_f(&g_SConfig.SMvgAvg_handle_fVoltage, g_SConfig.pSUM34C_config->SCurrentData.fVoltage);
                    movingAvg_calc_f(&g_SConfig.SMvgAvg_handle_fCurrent, g_SConfig.pSUM34C_config->SCurrentData.fCurrent);
                }

                // Save to CSV file (if used)
                if(g_SConfig.bSaveToCSVfile) {
                    // Check if moving average filter is used
                    if(g_SConfig.wMovAvgStrength == 0) {
                        // Moving average filter is NOT used
                        EAppendRetVal = byAppendToCSVfile(g_SConfig.pSFileHandler_config, &g_SConfig.pSUM34C_config->SCurrentData);
                    } else {
                        // Moving average gilter is used
                        // Writing to file only after 'g_SConfig.wMovAvgStrength' samples were taken, this is
                        // when 'wPos' is 0
                        if(g_SConfig.SMvgAvg_handle_fCurrent.wPos == 0) {
                            // Store current voltage and current (U and I)
                            fVoltage_tmp = g_SConfig.pSUM34C_config->SCurrentData.fVoltage;
                            fCurrent_tmp = g_SConfig.pSUM34C_config->SCurrentData.fCurrent;

                            // Change data of current voltage and current to be average value
                            g_SConfig.pSUM34C_config->SCurrentData.fVoltage = g_SConfig.SMvgAvg_handle_fVoltage.fAverage;
                            g_SConfig.pSUM34C_config->SCurrentData.fCurrent = g_SConfig.SMvgAvg_handle_fCurrent.fAverage;

                            // Append average data of voltage and current to CSV file
                            EAppendRetVal = byAppendToCSVfile(g_SConfig.pSFileHandler_config, &g_SConfig.pSUM34C_config->SCurrentData);

                            // Return voltage and current data back to normal
                            g_SConfig.pSUM34C_config->SCurrentData.fVoltage = fVoltage_tmp;
                            g_SConfig.pSUM34C_config->SCurrentData.fCurrent = fCurrent_tmp;

                        } else {
                            // Did not write to CSV file, but just pretend so there is no logger message
                            EAppendRetVal = fileHandler_info_OK;
                        }
                    }

                    // Check for errors
                    if(EAppendRetVal == fileHandler_info_FILE_CREATED) {
                        logger(log_lvl_debug, "main", "CSV file created %s", g_SConfig.pSFileHandler_config->szCSVfileName);
                    } else if (EAppendRetVal != fileHandler_info_OK) {
                        logger(log_lvl_error, "main", "Error on appending to CSV file");
                    }
                }
            } else {
                // Error while getting data from UM34C
            }
        }
       
        usleep(g_SConfig.pSUM34C_config->dwTimerInterval);  // sleep time should be the as long as timer interval
    }
}

void checkForCommand(void) {

    if(g_SConfig.nCmdChar == ERR) {
        // no input from terminal
    } else {
        switch (g_SConfig.nCmdChar) {
        case KEY_UP:  // Brightness or Record current threshold UP            
            if(g_SConfig.nCmdChar_prev == 'b') {
                g_SConfig.byDeviceBrightness = g_SConfig.pSUM34C_config->SCurrentData.byBrightness;
                if(g_SConfig.byDeviceBrightness < um34c_cmd_SB_Max-um34c_cmd_SB_Min) {
                    g_SConfig.byDeviceBrightness++;
                }
                UM34C_sendCmd(g_SConfig.pSUM34C_config->nSocketHandle, (um34c_cmd_E)(g_SConfig.byDeviceBrightness + um34c_cmd_SB_Min), &g_SConfig.pSUM34C_config->nStatus);
                sprintf(g_SConfig.szLastCmdBuff, "Brightness: %d (UP)", g_SConfig.byDeviceBrightness);
            } else if(g_SConfig.nCmdChar_prev == 'i') {
                g_SConfig.byRecCurrThreshold_cA = g_SConfig.pSUM34C_config->SCurrentData.byThreshold_cA;
                if(g_SConfig.byRecCurrThreshold_cA < um34c_cmd_SRCT_Max-um34c_cmd_SRCT_Min) {
                    g_SConfig.byRecCurrThreshold_cA++;
                }
                UM34C_sendCmd(g_SConfig.pSUM34C_config->nSocketHandle, g_SConfig.byRecCurrThreshold_cA + um34c_cmd_SRCT_Min, &g_SConfig.pSUM34C_config->nStatus);
                sprintf(g_SConfig.szLastCmdBuff, "Record current threshold: %0.2f (UP)", g_SConfig.byRecCurrThreshold_cA/100.0);
            } else {
                sprintf(g_SConfig.szLastCmdBuff, "Frist press `b` or `i` to change brightness or record current threshold.");
            }

            // set current char to previous char so that we can press 0, 1, 2... multiple times, 
            // without pressing 'b' or 'i' again
            g_SConfig.nCmdChar = g_SConfig.nCmdChar_prev;
            break;
        case KEY_DOWN:  // Brightness or Record current threshold DOWN
            if(g_SConfig.nCmdChar_prev == 'b') {
                g_SConfig.byDeviceBrightness = g_SConfig.pSUM34C_config->SCurrentData.byBrightness;
                if(g_SConfig.byDeviceBrightness > 0) {
                    g_SConfig.byDeviceBrightness--;
                }
                UM34C_sendCmd(g_SConfig.pSUM34C_config->nSocketHandle, (um34c_cmd_E)(g_SConfig.byDeviceBrightness + um34c_cmd_SB_Min), &g_SConfig.pSUM34C_config->nStatus);
                sprintf(g_SConfig.szLastCmdBuff, "Brightness: %d (DOWN)", g_SConfig.byDeviceBrightness);
            } else if(g_SConfig.nCmdChar_prev == 'i') {
                g_SConfig.byRecCurrThreshold_cA = g_SConfig.pSUM34C_config->SCurrentData.byThreshold_cA;
                if(g_SConfig.byRecCurrThreshold_cA > 0) {
                    g_SConfig.byRecCurrThreshold_cA--;
                }
                UM34C_sendCmd(g_SConfig.pSUM34C_config->nSocketHandle, g_SConfig.byRecCurrThreshold_cA + um34c_cmd_SRCT_Min, &g_SConfig.pSUM34C_config->nStatus);
                sprintf(g_SConfig.szLastCmdBuff, "Record current threshold: %0.2f (DOWN)", g_SConfig.byRecCurrThreshold_cA/100.0);
            } else {
                sprintf(g_SConfig.szLastCmdBuff, "Frist press `b` or `i` to change brightness or record current threshold.");
            }

            // set current char to previous char so that we can press 0, 1, 2... multiple times, 
            // without pressing 'b' or 'i' again
            g_SConfig.nCmdChar = g_SConfig.nCmdChar_prev;
            break;
        case KEY_LEFT:  // Previous display
            UM34C_sendCmd(g_SConfig.pSUM34C_config->nSocketHandle, um34c_cmd_prevScreen, &g_SConfig.pSUM34C_config->nStatus);
            sprintf(g_SConfig.szLastCmdBuff, "Previous display");
           break;
        case KEY_RIGHT:  // Next display
            UM34C_sendCmd(g_SConfig.pSUM34C_config->nSocketHandle, um34c_cmd_nextScreen, &g_SConfig.pSUM34C_config->nStatus);
            sprintf(g_SConfig.szLastCmdBuff, "Next display");
            break;
        case 'c':  // Exit program
            exitProgram(exitProgram_param_C);
            break;
        case 0x20:  // Toggle data sampling - 'space'
            g_SConfig.bReadData = !g_SConfig.bReadData;
            sprintf(g_SConfig.szLastCmdBuff, "Toggle data sampling.");
            break;
        case 's':  // Toggle saving to CSV file
            g_SConfig.bSaveToCSVfile = !g_SConfig.bSaveToCSVfile;
            sprintf(g_SConfig.szLastCmdBuff, "Toggle saving to CSV file.");
            break;
        case 'r':  // rotate screen
            UM34C_sendCmd(g_SConfig.pSUM34C_config->nSocketHandle, um34c_cmd_rotateClockwise, &g_SConfig.pSUM34C_config->nStatus);
            g_SConfig.byCurrentScreenRotation++;
            if(g_SConfig.byCurrentScreenRotation % 4 == 0) {
                g_SConfig.byCurrentScreenRotation = 0;
            }
            sprintf(g_SConfig.szLastCmdBuff, "Rotating screen: %d", g_SConfig.byCurrentScreenRotation);
            break;
        case 'h':  // Toggle show help
            g_SConfig.bShowHelp = !g_SConfig.bShowHelp;
            break;
        case '0':  // set screen timeout or data group
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            if(g_SConfig.nCmdChar_prev == 'g') {
                UM34C_sendCmd(g_SConfig.pSUM34C_config->nSocketHandle, (g_SConfig.nCmdChar - '0') + um34c_cmd_SDG_0, &g_SConfig.pSUM34C_config->nStatus);
                sprintf(g_SConfig.szLastCmdBuff, "Data group: %d", g_SConfig.nCmdChar - '0');
            } else if(g_SConfig.nCmdChar_prev == 't') {
                UM34C_sendCmd(g_SConfig.pSUM34C_config->nSocketHandle, (g_SConfig.nCmdChar - '0') + um34c_cmd_ST_OFF, &g_SConfig.pSUM34C_config->nStatus);
                sprintf(g_SConfig.szLastCmdBuff, "Screen timeout: %d", g_SConfig.nCmdChar - '0');
            } else {
                sprintf(g_SConfig.szLastCmdBuff, "Frist press `g` or `t` to change data group or screen timeout.");
            }
            // set current char to previous char so that we can press 0, 1, 2... multiple times, 
            // without pressing 'g' or 't' again
            g_SConfig.nCmdChar = g_SConfig.nCmdChar_prev;
            break;
        case 'b':
            sprintf(g_SConfig.szLastCmdBuff, "Changing brightness");
            break;
        case 'i':
            sprintf(g_SConfig.szLastCmdBuff, "Changing record current threshold");
            break;
        case 'g':
            sprintf(g_SConfig.szLastCmdBuff, "Changing data group");
            break;
        case 't':
            sprintf(g_SConfig.szLastCmdBuff, "Changing screen timeout");
            break;
        default:
            break;
        }
    
    }

    g_SConfig.nCmdChar_prev = g_SConfig.nCmdChar;

    logger(log_lvl_debug, "display", "Last command: %s", g_SConfig.szLastCmdBuff);
}

void tryToSetRCT() {
    uint8_t bGotData;
    uint8_t byMaxRetries = 10;  // try to set record current threshold x times, if it fails x times exit program

    printf("Trying to set record current threshold...");
    logger(log_lvl_warning, "main", "Trying to set record current threshold");
    
    do {
        UM34C_sendCmd(g_SConfig.pSUM34C_config->nSocketHandle, g_SConfig.byRecCurrThreshold_cA + um34c_cmd_SRCT_Min, &g_SConfig.pSUM34C_config->nStatus);

        if(g_SConfig.pSUM34C_config->nStatus == UM34C_SEND_CMD_SIZE) {
            usleep(1000*1000);  // sleep between (for some reason program holds if there is no sleep...)
            bGotData = UM34C_bGetData(g_SConfig.pSUM34C_config);

            if(bGotData == TRUE && 
               g_SConfig.pSUM34C_config->SCurrentData.byThreshold_cA  == g_SConfig.byRecCurrThreshold_cA) {
                // Record current threshold was set correctly
                printf(" done\n\r");
                logger(log_lvl_warning, "main", "Record current threshold was set");
                break;
            } else {
                // Error on receiving record current threshold
                // printf("Error on receiving record current threshold\n\r");
                logger(log_lvl_warning, "main", "Error on receiving record current threshold");
                byMaxRetries--;
            }
        } else {
            // Failed to send data
            // printf("Failed to send record current threshold\n\r");
            logger(log_lvl_warning, "main", "Failed to send record current threshold");
            byMaxRetries--;
       }

       if(byMaxRetries == 0) {
           // Failed to set record current threshold, exit program
           printf(" FAILED\n\r");
           logger(log_lvl_warning, "main", "Failed to set record current threshold");
           exitProgram(exitProgram_param_SettingRCT);
       }
    } while(1);
}

int main(int argc, char **argv) {
    pthread_t threadUM34C;
    pthread_t threadDisplay;

    g_SConfig.startTime = time(NULL);

    init_logger(LOGGER_FILE_NAME, LOG_DFLT_DATE_TIME_STRING_FORMAT, log_lvl_debug);

    logger(log_lvl_debug, "main", "Program started");

    signal(SIGINT, fs_sigintHandler);  // Catch CTRL+C

    g_SConfig.pSUM34C_config = &g_SUM34C_config;
    g_SConfig.pSFileHandler_config = &g_SFileHandler_config;

    UM34C_init(g_SConfig.pSUM34C_config);

    g_SConfig.bSaveToCSVfile = FALSE;
    g_SConfig.bReadData =FALSE;
    g_SConfig.byDeviceBrightness = 3;
    g_SConfig.byCurrentScreenRotation = 0;
    g_SConfig.pSFileHandler_config->bFileCreated_CSV = FALSE;
    g_SConfig.bUpdateDisp = TRUE;
    g_SConfig.wMovAvgStrength = 0;
    g_SConfig.byRecCurrThreshold_cA = 0;
    g_SConfig.bShowHelp = FALSE;
    g_SConfig.bUseGUI = TRUE;
    strcpy(g_SConfig.szLastCmdBuff, "No command yet");

    // Check passed arguments
    if(argc > 1) {
        for(uint8_t i=1; i < argc; i++) {
            if(strcmp(argv[i], "-a") == 0) {
                // Device address
                if(++i < argc) {
                    strncpy(g_SConfig.pSUM34C_config->szDestDevAddr, argv[i], UM34C_ADDR_LEN);
                } else {
                    printf("Error on entering address\n\r");
                    logger(log_lvl_error, "main", "Error on entering address");
                    return 1;
                }
            } else if (strcmp(argv[i], "-q") == 0) {
                // Timer interval
                if(++i < argc) {
                    g_SConfig.pSUM34C_config->dwTimerInterval = ms_TO_us(strtoul(argv[i], NULL, 10));
                    if(g_SConfig.pSUM34C_config->dwTimerInterval == 0) { 
                        printf("Error parsing interval value\n\r");
                        logger(log_lvl_error, "main", "Error parsing interval value");
                        return 1;
                    } else if(g_SConfig.pSUM34C_config->dwTimerInterval < UM34C_INTERVAL_MIN) {
                        printf("Interval value too small (%u), setting to default (%u)\n\r", g_SConfig.pSUM34C_config->dwTimerInterval, UM34C_INTERVAL_DEFAULT);
                        logger(log_lvl_warning, "main", "Interval value too small (%u), setting to default (%u)", g_SConfig.pSUM34C_config->dwTimerInterval, UM34C_INTERVAL_DEFAULT);
                        g_SConfig.pSUM34C_config->dwTimerInterval = UM34C_INTERVAL_DEFAULT;
                    } else if(g_SConfig.pSUM34C_config->dwTimerInterval > UM34C_INTERVAL_MAX) {
                        printf("Interval value too big (%u), setting to default (%u)\n\r", g_SConfig.pSUM34C_config->dwTimerInterval, UM34C_INTERVAL_DEFAULT);
                        logger(log_lvl_warning, "main", "Interval value too big (%u), setting to default (%u)", g_SConfig.pSUM34C_config->dwTimerInterval, UM34C_INTERVAL_DEFAULT);
                        g_SConfig.pSUM34C_config->dwTimerInterval = UM34C_INTERVAL_DEFAULT;
                    }
                } else {
                    printf("Error on entering interval\n\r");
                    logger(log_lvl_error, "main", "Error on entering interval");
                    return 1;
                }
            } else if(strcmp(argv[i], "-csv") == 0) {
                // Save UM34C's data in CSV file
                g_SConfig.bSaveToCSVfile = TRUE;
            } else if(strcmp(argv[i], "-r") == 0) {
                g_SConfig.bReadData = TRUE;
            } else if (strcmp(argv[i], "-h") == 0 || 
                       strcmp(argv[i], "--help") == 0) {
                displayHelp(0, 0, FALSE);
                return 0;
            } else if (strcmp(argv[i], "-f") == 0) {
                // Moving average filter strength
                if(++i < argc) {
                    g_SConfig.wMovAvgStrength = strtoul(argv[i], NULL, 10);
                    if(g_SConfig.wMovAvgStrength == 0) { 
                        printf("Error parsing moving average filter strength\n\r");
                        logger(log_lvl_error, "main", "Error parsing moving average filter strength");
                        return 1;
                    } else if(g_SConfig.wMovAvgStrength < MOV_AVG_FILTER_STRENGTH_MIN) {
                        printf("Interval value too small (%u), minimal value is %u\n\r", g_SConfig.wMovAvgStrength, MOV_AVG_FILTER_STRENGTH_MIN);
                        logger(log_lvl_warning, "main", "Interval value too small (%u), minimal value is %u", g_SConfig.wMovAvgStrength, MOV_AVG_FILTER_STRENGTH_MIN);
                        printf("Using use moving average filter is optional.\n\r");
                        return 1;
                    } else if(g_SConfig.wMovAvgStrength > MOV_AVG_FILTER_STRENGTH_MAX_FLOAT) {
                        printf("Interval value too big (%u), setting to maximal (%u)\n\r", g_SConfig.wMovAvgStrength, MOV_AVG_FILTER_STRENGTH_MAX_FLOAT);
                        logger(log_lvl_warning, "main", "Interval value too big (%u), setting to maximal (%u)", g_SConfig.wMovAvgStrength, MOV_AVG_FILTER_STRENGTH_MAX_FLOAT);
                        g_SConfig.wMovAvgStrength = MOV_AVG_FILTER_STRENGTH_MAX_FLOAT;
                    }
                } else {
                    printf("Error on entering moving average filter strength\n\r");
                    logger(log_lvl_error, "main", "Error on entering moving average filter strength");
                    return 1;
                }
            } else if (strcmp(argv[i], "-i") == 0) {
                // Set current threshold
                if(++i < argc) {
                    g_SConfig.byRecCurrThreshold_cA = strtoul(argv[i], NULL, 10);
                    if(g_SConfig.byRecCurrThreshold_cA == 0) { 
                        printf("Error parsing record current threshold or it was set to 0\n\r");
                        logger(log_lvl_error, "main", "Error parsing record current threshold or it was set to 0");
                        return 1;
                    } else if(g_SConfig.byRecCurrThreshold_cA > UM34C_SRCT_MAX) {
                        printf("Record current threshold value too big (%u), setting to maximal (%u)\n\r", g_SConfig.byRecCurrThreshold_cA, UM34C_SRCT_MAX);
                        logger(log_lvl_warning, "main", "Record current threshold value too big (%u), setting to maximal (%u)", g_SConfig.byRecCurrThreshold_cA, UM34C_SRCT_MAX);
                        g_SConfig.byRecCurrThreshold_cA = UM34C_SRCT_MAX;
                    }
                } else {
                    printf("Error on entering record current threshold\n\r");
                    logger(log_lvl_error, "main", "Error on entering record current threshold");
                    return 1;
                }
            // } else if (strcmp(argv[i], "-g") == 0) {
                // Set data groups (reset it also?)
            } else if(strcmp(argv[i], "-n") == 0) {
                // No GUI
                g_SConfig.bUseGUI = FALSE;
            }
        }
    }


    if(g_SConfig.wMovAvgStrength != 0) {
        // Looks like we are using moving average filter
        if(movingAvg_init_f(&g_SConfig.SMvgAvg_handle_fVoltage, g_SConfig.wMovAvgStrength) == TRUE) {
            printf("Filed to init moving average: fVoltage\n\r");
            logger(log_lvl_error, "main", "Filed to init moving average: fVoltage");
            return 1;
        }
    
        if(movingAvg_init_f(&g_SConfig.SMvgAvg_handle_fCurrent, g_SConfig.wMovAvgStrength) == TRUE) {
            printf("Filed to init moving average: fCurrent\n\r");
            logger(log_lvl_error, "main", "Filed to init moving average: fCurrent");
            return 1;
        }
    }    

    // printf("Address (after): %s\n\r", g_SConfig.szDestDevAddr);
    // printf("Interval value: %u\n\r", g_SConfig.dwTimerInterval);

    #ifndef UM34C_NOT_IN_RANGE
    // Check if UM34C device address was passed through command line and if it is "OK"
    if(strchr(g_SConfig.pSUM34C_config->szDestDevAddr, ':') == NULL) {
        // Check if it was previously found/connected and stored in file
        if(bReadDevAddrFromFile(g_SConfig.pSUM34C_config->szDestDevAddr)) {
            // Address stored in file, using that address
            printf("Using device address which was stored in file: %s\n\r", g_SConfig.pSUM34C_config->szDestDevAddr);
            logger(log_lvl_info, "main", "Using device address which was stored in file: %s", g_SConfig.pSUM34C_config->szDestDevAddr);
        } else {
            // Address not passed, try to find device with name "UM34C"
            printf("UM34C device address not passed, nor found in file, searching for device...\n\r");
            logger(log_lvl_info, "main", "UM34C device address not passed, nor found in file, searching for device...");
            if(UM34C_bGetDestDevAddr(g_SConfig.pSUM34C_config->szDestDevAddr)) {
                printf("Found UM34C device with address: %s\n\r", g_SConfig.pSUM34C_config->szDestDevAddr);
                logger(log_lvl_info, "main", "Found UM34C device with address: %s", g_SConfig.pSUM34C_config->szDestDevAddr);
            } else {
                printf("No UM34C device found!\n\r");
                logger(log_lvl_error, "main", "No UM34C device found!");
                return 1;
            }
        }
        // strncpy(g_SConfig.szDestDevAddr, UM34C_ADDR, UM34C_ADDR_LEN);
    } else {
        // Address passed, checking if it is "OK"
        if(g_SConfig.pSUM34C_config->szDestDevAddr[2] != ':' || 
           g_SConfig.pSUM34C_config->szDestDevAddr[5] != ':' || 
           g_SConfig.pSUM34C_config->szDestDevAddr[8] != ':' ||
           g_SConfig.pSUM34C_config->szDestDevAddr[11] != ':' || 
           g_SConfig.pSUM34C_config->szDestDevAddr[14] != ':') {
            printf("Error address value (%s)\n\r", g_SConfig.pSUM34C_config->szDestDevAddr);
            logger(log_lvl_error, "main", "Error address value (%s)", g_SConfig.pSUM34C_config->szDestDevAddr);
            return 1;
        }
    }
    // Copy device address from string to byte array
    str2ba(g_SConfig.pSUM34C_config->szDestDevAddr, &g_SConfig.pSUM34C_config->abyDestDevAddr);

    // Connect to UM34C
    if(UM34C_bConnectToBtAddapter(g_SConfig.pSUM34C_config) == FALSE) {
        printf("Filed to connect to UM34C device\n\r");
        logger(log_lvl_error, "main", "Filed to connect to UM34C device");
        return 1;
    }
    #endif  // UM34C_NOT_IN_RANGE

    // Everyting OK, store UM34C device address in a file, for later usage 
    // (so next time there is no need to search for device or pass it through command line argument)
    storeDevAddrToFile(g_SConfig.pSUM34C_config->szDestDevAddr);

    #ifndef UM34C_NOT_IN_RANGE
    if(g_SConfig.byRecCurrThreshold_cA != 0) {
        // Looks like we need to set record current threshold (it was set by program input parameter)
        tryToSetRCT();
    }
    #endif  // UM34C_NOT_IN_RANGE



    // File is created on first attempt to append data to it...
    // // Create new CSV file in which to store UM34C's data
    // if(g_SConfig.bSaveToCSVfile) {
    //     if(bMakeNewCSVfile(g_SConfig.pSFileHandler_config)) {
    //         logger(log_lvl_debug, "main", "CSV file created %s", g_SConfig.pSFileHandler_config->szCSVfileName);
    //     } else {
    //         logger(log_lvl_error, "main", "Error on creating CSV file");
    //     }
    // }

    // uint8_t inputMode = 1;
    // char inputBuff[4];
    // int8_t hexCommand;
    // printf("Enter command (hex), to leave loop press 'x'\n\r");
    // while(1) {
    //     fgets(inputBuff, sizeof(inputBuff)/sizeof(inputBuff[0]), stdin);

    //     // Check if loop should be exited
    //     if(inputBuff[0] == 'x') {
    //         printf("Exiting command mode\n\r");
    //         break;
    //     }

    //     printf("Got: %s", inputBuff);
    //     hexCommand = convertStringToHex(inputBuff);
    //     printf("Decoded command: %d\n", (uint8_t)hexCommand);
    //     if(hexCommand == -1) {
    //         printf("Error - HEX value not decoded\n\r");
    //     } else {
    //         buffSend[0] = (uint8_t)hexCommand;
    //         status = write(socketHandle, (void*)buffSend, 1);
    //         if(status != 1) {
    //             printf("Error - sending message\n\r");
    //         }
    //     }
    // }

    if(g_SConfig.bUseGUI == TRUE) {
        // Create display thread (only if GUI is used)
        pthread_create(&threadDisplay, NULL, threadDisplayStuff, NULL);
    }

    // Create thread to request data from UM34C
    pthread_create(&threadUM34C, NULL, threadReadData_UM34C, NULL);

    while(1) {       

        if(g_SConfig.bUseGUI == TRUE) {
            // Check for user inputs (only if GUI is used)
            g_SConfig.nCmdChar = getch();
            if(g_SConfig.nCmdChar != ERR) {
               checkForCommand();
               g_SConfig.bUpdateDisp = TRUE;
            }
        }

        usleep(INTREVAL_CHECK_FOR_CHAR);  // sleep time should be the as long as timer interval
    }


    return 0;
}