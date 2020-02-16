

#include "fileHandler.h"
#include "time.h"
#include "config.h"

fileHandler_config_S g_SFileHandler_config;

uint8_t bReadDevAddrFromFile(char *pszDevAddr) {
    FILE * fp;
    uint8_t bAddressStoredInFile = FALSE;
    int nTmpChar;
    int i = 0;

    // Try to read address from file
    fp = fopen (UM34C_ADDR_FILE_NAME, "r");
    if(fp != NULL) {
        while(1) {
            nTmpChar = fgetc(fp);
            if(nTmpChar == EOF) {
                break;
            }
            pszDevAddr[i++] = nTmpChar;
        }

        pszDevAddr[i] = '\0';
        if(i == UM34C_ADDR_LEN-1) {
            bAddressStoredInFile = TRUE;
            // printf("Address read from file: %s\n\r", pszDevAddr);
        } else {
            printf("Address stored in file is corrupted\n\r");
        }
        
    }

    if(fp != NULL) {
        fclose(fp);
    }

    return bAddressStoredInFile;
}

void storeDevAddrToFile(char *pszDevAddr) {
    FILE * fp;

    // Create new file or erase old file and create new one
    fp = fopen (UM34C_ADDR_FILE_NAME, "w");
    if(fp != NULL) {
        fprintf(fp, "%s", pszDevAddr);
        fclose(fp);
    }
}

void makeNewCSVfile(fileHandler_config_S *pSConfig) {
    FILE * fp;
    time_t currentTime;
    struct tm *tm;

    pSConfig->dwNumbOfAppends = 0;
    
    currentTime = time(NULL);
    tm = localtime(&currentTime);
    // printf("strftime: %d\n\r", (strftime(g_SConfig.SCurrentData.szTimeDate, DATE_TIME_STRING_SIZE, DATE_TIME_STRING_FORMAT, tm)));
    strftime(pSConfig->szCSVfileName, UM34C_DATA_FILE_NAME_LEN, UM34C_DATA_FILE_NAME, tm);
    printf("Creating file: %s\n\r", pSConfig->szCSVfileName);

    // Create new file or erase old file and create new one
    fp = fopen(pSConfig->szCSVfileName, "w");
    if(fp != NULL) {
        fprintf(fp, "%s", UM34C_DATA_CSV_HEADERS);

        pSConfig->bFileCreated_CSV = TRUE;
        fclose(fp);
    }
}

void appendToCSVfile(fileHandler_config_S *pSConfig, um34c_data_S *pSData) {
    FILE * fp;
    #ifdef UM34C_NOT_IN_RANGE
    static float s_fTmp = 0;
    #endif  // UM34C_NOT_IN_RANGE

    if(pSConfig->bFileCreated_CSV == FALSE) {
        makeNewCSVfile(pSConfig);
    }
    // Create new file or erase old file and create new one
    fp = fopen (pSConfig->szCSVfileName, "a");

    if(fp != NULL) {

        #ifdef UM34C_NOT_IN_RANGE
        s_fTmp += 1;
        pSData->fVoltage = s_fTmp;
        #endif  // UM34C_NOT_IN_RANGE

        // CSV headers: "Time;Voltage;Current;Power;Resistance;Temperature;mAh;mWh;\n"
        fprintf(fp, "%s;%0.2f;%01.3f;%0.3f;%0.1f;%d;%d;%d;\n", pSData->szTimeDate, 
                pSData->fVoltage, pSData->fCurrent, pSData->fPower,
                pSData->fResistance, pSData->byTemperatureC, 
                pSData->wCapacity_mAh[pSData->bySelectedGroup],
                pSData->wCapacity_mWh[pSData->bySelectedGroup]
                );
        pSConfig->dwNumbOfAppends++;
        // fprintf(fp, "%s;%0.2f;%01.3f;\n", pSData->szTimeDate, s_fTmp, s_fTmp);
    } else {
        printf("Failed to open CSV file!\n\r");
    }

    if(fp != NULL) {
        fclose(fp);
    }
}

long int getFileSize(char *pszFileName) {
    FILE *fp;
    long int size=0;
     
    // Open file in Read Mode
    fp = fopen(pszFileName,"r");
 
    if(fp != NULL) {
        // Move file point at the end of file.
        fseek(fp,0,SEEK_END);
        
        // Get the current position of the file pointer.
        size = ftell(fp);
    }

    return size;
}

