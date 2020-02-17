/*
 * https://people.csail.mit.edu/albert/bluez-intro/c404.html
 * https://github.com/sebastianha/um34c
 * https://linux.die.net/man/2/setitimer
 * 
 * 00:15:A3:00:2D:BF  UM34C
 */
// #include "main.h"
#include "um34c.h"
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
#include <string.h>
#include <ncurses.h>
#include "customPrint.h"
#include "config.h"
#include "logger.h"

static int16_t fs_sConvertStringToHex(char *cmd);
static uint8_t fs_byGetNum(char ch);

um34c_config_S g_SUM34C_config;
uint8_t g_pbRequestData = FALSE;

void UM34C_init(um34c_config_S *pSUM34C_config) {
    if(pSUM34C_config != NULL) {
        memset(pSUM34C_config->szDestDevAddr, '0', UM34C_ADDR_LEN);  // Clear address string
        strcpy(pSUM34C_config->SCurrentData.szTimeDate, DATE_TIME_STRING_INIT);  // Clear date time string
        pSUM34C_config->dwTimerInterval = UM34C_INTERVAL_DEFAULT;  // Set interval at which data from UM34C is read to default
        pSUM34C_config->bInitialized = TRUE;
    } else {
        BREAKPOINT
    }
}

void UM34C_deinit(um34c_config_S *pSUM34C_config) {
    if(pSUM34C_config != NULL) {
        close(pSUM34C_config->nSocketHandle);
    } else {
        BREAKPOINT
    }
}

uint8_t UM34C_bGetDestDevAddr(char *pszUM34CAddress) {
    inquiry_info *ii = NULL;
    int max_rsp, num_rsp;
    int dev_id, sock, len, flags;
    int i;
    char addr[19] = { 0 };
    char name[248] = { 0 };
    uint8_t bUM34CdeviceFound = FALSE;

    // dev_id = connectToBtAddapter();
    // Get bluetooth adapter ID
    printf("Getting bluetooth adapter ID... ");
    logger(log_lvl_info, "um34c", "Getting bluetooth adapter ID... ");

    dev_id = hci_get_route(NULL);
    // dev_id = hci_devid( "01:23:45:67:89:AB" );  // connect to specific adapter
    printf("ID: %d\n\r", dev_id);
    logger(log_lvl_info, "um34c", "ID: %d", dev_id);

    // Connect to adapter 
    printf("Connecting to adapter... ");
    logger(log_lvl_info, "um34c", "Connecting to adapter... ");
    sock = hci_open_dev( dev_id );  // connection to the microcontroller on the specified local Bluetooth adapter
    if (dev_id < 0 || sock < 0) {
        printf("\n\r Error on opening socket\n\r");
        logger(log_lvl_error, "um34c", "Opening socket");
        exit(1);
    } else {
        printf("done\n\r");
        logger(log_lvl_info, "um34c", "done");
    }

    // scan for nearby Bluetooth devices
    len  = 8;  // The inquiry lasts for at most 1.28 * 'len' seconds
    max_rsp = 255;  // max number of returned devices
    flags = IREQ_CACHE_FLUSH;  // cache of previously detected devices is flushed before performing the current inquiry
    // flags = 0;  // cache of previously detected devices is flushed before performing the current inquiry
    printf("malloc... ");
    logger(log_lvl_info, "um34c", "malloc... ");
    ii = (inquiry_info*)malloc(max_rsp * sizeof(inquiry_info));  // in 'ii' deteceted devices data is returned, must support max number of devices ('max_rsp')
    printf("done\n");
    logger(log_lvl_info, "um34c", "done");

    printf("Scanning for nearby devices... ");
    logger(log_lvl_info, "um34c", "Scanning for nearby devices... ");
    num_rsp = hci_inquiry(dev_id, len, max_rsp, NULL, &ii, flags);
    if( num_rsp < 0 ) {
        printf("\n\r Error on hci_inquiry\n\r");
        logger(log_lvl_error, "um34c", "Error on hci_inquiry");
    } else {
        printf("done\n\r");
        logger(log_lvl_info, "um34c", "done");
    }
    printf("Number of detected devices: %d\n\r", num_rsp);
    logger(log_lvl_info, "um34c", "Number of detected devices: %d", num_rsp);

    for (i = 0; i < num_rsp; i++) {
        ba2str(&(ii+i)->bdaddr, addr);
        memset(name, 0, sizeof(name));
        // Try to get user-friendly name
        if (hci_read_remote_name(sock, &(ii+i)->bdaddr, sizeof(name), name, 0) < 0) {
            strcpy(name, "[unknown]");
        }
        printf("%s  %s\n", addr, name);
        logger(log_lvl_info, "um34c", "%s  %s", addr, name);
        if(strcmp(name, "UM34C") == 0) {
        // if(strcmp(name, "[unknown]") == 0) {
            printf("Address noted\n\r");
            logger(log_lvl_info, "um34c", "Address noted");
            strncpy(pszUM34CAddress, addr, UM34C_ADDR_LEN);
            bUM34CdeviceFound = TRUE;
        }
    }

    free( ii );
    close(sock);

    return bUM34CdeviceFound;
}

