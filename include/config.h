#ifndef __config_h
#define __config_h

// #define UM34C_NOT_IN_RANGE

#define DEBUG_PROGRAM 0  // 1 or 0

#if (DEBUG_PROGRAM)
#define CHECK_NULL_PTR 0  // 1 or 0
#define BREAKPOINT  while(1){}
#else
#define BREAKPOINT
#endif // DEBUG_PROGRAM



#endif  // __config_h