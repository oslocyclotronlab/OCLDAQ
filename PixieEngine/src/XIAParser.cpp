//
// Created by Vetle Wegner Ingeberg on 11/02/2021.
//

#include "XIAParser.h"

#include <cstring>

struct XIA_base_t {
    unsigned chanID : 4;
    unsigned slotID : 4;
    unsigned crateID : 4;
    unsigned headerLen : 5;
    unsigned eventLen : 14;
    bool finishCode : 1;
    unsigned event_ts_lo : 32;
    unsigned event_ts_hi : 16;
    unsigned cfd_result : 16;
    unsigned event_energy : 16;
    unsigned traceLen : 15;
    bool traceOutOfRange : 1;
};

std::vector<XIAEvent> XIAParser::Parse(uint32_t *lmdata, const size_t &size, size_t &buf_size) const
{
    std::vector<XIAEvent> events;
    auto pos = reinterpret_cast<const char *>(lmdata);
    auto end = pos + size*sizeof(uint32_t);

    const XIA_base_t *hdr;
    XIAEvent event;

    buf_size = 0;
    while ( pos < end ){
        hdr = reinterpret_cast<const XIA_base_t *>(pos);
        if ( pos + hdr->eventLen >= end ){
            buf_size = (end - pos) / sizeof( uint32_t ); // Buf size in 32-bit words
            memcpy(lmdata, pos, end-pos);
            break;
        }

        event.timestamp = hdr->event_ts_hi;
        event.timestamp <<= 28;
        event.timestamp |= hdr->event_ts_lo;
        event.timestamp *= ts;
        event.size_raw = hdr->headerLen;
        memcpy(reinterpret_cast<void *>(event.data), pos, hdr->headerLen * sizeof(uint32_t));
        reinterpret_cast<XIA_base_t *>(event.data)->eventLen = hdr->headerLen;
        reinterpret_cast<XIA_base_t *>(event.data)->traceLen = 0;
        events.push_back(event);
        pos += hdr->eventLen;
    }
    return events;
}