uint8_t UM34C_bConnectToBtAddapter(um34c_config_S *pSUM34C_config) {
    uint8_t bRetVal = FALSE;
    struct sockaddr_rc SSocketAddr;

    if(pSUM34C_config != NULL) {
        // Allocate a socket
        printf("Allocating socket...");
        logger(log_lvl_info, "um34c", "Allocating socket...");
        pSUM34C_config->nSocketHandle = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);
        if(pSUM34C_config->nSocketHandle == -1) {
            printf(" FAILED\n\r");
            logger(log_lvl_error, "um34c", "FAILED");
        } else {
            printf(" done\n\r");
            logger(log_lvl_info, "um34c", "done");
            // Set the connection parameters (who to connect to)
            SSocketAddr.rc_family = AF_BLUETOOTH;
            SSocketAddr.rc_channel = (uint8_t) 1;
            for(uint8_t i=0; i < sizeof(bdaddr_t); i++) {
                SSocketAddr.rc_bdaddr.b[i] =  pSUM34C_config->abyDestDevAddr.b[i];
            }

            // Connect to server
            printf("Connecting to socket...");
            logger(log_lvl_info, "um34c", "Connecting to socket...");
            pSUM34C_config->nStatus = connect(pSUM34C_config->nSocketHandle, (struct sockaddr *)&SSocketAddr, UM34C_ADDR_LEN);
            if(pSUM34C_config->nStatus == 0) {
                printf(" done\n\r");
                logger(log_lvl_info, "um34c", "done");
                bRetVal = TRUE;
            } else {
                printf(" FAILED\n\r");
                logger(log_lvl_error, "um34c", "FAILED");
            }
        }

    } else {
        BREAKPOINT
    }

    return bRetVal;
}

// http://www.informit.com/articles/article.aspx?p=23618&seqNum=14
void createTimer(um34c_config_S *pSUM34C_config) {
    struct sigaction sa;
    struct itimerval timer;

    if(pSUM34C_config != NULL) {
        /* Install timer_handler as the signal handler for SIGVTALRM. */
        printf("Installing timer handler\n\r");
        logger(log_lvl_info, "um34c", "Installing timer handler");
        memset (&sa, 0, sizeof (sa));
        sa.sa_handler = &timer_handler;
        // sigaction(SIGVTALRM, &sa, NULL);
        sigaction(SIGALRM, &sa, NULL);

        printf("Setting timer...");
        logger(log_lvl_info, "um34c", "Setting timer...");
        timer.it_value.tv_sec = 0;
        timer.it_value.tv_usec = pSUM34C_config->dwTimerInterval;
        timer.it_interval.tv_sec = 0;
        timer.it_interval.tv_usec = pSUM34C_config->dwTimerInterval;
        /* Start a virtual timer. It counts down whenever this process is
         * executing. 
         * ITIMER_REAL -> delivers SIGALRM upon expiration
         * ITIMER_VIRTUAL -> delivers SIGVTALRM upon expiration
         * ITIMER_PROF -> delivers SIGPROF upon expiration
         */
        // if(setitimer(ITIMER_VIRTUAL, &timer, NULL) == -1) {
        if(setitimer(ITIMER_REAL, &timer, NULL) == -1) {
            printf(" FAILED\n\r");
            logger(log_lvl_error, "um34c", "FAILED");
        } else {
            printf(" done\n\r");
            logger(log_lvl_info, "um34c", "done");
        }

        // Connect timer expired flag
        pSUM34C_config->pbRequestData = &g_pbRequestData;
    } else {
        BREAKPOINT
    }
}

