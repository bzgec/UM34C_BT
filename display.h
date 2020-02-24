#ifndef __display_h
#define __display_h

#include <stdint.h>

void displayTrueFalse(uint16_t wY, uint16_t wX, uint8_t bDispTrue);
void displayHelp(uint16_t *wY, uint16_t *wX, uint8_t bUseNcurses);
void *threadDisplayStuff(void *arg);

#endif