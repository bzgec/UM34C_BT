#ifndef __customPrintf_h
#define __customPrintf_h

#include "dataTypes.h"
#include <stdio.h>

void deleteLine(void);
void cursorUpOneLine(void);
void cursorUpLines(uint8_t lines);
void cursorUpLinesClear(uint8_t lines);

#endif  // __customPrintf_h