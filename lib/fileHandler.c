

#include "fileHandler.h"
#include "time.h"


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
    fprintf(fp, "%s", pszDevAddr);
   
    fclose(fp);
}

void makeNewCSVfile(char *pszCSVfileName) {
    FILE * fp;
    time_t currentTime;
    struct tm *tm;

    currentTime = time(NULL);
    tm = localtime(&currentTime);
    // printf("strftime: %d\n\r", (strftime(g_SConfig.SCurrentData.szTimeDate, DATE_TIME_STRING_SIZE, DATE_TIME_STRING_FORMAT, tm)));
    strftime(pszCSVfileName, UM34C_DATA_FILE_NAME_LEN, UM34C_DATA_FILE_NAME, tm);
    printf("Creating file: %s\n\r", pszCSVfileName);

    // Create new file or erase old file and create new one
    fp = fopen (pszCSVfileName, "w");
    fprintf(fp, "%s", UM34C_DATA_CSV_HEADERS);
   
    fclose(fp);
}

void appendToCSVfile(char *pszCSVfileName, um34c_data_S *pSData) {
    FILE * fp;
    static float s_fTmp = 0;

    // Create new file or erase old file and create new one
    fp = fopen (pszCSVfileName, "a");

    if(fp != NULL) {
        // "Time;Voltage;Current;Power;Resistance;Temperature;mAh;mWh;\n"
        // printf("Voltage: %0.2f V\n\r", data->fVoltage);
        // printf("Current: %01.3f A\n\r", data->fCurrent);
        // printf("Power: %0.3f W\n\r", data->fPower);
        // printf("Resistance: %0.1f Ohm\n\r", data->fResistance);
        // printf("Temperature: %d*C\n\r", data->byTemperatureC);
        // printf("mAh: %d\n\r", data->wCapacity_mAh[data->bySelectedGroup]);
	    // printf("mWh: %d\n\r", data->wCapacity_mWh[data->bySelectedGroup])


        s_fTmp += 1;
        pSData->fVoltage = s_fTmp;
        fprintf(fp, "%s;%0.2f;%01.3f;%0.3f;%0.1f;%d;%d;%d;\n", pSData->szTimeDate, 
                pSData->fVoltage, pSData->fCurrent, pSData->fPower,
                pSData->fResistance, pSData->byTemperatureC, 
                pSData->wCapacity_mAh[pSData->bySelectedGroup],
                pSData->wCapacity_mWh[pSData->bySelectedGroup]
                );
        // fprintf(fp, "%s;%0.2f;%01.3f;\n", pSData->szTimeDate, s_fTmp, s_fTmp);
    } else {
        printf("Failed to open CSV file!\n\r");
    }

    if(fp != NULL) {
        fclose(fp);
    }
}

