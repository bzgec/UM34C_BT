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




mainConfig_S g_SConfig;
extern um34c_config_S g_SUM34C_config;
extern fileHandler_config_S g_SFileHandler_config;

void exitProgram(void) {
    time_t currentTime;
    uint64_t second;
    uint8_t minute;
    uint8_t hour;

    currentTime = time(NULL);

    // Store running time in this variable
    currentTime -= g_SConfig.startTime;

    hour = currentTime/3600;
    second = currentTime % 3600;
    minute = second/60;
    second %= 60;

    endwin();  // dellocates memory and ends ncurses

    UM34C_deinit(g_SConfig.pSUM34C_config);

    if(g_SConfig.pSFileHandler_config->bFileCreated_CSV == TRUE) {
        printf("Number of appends to CSV file: %d\n\r", g_SConfig.pSFileHandler_config->dwNumbOfAppends);
        printf("File size: %0.1f kB\n\r", getFileSize(g_SConfig.pSFileHandler_config->szCSVfileName)/1000.0);
    }
    printf("Running time: %02d:%02d:%02ld\n\r", hour, minute, second);


    exit(0);
}

// Catch CTRL+C
void sigintHandler(int nSigNum) {
    exitProgram();
}

void displayHelp(uint16_t *wY, uint16_t *wX) {
    /* 
    * Exit application: `CTRL+C` or `c`
    * Set device brightness: `UP`/`DOWN`
    * Move to previous/next display: `LEFT`/`RIGHT`
    * Toggle data sampling: `SPACE`
    * Toggle saving to CSV file: `s`
    * Rotate screen: `r`
    * Set screen timeout: `0`-`9`
    */
    mvwprintw(stdscr, (*wY)++, *wX, "Exit application: `CTRL+C` or `c`");
    mvwprintw(stdscr, (*wY)++, *wX, "Set device brightness: `UP`/`DOWN`");
    mvwprintw(stdscr, (*wY)++, *wX, "Move to previous/next display: `LEFT`/`RIGHT`");
    mvwprintw(stdscr, (*wY)++, *wX, "Toggle data sampling: `SPACE`");
    mvwprintw(stdscr, (*wY)++, *wX, "Toggle saving to CSV file: `s`");
    mvwprintw(stdscr, (*wY)++, *wX, "Rotate screen: `r`");
    mvwprintw(stdscr, (*wY)++, *wX, "Set screen timeout: `0`-`9`");
}

