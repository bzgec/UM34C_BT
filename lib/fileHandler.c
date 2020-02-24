

#include "fileHandler.h"
#include "time.h"
#include "config.h"
#include "logger.h"

fileHandler_config_S g_SFileHandler_config;

uint8_t bReadDevAddrFromFile(char *pszDevAddr) {
    FILE * fp;
    uint8_t bAddressStoredInFile = FALSE;
    int nTmpChar;
    int i = 0;

    if(pszDevAddr != NULL) {
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
            } else {
                logger(log_lvl_warning, "fileHandler", "Stored UM34C address in '%s' is not correct: '%s'", UM34C_ADDR_FILE_NAME, pszDevAddr);
            }

            fclose(fp);
        } else {
            logger(log_lvl_error, "fileHandler", "Reading device address file - failed to open '%s'", UM34C_ADDR_FILE_NAME);
        }
    }

    return bAddressStoredInFile;
}

void storeDevAddrToFile(char *pszDevAddr) {
    FILE * fp;

    if(pszDevAddr != NULL) {
        // Create new file or erase old file and create new one
        fp = fopen (UM34C_ADDR_FILE_NAME, "w");
        if(fp != NULL) {
            fprintf(fp, "%s", pszDevAddr);
            fclose(fp);
        } else {
            logger(log_lvl_error, "fileHandler", "Storing UM34C address to file - failed to open '%s', device address '%s'",
                   UM34C_ADDR_FILE_NAME, pszDevAddr);
        }
    }
}

uint8_t bMakeNewCSVfile(fileHandler_config_S *pSConfig) {
    FILE * fp;
    time_t currentTime;
    struct tm *tm;
    uint8_t bRetVal = FALSE;

    if(pSConfig != NULL) {
        pSConfig->dwNumbOfAppends = 0;

        currentTime = time(NULL);
        tm = localtime(&currentTime);
        strftime(pSConfig->szCSVfileName, UM34C_DATA_FILE_NAME_LEN, UM34C_DATA_FILE_NAME, tm);

        // Create new file or erase old file and create new one
        fp = fopen(pSConfig->szCSVfileName, "w");
        if(fp != NULL) {
            fprintf(fp, "%s", UM34C_DATA_CSV_HEADERS);

            pSConfig->bFileCreated_CSV = TRUE;
            bRetVal = TRUE;
            fclose(fp);
        } else {
            logger(log_lvl_error, "fileHandler", "Creating new CSV file - failed to open '%s'", pSConfig->szCSVfileName);
        }
    }

    return bRetVal;
}

// Retrun value: 0-ERROR, 1-OK, 2-FILE CREATED ('2' has precedence)
fileHandler_info_E byAppendToCSVfile(fileHandler_config_S *pSConfig, um34c_data_S *pSData) {
    FILE * fp;
    fileHandler_info_E ERetVal = fileHandler_info_FAILED_TO_OPEN_FILE;
    #ifdef UM34C_NOT_IN_RANGE
    static float s_fTmp = 0;
    #endif  // UM34C_NOT_IN_RANGE

    if(pSConfig != NULL) {
        if(pSConfig->bFileCreated_CSV == FALSE) {
            if(bMakeNewCSVfile(pSConfig) == TRUE) {
                ERetVal = fileHandler_info_FILE_CREATED;
            }
        }
        // Create new file or erase old file and create new one
        fp = fopen (pSConfig->szCSVfileName, "a");

        if(fp != NULL) {

            #ifdef UM34C_NOT_IN_RANGE
            s_fTmp += 1;
            pSData->fVoltage = s_fTmp;
            #endif  // UM34C_NOT_IN_RANGE

            // CSV headers: "Time;Voltage;Current;Power;Resistance;Temperature;mAh;mWh;\n"
            // CSV headers: "Time;Voltage;Current;Temperature;mAh;mWh;\n"
            // Power and resistance can be calculated from Voltage and current!
            // fprintf(fp, "%s;%0.2f;%01.3f;%0.3f;%0.1f;%d;%d;%d;\n", pSData->szTimeDate, 
            fprintf(fp, "%s;%0.2f;%01.3f;%d;%d;%d;\n", pSData->szTimeDate, 
                    pSData->fVoltage, pSData->fCurrent, /*pSData->fPower,
                    pSData->fResistance,*/ pSData->byTemperatureC, 
                    pSData->wCapacity_mAh[pSData->bySelectedGroup],
                    pSData->wCapacity_mWh[pSData->bySelectedGroup]
                    );
            pSConfig->dwNumbOfAppends++;

            // Check if file was created
            if(ERetVal != fileHandler_info_FILE_CREATED) {
                ERetVal = fileHandler_info_OK;
            }

            fclose(fp);
        } else {
            ERetVal = fileHandler_info_FAILED_TO_OPEN_FILE;
            logger(log_lvl_error, "fileHandler", "Appending to file - failed to open '%s'", pSConfig->szCSVfileName);
        }
    } else {
        ERetVal = fileHandler_info_NULL;
    }


    return ERetVal;
}

// Retrun file size in bytes
long int getFileSize(char *pszFileName) {
    FILE *fp;
    long int size=0;
    long int retVal = -1;
     
    if(pszFileName != NULL) {
        // Open file in Read Mode
        fp = fopen(pszFileName,"r");
    
        if(fp != NULL) {
            // Move file point at the end of file.
            fseek(fp,0,SEEK_END);

            // Get the current position of the file pointer.
            size = ftell(fp);

            if(size == -1) {
                retVal = -1;
                logger(log_lvl_error, "fileHandler", "Failed to find end position of '%s'", pszFileName);
            } else {
                retVal = size;
            }

            fclose(fp);

        } else {
            logger(log_lvl_error, "fileHandler", "Trying to get file size - failed to open '%s'", pszFileName);
        }  
    }

    return retVal;
}

// Check if file is bigger than 'dwMaxFileSizeB' (it is in bytes)
fileHandler_info_E fileHandler_checkIfTooBig(char *pszFileName, uint32_t dwMaxFileSizeB) {
    long int fileSize;
    fileHandler_info_E ERetVal = fileHandler_info_FILE_TOO_BIG;
    
    if(pszFileName != NULL) {
        fileSize = getFileSize(pszFileName);
        printf("File size is %ld\n\r", fileSize);

        if(fileSize == -1) {
            ERetVal = fileHandler_info_FAILED_TO_GET_FILE_SIZE;
            printf("Failed to get file size\n\r");
        } else {
            if(fileSize >= dwMaxFileSizeB) {
                printf("File is too big!\n\r");
            } else {
                ERetVal = fileHandler_info_OK;
                printf("File size OK\n\r");
            }
        }
    } else {
       ERetVal = fileHandler_info_NULL; 
    }

    return ERetVal;
}

