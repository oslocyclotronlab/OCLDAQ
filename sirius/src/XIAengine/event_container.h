#ifndef TYPES_DEF_H
#define TYPES_DEF_H

#include <stdint.h>

#include "defines.h"


typedef struct {
    int64_t timestamp;                  //! Timestamp of the event.
    uint32_t raw_data[MAX_RAWDATA_LEN]; //! Pointer to the raw data.
    int size_raw;                       //! Size of the raw data in number of 32 bit words.
} Event_t;

inline bool operator>(const Event_t &a, const Event_t &b) { return (a.timestamp>b.timestamp); }




#endif // TYPES_DEF_H
