#include "movingAverage.h"

// code copied and modified from from https://gist.github.com/bmccormack/d12f4bf0c96423d03f82

static void my_memset(void *pDest, char cValue, uint16_t wLen);


static void my_memset(void *pDest, char cValue, uint16_t wLen) {
    uint8_t *pbyDest;
    if(pDest != NULL) {
        pbyDest = pDest;
        while(wLen > 0) {
            *pbyDest = cValue;
            pbyDest++;
            wLen--;
        }
    }
}

#ifdef USING_BYTE
uint8_t movingAvg_init_by(movingAvg_handle_by_S *pHandle, uint16_t wFilterStrength) {
    uint8_t bError = TRUE;
    if(pHandle != NULL) {
        my_memset(pHandle, 0, sizeof(movingAvg_handle_by_S));
        if(wFilterStrength <= MOV_AVG_FILTER_STRENGTH_MAX_BYTE) {
            pHandle->bIsInitialized = TRUE;
            pHandle->wFilterStrength = wFilterStrength;
            bError = FALSE;
        }
    }

    return bError;
}

uint8_t movingAvg_calc_by(movingAvg_handle_by_S *pHandle, uint8_t byNewValue) {
    if(pHandle != NULL) {
        if(pHandle->bIsInitialized == TRUE) {
            // Subtract the oldest number from previous sum and add new number
            pHandle->dwSum = pHandle->dwSum - pHandle->abyOldNumbers[pHandle->wPos] + byNewValue;

            // Assign the new value to the position in the array
            pHandle->abyOldNumbers[pHandle->wPos++] = byNewValue;

            if(pHandle->wPos >= pHandle->wFilterStrength) {
                pHandle->wPos = 0;
            }

            // Calculate the average
            if(pHandle->wStartPos < pHandle->wFilterStrength) {
                pHandle->wStartPos++;
                pHandle->byAverage = pHandle->dwSum / pHandle->wStartPos;
            } else {
                pHandle->byAverage = pHandle->dwSum / pHandle->wFilterStrength;
            }
        } else {
            pHandle->byAverage = 0;
        }
    }  
    
    return pHandle->byAverage;
}
#endif  // USING_BYTE


#ifdef USING_WORD
uint8_t movingAvg_init_w(movingAvg_handle_w_S *pHandle, uint16_t wFilterStrength) {
    uint8_t bError = TRUE;
    if(pHandle != NULL) {
        my_memset(pHandle, 0, sizeof(movingAvg_handle_w_S));
        if(wFilterStrength <= MOV_AVG_FILTER_STRENGTH_MAX_WORD) {
            pHandle->bIsInitialized = TRUE;
            pHandle->wFilterStrength = wFilterStrength;
            bError = FALSE;
        }
    }
    return bError;
}

uint16_t movingAvg_calc_w(movingAvg_handle_w_S *pHandle, uint16_t wNewValue) {
    if(pHandle != NULL) {
        if(pHandle->bIsInitialized == TRUE) {
            // Subtract the oldest number from previous sum and add new number
            pHandle->dwSum = pHandle->dwSum - pHandle->awOldNumbers[pHandle->wPos] + wNewValue;

            // Assign the new value to the position in the array
            pHandle->awOldNumbers[pHandle->wPos++] = wNewValue;

            if(pHandle->wPos >= pHandle->wFilterStrength) {
                pHandle->wPos = 0;
            }

            // Calculate the average
            if(pHandle->wStartPos < pHandle->wFilterStrength) {
                pHandle->wStartPos++;
                pHandle->wAverage = pHandle->dwSum / pHandle->wStartPos;
            } else {
                pHandle->wAverage = pHandle->dwSum / pHandle->wFilterStrength;
            }
        } else {
            pHandle->wAverage = 0;
        }

    }
    
    return pHandle->wAverage;
}
#endif  // USING_WORD

#ifdef USING_FLOAT
uint8_t movingAvg_init_f(movingAvg_handle_f_S *pHandle, uint16_t wFilterStrength) {
    uint8_t bError = TRUE;
    if(pHandle != NULL) {
        my_memset(pHandle, 0, sizeof(movingAvg_handle_f_S));
        if(wFilterStrength <= MOV_AVG_FILTER_STRENGTH_MAX_FLOAT) {
            pHandle->bIsInitialized = TRUE;
            pHandle->wFilterStrength = wFilterStrength;
            bError = FALSE;
        }
    }
    return bError;
}

float movingAvg_calc_f(movingAvg_handle_f_S *pHandle, float fNewValue) {
    if(pHandle != NULL) {
        if(pHandle->bIsInitialized == TRUE) {
            // Subtract the oldest number from previous sum and add new number
            pHandle->fSum = pHandle->fSum - pHandle->afOldNumbers[pHandle->wPos] + fNewValue;

            // Assign the new value to the position in the array
            pHandle->afOldNumbers[pHandle->wPos++] = fNewValue;

            if(pHandle->wPos >= pHandle->wFilterStrength) {
                pHandle->wPos = 0;
            }

            // Calculate the average
            if(pHandle->wStartPos < pHandle->wFilterStrength) {
                pHandle->wStartPos++;
                pHandle->fAverage = pHandle->fSum / pHandle->wStartPos;
            } else {
                pHandle->fAverage = pHandle->fSum / pHandle->wFilterStrength;
            }
        } else {
            pHandle->fAverage = 0;
        }
    }  
    
    return pHandle->fAverage;
}
#endif  // USING_FLOAT
