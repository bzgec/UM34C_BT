#ifndef __dataTypes_h
#define __dataTypes_h

#include <stdint.h>


// typedef     data_type     new_name     Type prefix
typedef         uint8_t       BOOL;         // b
typedef         uint8_t       BYTE;         // by
typedef         uint16_t      WORD;         // w
typedef         int16_t       SHORT;        // sh (s is for struct)
typedef         uint32_t      DWORD;        // dw
typedef         int32_t       LONG;         // l
typedef         uint64_t      QWORD;        // qw
typedef         int64_t       LWORD;        // ql

/* Truth values */
#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef NULL
#define NULL 0L
#endif

#endif  // __dataTypes_h