void timer_handler (int signum) {
    if(signum == SIGALRM) {
        g_pbRequestData = TRUE;
    }
}

uint8_t UM34C_bGetData(um34c_config_S *pSConfig) {
    uint8_t abyBuff[UM34C_MSG_SIZE] = {0};
    time_t currentTime;
    struct tm *tm;
    uint8_t bRetVal = FALSE;

    // Send query
    UM34C_sendCmd(pSConfig->nSocketHandle, um34c_cmd_getData, &pSConfig->nStatus);
    if(pSConfig->nStatus == UM34C_SEND_CMD_SIZE) {
        // Data sent successfully, now read data from device
        // printf("Reading device...\n\r");
        UM34C_readCmd(pSConfig->nSocketHandle, abyBuff, UM34C_MSG_SIZE, &pSConfig->nStatus);
        // Check if data was received correctly (first 4 chars should be '0d4c' and 'UM34C_MSG_SIZE' bytes should be received)
        // printf("Read %d bytes\n\r", pSConfig->nStatus);    
        if(abyBuff[0] == '0' && abyBuff[1] == 'd' && abyBuff[2] == '4' && abyBuff[3] == 'c' && pSConfig->nStatus == UM34C_MSG_SIZE) {
            currentTime = time(NULL);
            tm = localtime(&currentTime);
            strftime(pSConfig->SCurrentData.szTimeDate, DATE_TIME_STRING_SIZE, DATE_TIME_STRING_FORMAT, tm);
            // printf("date: %s\n\r", pSConfig->SCurrentData.szTimeDate);
            UM34C_decodeData(abyBuff, &pSConfig->SCurrentData);
            bRetVal = TRUE;
        } else {
            printf("Error on receiving data from UM34C (received %d bytes)\n\r", pSConfig->nStatus);
            logger(log_lvl_error, "um34c", "Error on receiving data from UM34C (received %d bytes)", pSConfig->nStatus);
        }
    } else {
        printf("Error on sending data to UM34C");
        logger(log_lvl_error, "um34c", "Error on sending data to UM34C");
    }
    
    // for(i=0; i<UM34C_MSG_SIZE; i++) {
    //     printf("  0x%02X", buff[i]);
    //     if((i+1) % 16 == 0) {
    //         printf("\n\r");
    //     }
    // }
    // printf("\n\r");

    return bRetVal;
}

// If 'UM34C_SEND_CMD_SIZE' is returned everything is OK
void UM34C_sendCmd(int nSocketHandle, uint8_t byBuffSend, int *pnStatus) {
    *pnStatus = write(nSocketHandle, (void*)&byBuffSend, UM34C_SEND_CMD_SIZE);
}

// Retrun value must be the same as 'size'
void UM34C_readCmd(int nSocketHandle, uint8_t *pabyBuff, size_t size, int *pnStatus) {
    *pnStatus = read(nSocketHandle, pabyBuff, size);
}