// http://www.cs.ukzn.ac.za/~hughm/os/notes/ncurses.html
void *threadDisplayStuff(void *arg) {
    initscr(); // Initialize the window
    noecho(); // Don't echo any keypresses
    // curs_set(FALSE); // Don't display a cursor
    keypad(stdscr, TRUE);  // To capture special keystrokes like Backspace, Delete and the four arrow keys by getch()
    cbreak();  // no key buffering
    // nodelay(stdscr, TRUE);  // non-blocking getch()
    nodelay(stdscr, FALSE);  // blocking getch()

    uint16_t wX;
    uint16_t wY;
    char szLastCmdBuff[40];
    
    while(1) {
        wX = 0;
        wY = 0;

        wclear(stdscr); // Clear the screen of all

        displayHelp(&wY, &wX);
        wY += 5;
        mvwprintw(stdscr, wY++, wX, "Reading data: %s", (g_SConfig.bReadData == 0)? "FALSE" : "TRUE");
        mvwprintw(stdscr, wY++, wX, "Saving to CSV file: %s", (g_SConfig.bSaveToCSVfile == 0)? "FALSE" : "TRUE");
        mvwprintw(stdscr, wY++, wX, "Last command: ");
        mvwprintw(stdscr, wY++, wX+4, "%s", szLastCmdBuff);

        UM34C_prettyPrintData(&g_SConfig.pSUM34C_config->SCurrentData);


        if(g_SConfig.nCmdChar != g_SConfig.nCmdChar_prev) {
            if(g_SConfig.nCmdChar == ERR) {
                // no input from terminal
            } else {
                switch (g_SConfig.nCmdChar) {
                case KEY_UP:  // Brightness UP
                    g_SConfig.byDeviceBrightness = g_SConfig.pSUM34C_config->SCurrentData.byBrightness;
                    if(g_SConfig.byDeviceBrightness < um34c_cmd_setBrightness5-um34c_cmd_setBrightness0) {
                        g_SConfig.byDeviceBrightness++;
                    }
                    UM34C_sendCmd(g_SConfig.pSUM34C_config->nSocketHandle, (um34c_cmd_E)(g_SConfig.byDeviceBrightness + um34c_cmd_setBrightness0), &g_SConfig.pSUM34C_config->nStatus);
                    sprintf(szLastCmdBuff, "Brightness: %d (UP)", g_SConfig.byDeviceBrightness);
                    break;
                case KEY_DOWN:  // Brightness DOWN
                    g_SConfig.byDeviceBrightness = g_SConfig.pSUM34C_config->SCurrentData.byBrightness;
                    if(g_SConfig.byDeviceBrightness > um34c_cmd_setBrightness0-um34c_cmd_setBrightness0) {
                        g_SConfig.byDeviceBrightness--;
                    }
                    UM34C_sendCmd(g_SConfig.pSUM34C_config->nSocketHandle, (um34c_cmd_E)(g_SConfig.byDeviceBrightness + um34c_cmd_setBrightness0), &g_SConfig.pSUM34C_config->nStatus);
                    sprintf(szLastCmdBuff, "Brightness: %d (DOWN)", g_SConfig.byDeviceBrightness);
                    break;
                case KEY_LEFT:  // Previous display
                    UM34C_sendCmd(g_SConfig.pSUM34C_config->nSocketHandle, um34c_cmd_prev, &g_SConfig.pSUM34C_config->nStatus);
                    sprintf(szLastCmdBuff, "Previous display");
                   break;
                case KEY_RIGHT:  // Next display
                    UM34C_sendCmd(g_SConfig.pSUM34C_config->nSocketHandle, um34c_cmd_next, &g_SConfig.pSUM34C_config->nStatus);
                    sprintf(szLastCmdBuff, "Next display");
                    break;
                case 'c':  // Exit program
                    exitProgram();
                    break;
                case 0x20:  // Toggle data sampling - 'space'
                    g_SConfig.bReadData = !g_SConfig.bReadData;
                    sprintf(szLastCmdBuff, "Toggle data sampling.");
                    break;
                case 's':  // Toggle saving to CSV file
                    g_SConfig.bSaveToCSVfile = !g_SConfig.bSaveToCSVfile;
                    sprintf(szLastCmdBuff, "Toggle saving to CSV file.");
                    break;
                case 'r':  // rotate screen
                    UM34C_sendCmd(g_SConfig.pSUM34C_config->nSocketHandle, um34c_cmd_rotateClockwise, &g_SConfig.pSUM34C_config->nStatus);
                    g_SConfig.byCurrentScreenRotation++;
                    if(g_SConfig.byCurrentScreenRotation % 4 == 0) {
                        g_SConfig.byCurrentScreenRotation = 0;
                    }
                    sprintf(szLastCmdBuff, "Rotating screen: %d", g_SConfig.byCurrentScreenRotation);
                    break;
                case '0':  // set screen timeout
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                case '8':
                case '9':
                    UM34C_sendCmd(g_SConfig.pSUM34C_config->nSocketHandle, (g_SConfig.nCmdChar - '0') + um34c_cmd_setTimeout0, &g_SConfig.pSUM34C_config->nStatus);
                    sprintf(szLastCmdBuff, "Screen timeout: %d", g_SConfig.nCmdChar - '0');
                    break;
                default:
                    break;
                }
            }
            g_SConfig.nCmdChar = ERR;  // so that we can press 'UP' two times...
            g_SConfig.nCmdChar_prev = g_SConfig.nCmdChar;
        }

        wrefresh(stdscr);

        usleep(INTREVAL_UPDATE_DISPLAY);  // sleep time should be the as long as timer interval
    }
}

