/*
 * https://people.csail.mit.edu/albert/bluez-intro/c404.html
 * https://github.com/sebastianha/um34c
 * https://linux.die.net/man/2/setitimer
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
#include "customPrint.h"


um34c_config_S g_SConfig;

int main(int argc, char **argv)
{
    printf("Program started\n\r");

    memset(g_SConfig.szDestDevAddr, '0', UM34C_ADDR_LEN);
    g_SConfig.dwTimerInterval = UM34C_INTERVAL_DEFAULT;
    // g_SConfig.bSaveToCSVfile = FALSE;
    g_SConfig.bSaveToCSVfile = TRUE;

    // Check for passed arguments
    if(argc > 1) {
        for(uint8_t i=1; i < argc; i++) {
            if(strcmp(argv[i], "-a") == 0) {
                // Device address
                if(++i < argc) {
                    strncpy(g_SConfig.szDestDevAddr, argv[i], UM34C_ADDR_LEN);
                } else {
                    printf("Error on entering address\n\r");
                    return 1;
                }
            } else if (strcmp(argv[i], "-i") == 0) {
                // Timer interval
                if(++i < argc) {
                    g_SConfig.dwTimerInterval = strtoul(argv[i], NULL, 10);
                    if(g_SConfig.dwTimerInterval == 0) { 
                        printf("Error parsing interval value\n\r");
                        return 1;
                    } else if(g_SConfig.dwTimerInterval < UM34C_INTERVAL_MIN) {
                        printf("Interval value too small (%u), setting to default (%u)\n\r", g_SConfig.dwTimerInterval, UM34C_INTERVAL_DEFAULT);
                        g_SConfig.dwTimerInterval = UM34C_INTERVAL_DEFAULT;
                    } else if(g_SConfig.dwTimerInterval > UM34C_INTERVAL_MAX) {
                        printf("Interval value too big (%u), setting to default (%u)\n\r", g_SConfig.dwTimerInterval, UM34C_INTERVAL_DEFAULT);
                        g_SConfig.dwTimerInterval = UM34C_INTERVAL_DEFAULT;
                    }
                } else {
                    printf("Error on entering interval\n\r");
                    return 1;
                }
            } else if(strcmp(argv[i], "-CSV") == 0) {
                // Save UM34C's data in CSV file
                g_SConfig.bSaveToCSVfile = TRUE;
            }
        }
    }

    // printf("Address (after): %s\n\r", g_SConfig.szDestDevAddr);
    // printf("Interval value: %u\n\r", g_SConfig.dwTimerInterval);

    // Check if UM34C device address was passed through command line and if it is "OK"
    if(strchr(g_SConfig.szDestDevAddr, ':') == NULL) {
        // Check if it was previously found/connected and stored in file
        if(bReadDevAddrFromFile(g_SConfig.szDestDevAddr)) {
            // Address stored in file, using that address
            printf("Using device address which was stored in file: \"%s\"\n\r", g_SConfig.szDestDevAddr);
        } else {
            // Address not passed, try to find device with name "UM34C"
            printf("UM34C device address not passed, nor found in file, searching for device...\n\r");
            if(bGetDestDevAddr(g_SConfig.szDestDevAddr)) {
                printf("Found UM34C device with address: %s\n\r", g_SConfig.szDestDevAddr);
            } else {
                printf("No UM34C device found!\n\r");
                return 1;
            }
        }
        // strncpy(g_SConfig.szDestDevAddr, UM34C_ADDR, UM34C_ADDR_LEN);
    } else {
        // Address passed, checking if it is "OK"
        if(g_SConfig.szDestDevAddr[2] != ':' || g_SConfig.szDestDevAddr[5] != ':' || g_SConfig.szDestDevAddr[8] != ':' ||
           g_SConfig.szDestDevAddr[11] != ':' || g_SConfig.szDestDevAddr[14] != ':') {
           printf("Error address value (%s)\n\r", g_SConfig.szDestDevAddr);
           return 1;
        }
    }
    // Copy device address from string to byte array
    str2ba(g_SConfig.szDestDevAddr, &g_SConfig.abyDestDevAddr);

    // Connect to UM34C
    if(bConnectToBtAddapter(&g_SConfig.nSocketHandle, &g_SConfig.abyDestDevAddr, &g_SConfig.nStatus) != 0) {
        printf("Filed to connect to UM34C device\n\r");
        // return 1;
    }

    // Everyting OK, store UM34C device address in a file, for later usage 
    // (so next time there is no need to search for device or pass it through command line argument)
    storeDevAddrToFile(g_SConfig.szDestDevAddr);

    // Create new CSV file in which to store UM34C's data
    if(g_SConfig.bSaveToCSVfile) {
        makeNewCSVfile(g_SConfig.szCSVfileName);
    }

    uint8_t inputMode = 1;
    char inputBuff[4];
    int8_t hexCommand;
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


    g_SConfig.pbRequestData = createTimer(g_SConfig.dwTimerInterval);
    printf("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\r");
    uint8_t abyBuff[UM34C_MSG_SIZE] = {0};
    uint8_t byBuffSend;
    time_t currentTime;
    struct tm *tm;

    while(1) {
        // if(getchar() == 'x') {
        //     break;
        // }
        
        if(*g_SConfig.pbRequestData == TRUE) {
            // byBuffSend = um34c_cmd_getData;
            // if(write(g_SConfig.nSocketHandle, (void*)&byBuffSend, 1) != 1) {
            //     printf("Error sending request\n\r");
            // }
            // // // 0d4c
            // // status = read(socketHandle, (void*)buff, 1);
            // // if(status == 1) {
            // //     buff[1] = 0;
            // //     if(strcmp(buff, "0d") == 0) {
            // //         status = read(socketHandle, &(buff[1]), 1);
            // //         if(status == 1) {
            // //             buff[2] = 0;
            // //             if(strcmp(buff, "0d4c") == 0) {
            // //                 if(read(socketHandle, &(buff[2]), UM34C_MSG_SIZE-2) != UM34C_MSG_SIZE-2) {
            // //                     printf("Error receiving data\n\r");
            // //                     for(uint8_t i=0; i<UM34C_MSG_SIZE; i++) {
            // //                         printf("  0x%02X", buff[i]);
            // //                         if((i+1) % 16 == 0) {
            // //                             printf("\n\r");
            // //                         }
            // //                     }
            // //                     printf("\n\r");
            // //                 } else {
            // //                     decodeData(buff, &um34c_data);
            // //                 }
            // //             }
            // //         }
            // //     }
            // // }
            // // g_bRequestData = 0;

            // // printf("Reading device...\n\r");
            // g_SConfig.nStatus = read(g_SConfig.nSocketHandle, abyBuff, UM34C_MSG_SIZE);
            // currentTime = time(NULL);
            // tm = localtime(&currentTime);
            // strftime(g_SConfig.SCurrentData.szTimeDate, DATE_TIME_STRING_SIZE, DATE_TIME_STRING_FORMAT, tm);
            // printf("date: %s\n\r", g_SConfig.SCurrentData.szTimeDate);
            // // printf("Read %d bytes\n\r", status);    
            // if(g_SConfig.nStatus == UM34C_MSG_SIZE) {
            //     decodeData(abyBuff, &g_SConfig.SCurrentData);
            // }
            // // for(i=0; i<UM34C_MSG_SIZE; i++) {
            // //     printf("  0x%02X", buff[i]);
            // //     if((i+1) % 16 == 0) {
            // //         printf("\n\r");
            // //     }
            // // }
            // // printf("\n\r");

            currentTime = time(NULL);
            tm = localtime(&currentTime);
            // printf("strftime: %d\n\r", (strftime(g_SConfig.SCurrentData.szTimeDate, DATE_TIME_STRING_SIZE, DATE_TIME_STRING_FORMAT, tm)));
            strftime(g_SConfig.SCurrentData.szTimeDate, DATE_TIME_STRING_SIZE, DATE_TIME_STRING_FORMAT, tm);
            printf("date: %s\n\r", g_SConfig.SCurrentData.szTimeDate);

            appendToCSVfile(g_SConfig.szCSVfileName, &g_SConfig.SCurrentData);

            *g_SConfig.pbRequestData = FALSE;
        }
        


        usleep(1000*1000);  // sleep time should be the as long as timer interval

    }

    close(g_SConfig.nSocketHandle);

    return 0;
}