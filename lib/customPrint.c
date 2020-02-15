#include "customPrint.h"

void cursorUpOneLine(void) {
    printf("\033[F");  // move cursor up 1 line
}

void cursorUpLines(uint8_t lines) {
    while(lines-- != 0) {
        cursorUpOneLine();
    }
}
void cursorUpLinesClear(uint8_t lines) {
    while(lines-- != 0) {
        deleteLine();
        cursorUpOneLine();
    }
}

void deleteLine(void) {
    printf("\033[K");  // delete line
} 