void UM34C_prettyPrintData(um34c_data_S *pSData, uint8_t bUseNcurses) {
    uint16_t wX = 0;
    uint16_t wY = 0;
    uint16_t wXOffset = 50;
    uint16_t wYOffset = 0;
    char *pszModes [] = {
		"Unknown",  // 0
		"QC2.0",    // 1
		"QC3.0",    // 2
		"APP2.4A",  // 3
		"APP2.1A",  // 4
		"APP1.0A",  // 5
		"APP0.5A",  // 6
		"DCP1.5A",  // 7
		"SAMSUNG"   // 8
    };
    
    if(bUseNcurses) {
        mvwprintw(stdscr, wYOffset + wY++, wXOffset + wX, "Last data:");
        wX += 4;
        mvwprintw(stdscr, wYOffset + wY++, wXOffset + wX, "Time: %s", pSData->szTimeDate);
        mvwprintw(stdscr, wYOffset + wY++, wXOffset + wX, "Voltage: %0.2f V", pSData->fVoltage);
        mvwprintw(stdscr, wYOffset + wY++, wXOffset + wX, "Current: %01.3f A", pSData->fCurrent);
        mvwprintw(stdscr, wYOffset + wY++, wXOffset + wX, "Power: %0.3f W", pSData->fPower);
        mvwprintw(stdscr, wYOffset + wY++, wXOffset + wX, "Resistance: %0.1f Ohm", pSData->fResistance);
        mvwprintw(stdscr, wYOffset + wY++, wXOffset + wX, "Temperature: %d*C", pSData->byTemperatureC);
        mvwprintw(stdscr, wYOffset + wY++, wXOffset + wX, "Selected group: %d", pSData->bySelectedGroup);
        mvwprintw(stdscr, wYOffset + wY++, wXOffset + wX, "Capacity [mAh]: %d", pSData->wCapacity_mAh[pSData->bySelectedGroup]);
	    mvwprintw(stdscr, wYOffset + wY++, wXOffset + wX, "Capacity [mWh]: %d", pSData->wCapacity_mWh[pSData->bySelectedGroup]);
        mvwprintw(stdscr, wYOffset + wY++, wXOffset + wX, "Current screen: %d", pSData->byCurrentScreen);
        mvwprintw(stdscr, wYOffset + wY++, wXOffset + wX, "Brightness: %d", pSData->byBrightness);
        mvwprintw(stdscr, wYOffset + wY++, wXOffset + wX, "Screen timeout: %d", pSData->byScreenTimeout);
        mvwprintw(stdscr, wYOffset + wY++, wXOffset + wX, "USB + data line: %0.2f", pSData->fUsbDataPlus);
        mvwprintw(stdscr, wYOffset + wY++, wXOffset + wX, "USB - data line: %0.2f", pSData->fUsbDataMinus);
        mvwprintw(stdscr, wYOffset + wY++, wXOffset + wX, "Charging mode: %s", pszModes[pSData->byChargingMode]);
    } else {
        printf("Last data:\n\r");
        printf("\tTime: %s\n\r", pSData->szTimeDate);
        printf("\tVoltage: %0.2f V\n\r", pSData->fVoltage);
        printf("\tCurrent: %01.3f A\n\r", pSData->fCurrent);
        printf("\tPower: %0.3f W\n\r", pSData->fPower);
        printf("\tResistance: %0.1f Ohm\n\r", pSData->fResistance);
        printf("\tTemperature: %d*C\n\r", pSData->byTemperatureC);
        printf("\tSelected group: %d\n\r", pSData->bySelectedGroup);
        printf("\tCapacity [mAh]: %d\n\r", pSData->wCapacity_mAh[pSData->bySelectedGroup]);
	    printf("\tCapacity [mWh]: %d\n\r", pSData->wCapacity_mWh[pSData->bySelectedGroup]);
        printf("\tCurrent screen: %d\n\r", pSData->byCurrentScreen);
        printf("\tBrightness: %d\n\r", pSData->byBrightness);
        printf("\tScreen timeout: %d\n\r", pSData->byScreenTimeout);
        printf("\tUSB + data line: %0.2f\n\r", pSData->fUsbDataPlus);
        printf("\tUSB - data line: %0.2f\n\r", pSData->fUsbDataMinus);
        printf("\tCharging mode: %s\n\r", pszModes[pSData->byChargingMode]);
    }
    
    // for(uint8_t g=0; g<10; g++) {
	// 	printf("  %d  mAh: %d", g, pSData->mAh[g]);
	// 	printf("  mWh: %d\n\r", pSData->mWh[g]);
	// }
}

