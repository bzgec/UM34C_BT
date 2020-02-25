#ifndef __movingAverage_h
#define __movingAverage_h

#include <stdint.h>

#define MOV_AVG_FILTER_STRENGTH_MAX_BYTE  20  // max filter strength
#define MOV_AVG_FILTER_STRENGTH_MAX_WORD  20  // max filter strength
#define MOV_AVG_FILTER_STRENGTH_MAX_FLOAT 1200  // max filter strength (2*60*10 -> 2 samples per second - save every 10minutes)

// Which type of variables are you using:
// #define USING_BYTE
// #define USING_WORD
#define USING_FLOAT

#define MOV_AVG_FILTER_STRENGTH_MAX 65535
#define MOV_AVG_FILTER_STRENGTH_MIN     2



#ifndef NULL
#define NULL 0
#endif  // NULL

#ifndef TRUE
#define TRUE 1
#endif  // TRUE

#ifndef FALSE
#define FALSE 0
#endif  // FALSE



#ifdef USING_BYTE
typedef struct movingAvg_handle_by_STRUCTURE {
    uint8_t abyOldNumbers[MOV_AVG_FILTER_STRENGTH_MAX_BYTE];
    uint8_t byAverage;
    uint32_t dwSum;
    uint16_t wFilterStrength;
    uint16_t wPos;
    uint16_t wStartPos;   
    uint8_t bIsInitialized;
} movingAvg_handle_by_S;
#endif  // USING_BYTE

#ifdef USING_WORD
typedef struct movingAvg_handle_w_STRUCTURE {
    uint16_t awOldNumbers[MOV_AVG_FILTER_STRENGTH_MAX_WORD];
    uint16_t wAverage;
    uint32_t dwSum;
    uint16_t wFilterStrength;
    uint16_t wPos;
    uint16_t wStartPos;   
    uint8_t bIsInitialized;
} movingAvg_handle_w_S;
#endif  // USING_WORD

#ifdef USING_FLOAT
typedef struct movingAvg_handle_f_STRUCTURE {
    float afOldNumbers[MOV_AVG_FILTER_STRENGTH_MAX_FLOAT];
    float fAverage;
    float fSum;
    uint16_t wFilterStrength;
    uint16_t wPos;
    uint16_t wStartPos;
    uint8_t bIsInitialized;
} movingAvg_handle_f_S;
#endif  // USING_FLOAT


#if (MOV_AVG_FILTER_STRENGTH_MAX_BYTE  > 65535 || \
     MOV_AVG_FILTER_STRENGTH_MAX_WORD  > 65535 || \
     MOV_AVG_FILTER_STRENGTH_MAX_FLOAT > 65535)
#error 'MOV_AVG_FILTER_STRENGTH_MAX_x' should not be hihger than 65535!
#endif

#if (MOV_AVG_FILTER_STRENGTH_MAX_BYTE  < 2 || \
     MOV_AVG_FILTER_STRENGTH_MAX_WORD  < 2 || \
     MOV_AVG_FILTER_STRENGTH_MAX_FLOAT < 2)
#error 'MOV_AVG_FILTER_STRENGTH_MAX_x' should not be smaller than 2!
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifdef USING_BYTE
uint8_t movingAvg_init_by(movingAvg_handle_by_S *pHandle, uint16_t wFilterStrength);
uint8_t movingAvg_calc_by(movingAvg_handle_by_S *pHandle, uint8_t byNewValue);
#endif  // USING_BYTE

#ifdef USING_WORD
uint8_t movingAvg_init_w(movingAvg_handle_w_S *pHandle, uint16_t wFilterStrength);
uint16_t movingAvg_calc_w(movingAvg_handle_w_S *pHandle, uint16_t wNewValue);
#endif  // USING_WORD

#ifdef USING_FLOAT
uint8_t movingAvg_init_f(movingAvg_handle_f_S *pHandle, uint16_t wFilterStrength);
float movingAvg_calc_f(movingAvg_handle_f_S *pHandle, float fNewValue);
#endif  // USING_FLOAT

#ifdef __cplusplus
}
#endif




#endif  // __movingAverage_h