void *threadReadData(void *arg) {
    time_t currentTime;
    struct tm *tm;

    g_SConfig.pSFileHandler_config->dwNumbOfAppends = 0;

    while(1) {
        if(g_SConfig.bReadData) {
            #ifndef UM34C_NOT_IN_RANGE
            bUM34C_getData(g_SConfig.pSUM34C_config);

            #else  // #ifndef UM34C_NOT_IN_RANGE

            currentTime = time(NULL);
            tm = localtime(&currentTime);
            // printf("strftime: %d\n\r", (strftime(g_SConfig.SCurrentData.szTimeDate, DATE_TIME_STRING_SIZE, DATE_TIME_STRING_FORMAT, tm)));
            strftime(g_SConfig.pSUM34C_config->SCurrentData.szTimeDate, DATE_TIME_STRING_SIZE, DATE_TIME_STRING_FORMAT, tm);
            // printf("date: %s\n\r", g_SConfig.pSUM34C_config->SCurrentData.szTimeDate);

            #endif  // UM34C_NOT_IN_RANGE

            if(g_SConfig.bSaveToCSVfile) {
                appendToCSVfile(g_SConfig.pSFileHandler_config, &g_SConfig.pSUM34C_config->SCurrentData);
            }
        }
       

        usleep(g_SConfig.pSUM34C_config->dwTimerInterval);  // sleep time should be the as long as timer interval
    }
}

