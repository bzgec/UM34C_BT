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
#include "customPrint.h"

int16_t convertStringToHex(char *cmd);
uint8_t getNum(char ch);

uint8_t g_bRequestData = FALSE;

// https://github.com/sebastianha/um34c
void decodeData(uint8_t *buf, um34c_data_S *data) {

    char *modes [] = {
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

    /* Data from device
	 *      4    8    12   16   20   24   28   32   36   40   44   48   52   56   60
	 * 0d4c 01f5 008e 0000 02c7 001b 0051 0001 0000 0000 0000 0000 0000 0007 0000 0027 
	 *      V___ A___ W________ tC__ tF__ grou mAh_0____ mWh_0____ mAh_1____ mWh_1____
	 * 
	 * 64   68   72   76   80   84   88   92   96   100  104  108  112  116  120  124
	 * 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000
	 * mAh_2____ mWh_2____ mAh_3____ mWh_3____ mAh_4____ mWh_4____ mAh_5____ mWh_5____ 
	 * 
	 * 128  132  136  140  144  148  152  156  160  164  168  172  176  180  184  188
	 * 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 
	 * mAh_6____ mWh_6____ mAh_7____ mWh_7____ mAh_8____ mWh_8____ mAh_9____ mWh_9____
	 * 
	 * 192  196  200  204  208  212  216  220  224  228  232  236  240  244  248  252  256
	 * 0002 0001 0007 0000 0007 0000 0025 0013 0000 0046 0000 0001 0004 0000 0160 0000 8068
	 * d+__ d-__ mode recA_____ recW_____ recT recTime__ con? tout brig ohm______ scre
	 */
    // Voltage in "V"
	data->fVoltage = ((buf[2] << 8) + buf[3]) / 100.0;
	data->fCurrent = ((buf[4] << 8) + buf[5]) / 1000.0;
	data->fPower = ((buf[6] << 8*3) + (buf[7] << 8*2) + (buf[8] << 8*1) + (buf[9] << 8*0)) / 1000.0;
 	data->byTemperatureC = ((buf[10] << 8) + buf[11]);
    data->bySelectedGroup = (buf[14] << 8) + buf[15];
	data->fResistance = ((buf[122] << 8*3) + (buf[123] << 8*2) + (buf[124] << 8*1) + (buf[125] << 8*0)) / 10.0;
    data->byCurrentScreen = (buf[126] << 8) + buf[127];
    data->byBrightness = (buf[118] << 8) + buf[119];
    data->byScreenTimeout = (buf[120] << 8) + buf[121];

    // Fill groups with data
	for(uint8_t g=0; g<10; g++) {
		data->wCapacity_mAh[g] = (buf[16+8*g] << 8*3) + (buf[17+8*g] << 8*2) + (buf[18+8*g] << 8*1) + (buf[19+8*g] << 8*0);
		data->wCapacity_mWh[g] = (buf[20+8*g] << 8*3) + (buf[21+8*g] << 8*2) + (buf[22+8*g] << 8*1) + (buf[23+8*g] << 8*0);
	}



    cursorUpLinesClear(11);
    deleteLine();
    printf("Voltage: %0.2f V\n\r", data->fVoltage);
    printf("Current: %01.3f A\n\r", data->fCurrent);
    printf("Power: %0.3f W\n\r", data->fPower);
    printf("Resistance: %0.1f Ohm\n\r", data->fResistance);
    printf("Temperature: %d*C\n\r", data->byTemperatureC);
    printf("Selected group: %d\n\r", data->bySelectedGroup);
    printf("mAh: %d\n\r", data->wCapacity_mAh[data->bySelectedGroup]);
	printf("mWh: %d\n\r", data->wCapacity_mWh[data->bySelectedGroup]);
    printf("Current screen: %d\n\r", data->byCurrentScreen);
    printf("Brightness: %d\n\r", data->byBrightness);
    printf("Screen timeout: %d\n\r", data->byScreenTimeout);
    // for(uint8_t g=0; g<10; g++) {
	// 	printf("  %d  mAh: %d", g, data->mAh[g]);
	// 	printf("  mWh: %d\n\r", data->mWh[g]);
	// }

}

// http://www.informit.com/articles/article.aspx?p=23618&seqNum=14
uint8_t * createTimer(uint32_t dwInterval) {
    struct sigaction sa;
    struct itimerval timer;

    /* Install timer_handler as the signal handler for SIGVTALRM. */
    printf("Installing timer handler\n\r");
    memset (&sa, 0, sizeof (sa));
    sa.sa_handler = &timer_handler;
    // sigaction(SIGVTALRM, &sa, NULL);
    sigaction(SIGALRM, &sa, NULL);

    printf("Setting timer...");
    timer.it_value.tv_sec = 0;
    timer.it_value.tv_usec = dwInterval;
    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_usec = dwInterval;
    /* Start a virtual timer. It counts down whenever this process is
     * executing. 
     * ITIMER_REAL -> delivers SIGALRM upon expiration
     * ITIMER_VIRTUAL -> delivers SIGVTALRM upon expiration
     * ITIMER_PROF -> delivers SIGPROF upon expiration
     */
    // if(setitimer(ITIMER_VIRTUAL, &timer, NULL) == -1) {
    if(setitimer(ITIMER_REAL, &timer, NULL) == -1) {
        printf(" FAILED\n\r");
    } else {
        printf(" done\n\r");
    }

    return &g_bRequestData;
}

void timer_handler (int signum) {
    if(signum == SIGALRM) {
        g_bRequestData = TRUE;
    }
}


uint8_t getNum(char ch) {
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

int16_t convertStringToHex(char *cmd) {
    int16_t convertedVal[2];
    
    convertedVal[0] = getNum(cmd[0]);
    convertedVal[1] = getNum(cmd[1]);

    if(convertedVal[0] == 255 || convertedVal[1] == 255) {
        convertedVal[0] = -1;  // error
    } else {
        convertedVal[0] = (convertedVal[0])*16 + convertedVal[1];
    }

    return convertedVal[0];
}

uint8_t bGetDestDevAddr(char *pszUM34CAddress) {
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
    dev_id = hci_get_route(NULL);
    // dev_id = hci_devid( "01:23:45:67:89:AB" );  // connect to specific adapter
    printf("ID: %d\n\r", dev_id);

    // Connect to adapter 
    printf("Connecting to adapter... ");
    sock = hci_open_dev( dev_id );  // connection to the microcontroller on the specified local Bluetooth adapter
    if (dev_id < 0 || sock < 0) {
        perror("opening socket");
        exit(1);
    } else {
        printf("done\n\r");
    }

    //  scan for nearby Bluetooth devices
    len  = 8;  // The inquiry lasts for at most 1.28 * 'len' seconds
    max_rsp = 255;  // max number of returned devices
    flags = IREQ_CACHE_FLUSH;  // cache of previously detected devices is flushed before performing the current inquiry
    // flags = 0;  // cache of previously detected devices is flushed before performing the current inquiry
    printf("malloc... ");
    ii = (inquiry_info*)malloc(max_rsp * sizeof(inquiry_info));  // in 'ii' deteceted devices data is returned, must support max number of devices ('max_rsp')
    printf("done\n");

    printf("Scanning for nearby devices... ");
    num_rsp = hci_inquiry(dev_id, len, max_rsp, NULL, &ii, flags);
    if( num_rsp < 0 ) {
        perror("hci_inquiry");
    } else {
        printf("done\n\r");
    }
    printf("Number of detected devices: %d\n\r", num_rsp);

    for (i = 0; i < num_rsp; i++) {
        ba2str(&(ii+i)->bdaddr, addr);
        memset(name, 0, sizeof(name));
        // Try to get user-friendly name
        if (hci_read_remote_name(sock, &(ii+i)->bdaddr, sizeof(name), name, 0) < 0) {
            strcpy(name, "[unknown]");
        }
        printf("%s  %s\n", addr, name);
        if(strcmp(name, "UM34C") == 0) {
        // if(strcmp(name, "[unknown]") == 0) {
            printf("Address noted\n\r");
            strncpy(pszUM34CAddress, addr, UM34C_ADDR_LEN);
            bUM34CdeviceFound = TRUE;
        }
    }

    free( ii );
    close(sock);

    // int dev_id;

    // // Get bluetooth adapter ID
    // printf("Getting bluetooth adapter ID... ");
    // dev_id = hci_get_route(NULL);
    // // dev_id = hci_devid( "01:23:45:67:89:AB" );  // connect to specific adapter
    // printf("ID: %d\n\r", dev_id);

    // // Connect to adapter 
    // printf("Connecting to adapter... ");
    // sock = hci_open_dev( dev_id );  // connection to the microcontroller on the specified local Bluetooth adapter
    // if (dev_id < 0 || sock < 0) {
    //     perror("opening socket");
    //     exit(1);
    // } else {
    //     printf("done\n\r");
    // }
    return bUM34CdeviceFound;
}

uint8_t bConnectToBtAddapter(int *pnSocketHandle, bdaddr_t *pabyDestDevAddr, int *pnStatus) {
    struct sockaddr_rc SSocketAddr;

    // Allocate a socket
    printf("Allocating socket...");
    *pnSocketHandle = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);
    if(*pnSocketHandle == -1) {
        printf(" FAILED\n\r");
        return 1;
    } else {
        printf(" done\n\r");
    }

    // Set the connection parameters (who to connect to)
    SSocketAddr.rc_family = AF_BLUETOOTH;
    SSocketAddr.rc_channel = (uint8_t) 1;
    for(uint8_t i=0; i < sizeof(bdaddr_t); i++) {
        SSocketAddr.rc_bdaddr.b[i] = pabyDestDevAddr->b[i];
    }

    // Connect to server
    printf("Connecting to socket...");
    *pnStatus = connect(*pnSocketHandle, (struct sockaddr *)&SSocketAddr, UM34C_ADDR_LEN);
    if(*pnStatus == 0) {
        printf(" done\n\r");
    } else {
        printf(" FAILED\n\r");
        return 1;
    }

    return 0;
}