// https://github.com/sebastianha/um34c
void UM34C_decodeData(uint8_t *buf, um34c_data_S *pSData) {

    /* Data from device
	 * 0    2    4    6    8    10   12   14   16   18   20   22   24   26   28   30
	 * 0d4c 01f5 008e 0000 02c7 001b 0051 0001 0000 0000 0000 0000 0000 0007 0000 0027 
	 *      V___ A___ W________ tC__ tF__ grou mAh_0____ mWh_0____ mAh_1____ mWh_1____
	 * 
	 * 32   34   36   38   40   42   44   46   48   50   52   54   56   58   60   62
	 * 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000
	 * mAh_2____ mWh_2____ mAh_3____ mWh_3____ mAh_4____ mWh_4____ mAh_5____ mWh_5____ 
	 * 
	 * 64   66   68   70   72   74   76   78   80   82   84   86   88   90   92   94
	 * 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 
	 * mAh_6____ mWh_6____ mAh_7____ mWh_7____ mAh_8____ mWh_8____ mAh_9____ mWh_9____
	 * 
	 * 96   98   100  102  104  106  108  110  112  114  116  118  120  124  126  128  130
	 * 0002 0001 0007 0000 0007 0000 0025 0013 0000 0046 0000 0001 0004 0000 0160 0000 8068
	 * d+__ d-__ mode recA_____ recW_____ recT recTime__ con? tout brig ohm______ scre
	 */
    // Voltage in "V"
	pSData->fVoltage = ((buf[2] << 8) + buf[3]) / 100.0;
	pSData->fCurrent = ((buf[4] << 8) + buf[5]) / 1000.0;
	pSData->fPower = ((buf[6] << 8*3) + (buf[7] << 8*2) + (buf[8] << 8*1) + (buf[9] << 8*0)) / 1000.0;
 	pSData->byTemperatureC = ((buf[10] << 8) + buf[11]);
    pSData->bySelectedGroup = (buf[14] << 8) + buf[15];
    pSData->fUsbDataPlus = ((buf[96] << 8) + buf[97]) / 100.0;
    pSData->fUsbDataMinus = ((buf[98] << 8) + buf[99]) / 100.0;
    pSData->byChargingMode = (buf[100] << 8) + buf[101];
    pSData->byScreenTimeout = (buf[118] << 8) + buf[119];
    pSData->byBrightness = (buf[120] << 8) + buf[121];
	pSData->fResistance = ((buf[122] << 8*3) + (buf[123] << 8*2) + (buf[124] << 8*1) + (buf[125] << 8*0)) / 10.0;
    pSData->byCurrentScreen = (buf[126] << 8) + buf[127];

    // Fill groups with data
	for(uint8_t g=0; g<10; g++) {
		pSData->wCapacity_mAh[g] = (buf[16+8*g] << 8*3) + (buf[17+8*g] << 8*2) + (buf[18+8*g] << 8*1) + (buf[19+8*g] << 8*0);
		pSData->wCapacity_mWh[g] = (buf[20+8*g] << 8*3) + (buf[21+8*g] << 8*2) + (buf[22+8*g] << 8*1) + (buf[23+8*g] << 8*0);
	}
}

uint8_t fs_byGetNum(char ch) {
    uint8_t returnVal = 255;  // 255 is error value

    if(ch >= '0' && ch <= '9') {
        returnVal = ch - '0';
    } else {
        switch (ch) {
        case 'A':
        case 'a':
            returnVal = 10;
            break;
        case 'B':
        case 'b':
            returnVal = 11;
            break;
        case 'C':
        case 'c':
            returnVal = 12;
            break;
        case 'D':
        case 'd':
            returnVal = 13;
            break;
        case 'E':
        case 'e':
            returnVal = 14;
            break;
        case 'F':
        case 'f':
            returnVal = 15;
            break;
        default:
            break;
        }
    }

    return returnVal;
}

int16_t fs_sConvertStringToHex(char *cmd) {
    int16_t convertedVal[2];
    
    convertedVal[0] = fs_byGetNum(cmd[0]);
    convertedVal[1] = fs_byGetNum(cmd[1]);

    if(convertedVal[0] == 255 || convertedVal[1] == 255) {
        convertedVal[0] = -1;  // error
    } else {
        convertedVal[0] = (convertedVal[0])*16 + convertedVal[1];
    }

    return convertedVal[0];
}