int main(int argc, char **argv) {
    pthread_t threadUM34C;
    pthread_t threadDisplay;

    printf("Program started\n\r");

    signal(SIGINT, sigintHandler);  // Catch CTRL+C

    g_SConfig.pSUM34C_config = &g_SUM34C_config;
    g_SConfig.pSFileHandler_config = &g_SFileHandler_config;

    UM34C_init(g_SConfig.pSUM34C_config);

    memset(g_SConfig.pSUM34C_config->szDestDevAddr, '0', UM34C_ADDR_LEN);
    g_SConfig.pSUM34C_config->dwTimerInterval = UM34C_INTERVAL_DEFAULT;
    g_SConfig.bSaveToCSVfile = FALSE;
    g_SConfig.bReadData =FALSE;
    g_SConfig.byDeviceBrightness = 3;
    g_SConfig.byCurrentScreenRotation = 0;
    g_SConfig.pSFileHandler_config->bFileCreated_CSV = FALSE;

    // Check for passed arguments
    if(argc > 1) {
        for(uint8_t i=1; i < argc; i++) {
            if(strcmp(argv[i], "-a") == 0) {
                // Device address
                if(++i < argc) {
                    strncpy(g_SConfig.pSUM34C_config->szDestDevAddr, argv[i], UM34C_ADDR_LEN);
                } else {
                    printf("Error on entering address\n\r");
                    return 1;
                }
            } else if (strcmp(argv[i], "-i") == 0) {
                // Timer interval
                if(++i < argc) {
                    g_SConfig.pSUM34C_config->dwTimerInterval = strtoul(argv[i], NULL, 10);
                    if(g_SConfig.pSUM34C_config->dwTimerInterval == 0) { 
                        printf("Error parsing interval value\n\r");
                        return 1;
                    } else if(g_SConfig.pSUM34C_config->dwTimerInterval < UM34C_INTERVAL_MIN) {
                        printf("Interval value too small (%u), setting to default (%u)\n\r", g_SConfig.pSUM34C_config->dwTimerInterval, UM34C_INTERVAL_DEFAULT);
                        g_SConfig.pSUM34C_config->dwTimerInterval = UM34C_INTERVAL_DEFAULT;
                    } else if(g_SConfig.pSUM34C_config->dwTimerInterval > UM34C_INTERVAL_MAX) {
                        printf("Interval value too big (%u), setting to default (%u)\n\r", g_SConfig.pSUM34C_config->dwTimerInterval, UM34C_INTERVAL_DEFAULT);
                        g_SConfig.pSUM34C_config->dwTimerInterval = UM34C_INTERVAL_DEFAULT;
                    }
                } else {
                    printf("Error on entering interval\n\r");
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

    // Check if UM34C device address was passed through command line and if it is "OK"
    if(strchr(g_SConfig.pSUM34C_config->szDestDevAddr, ':') == NULL) {
        // Check if it was previously found/connected and stored in file
        if(bReadDevAddrFromFile(g_SConfig.pSUM34C_config->szDestDevAddr)) {
            // Address stored in file, using that address
            printf("Using device address which was stored in file: \"%s\"\n\r", g_SConfig.pSUM34C_config->szDestDevAddr);
        } else {
            // Address not passed, try to find device with name "UM34C"
            printf("UM34C device address not passed, nor found in file, searching for device...\n\r");
            if(bGetDestDevAddr(g_SConfig.pSUM34C_config->szDestDevAddr)) {
                printf("Found UM34C device with address: %s\n\r", g_SConfig.pSUM34C_config->szDestDevAddr);
            } else {
                printf("No UM34C device found!\n\r");
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
            return 1;
        }
    }
    // Copy device address from string to byte array
    str2ba(g_SConfig.pSUM34C_config->szDestDevAddr, &g_SConfig.pSUM34C_config->abyDestDevAddr);

    // Connect to UM34C
    #ifndef UM34C_NOT_IN_RANGE
    if(bConnectToBtAddapter(g_SConfig.pSUM34C_config) == FALSE) {
        printf("Filed to connect to UM34C device\n\r");
        return 1;
    }
    #endif  // UM34C_NOT_IN_RANGE

    // Everyting OK, store UM34C device address in a file, for later usage 
    // (so next time there is no need to search for device or pass it through command line argument)
    storeDevAddrToFile(g_SConfig.pSUM34C_config->szDestDevAddr);

    // Create new CSV file in which to store UM34C's data
    if(g_SConfig.bSaveToCSVfile) {
        makeNewCSVfile(g_SConfig.pSFileHandler_config);
    }

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

    // createTimer(g_SConfig.pSUM34C_config);
    pthread_create(&threadUM34C, NULL, threadReadData, NULL);
    printf("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\r");

    time_t currentTime;
    struct tm *tm;
    int nTmp;

    g_SConfig.startTime = time(NULL);
    while(1) {
        // if(getchar() == 'x') {
        //     break;
        // }
        
        // if(*g_SConfig.pSUM34C_config->pbRequestData == TRUE) {
        //     #ifndef UM34C_NOT_IN_RANGE
        //     bUM34C_getData(g_SConfig.pSUM34C_config);
        //     // // // 0d4c
        //     // // status = read(socketHandle, (void*)buff, 1);
        //     // // if(status == 1) {
        //     // //     buff[1] = 0;
        //     // //     if(strcmp(buff, "0d") == 0) {
        //     // //         status = read(socketHandle, &(buff[1]), 1);
        //     // //         if(status == 1) {
        //     // //             buff[2] = 0;
        //     // //             if(strcmp(buff, "0d4c") == 0) {
        //     // //                 if(read(socketHandle, &(buff[2]), UM34C_MSG_SIZE-2) != UM34C_MSG_SIZE-2) {
        //     // //                     printf("Error receiving data\n\r");
        //     // //                     for(uint8_t i=0; i<UM34C_MSG_SIZE; i++) {
        //     // //                         printf("  0x%02X", buff[i]);
        //     // //                         if((i+1) % 16 == 0) {
        //     // //                             printf("\n\r");
        //     // //                         }
        //     // //                     }
        //     // //                     printf("\n\r");
        //     // //                 } else {
        //     // //                     UM34C_decodeData(buff, &um34c_data);
        //     // //                 }
        //     // //             }
        //     // //         }
        //     // //     }
        //     // // }
        //     // // g_pbRequestData = 0;


        //     #else  // #ifndef UM34C_NOT_IN_RANGE

        //     currentTime = time(NULL);
        //     tm = localtime(&currentTime);
        //     // printf("strftime: %d\n\r", (strftime(g_SConfig.SCurrentData.szTimeDate, DATE_TIME_STRING_SIZE, DATE_TIME_STRING_FORMAT, tm)));
        //     strftime(g_SConfig.pSUM34C_config->SCurrentData.szTimeDate, DATE_TIME_STRING_SIZE, DATE_TIME_STRING_FORMAT, tm);
        //     // printf("date: %s\n\r", g_SConfig.pSUM34C_config->SCurrentData.szTimeDate);

        //     #endif  // UM34C_NOT_IN_RANGE

        //     cursorUpLinesClear(10);
        //     deleteLine();
        //     printf("Number of appends: %d\n\r", g_SConfig.pSFileHandler_config->dwNumbOfAppends);
        //     UM34C_prettyPrintData(&g_SConfig.pSUM34C_config->SCurrentData);
        //     appendToCSVfile(g_SConfig.pSFileHandler_config, &g_SConfig.pSUM34C_config->SCurrentData);

        //     *g_SConfig.pSUM34C_config->pbRequestData = FALSE;
        // }
        
        g_SConfig.nCmdChar = getch();

        usleep(INTREVAL_CHECK_FOR_CHAR);  // sleep time should be the as long as timer interval
    }


    return 0;
}