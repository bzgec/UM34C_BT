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
        UM34C_prettyPrintData(&g_SConfig.pSUM34C_config->SCurrentData, FALSE);

        // If there were no readings than no data could be saved to file...
        if(g_SConfig.pSFileHandler_config->bFileCreated_CSV == TRUE) {
            printf("Number of appends to CSV file: %d\n\r", g_SConfig.pSFileHandler_config->dwNumbOfAppends);
            logger(log_lvl_info, "main", "Number of appends to CSV file: %d", g_SConfig.pSFileHandler_config->dwNumbOfAppends);
            printf("File size: %0.1f kB\n\r", getFileSize(g_SConfig.pSFileHandler_config->szCSVfileName)/1000.0);
            logger(log_lvl_info, "main", "File size: %0.1f kB", getFileSize(g_SConfig.pSFileHandler_config->szCSVfileName)/1000.0);
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
    uint8_t byAppendRetVal;

    g_SConfig.pSFileHandler_config->dwNumbOfAppends = 0;

    while(1) {
        if(g_SConfig.bReadData) {
            #ifndef UM34C_NOT_IN_RANGE
            UM34C_bGetData(g_SConfig.pSUM34C_config);

            #else  // #ifndef UM34C_NOT_IN_RANGE

            currentTime = time(NULL);
            tm = localtime(&currentTime);
            strftime(g_SConfig.pSUM34C_config->SCurrentData.szTimeDate, DATE_TIME_STRING_SIZE, DATE_TIME_STRING_FORMAT, tm);

            #endif  // UM34C_NOT_IN_RANGE

            if(g_SConfig.bSaveToCSVfile) {
                byAppendRetVal = byAppendToCSVfile(g_SConfig.pSFileHandler_config, &g_SConfig.pSUM34C_config->SCurrentData);
                if(byAppendRetVal == 2) {
                    logger(log_lvl_debug, "main", "CSV file created %s", g_SConfig.pSFileHandler_config->szCSVfileName);
                } else if (byAppendRetVal == 0) {
                    logger(log_lvl_error, "main", "Error on appending to CSV file");
                }
            }
        }
       
        usleep(g_SConfig.pSUM34C_config->dwTimerInterval);  // sleep time should be the as long as timer interval
    }
}

int main(int argc, char **argv) {
    pthread_t threadUM34C;
    pthread_t threadDisplay;

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
            } else if (strcmp(argv[i], "-i") == 0) {
                // Timer interval
                if(++i < argc) {
                    g_SConfig.pSUM34C_config->dwTimerInterval = strtoul(argv[i], NULL, 10);
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
            }
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

    // Create display thread
    pthread_create(&threadDisplay, NULL, threadDisplayStuff, NULL);

    // Create thread to request data from UM34C
    pthread_create(&threadUM34C, NULL, threadReadData_UM34C, NULL);

    g_SConfig.startTime = time(NULL);

    while(1) {       

        g_SConfig.nCmdChar = getch();

        usleep(INTREVAL_CHECK_FOR_CHAR);  // sleep time should be the as long as timer interval
    }


    return 0;
}