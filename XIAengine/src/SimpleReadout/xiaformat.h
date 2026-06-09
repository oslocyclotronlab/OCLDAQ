//
// Created by Vetle Wegner Ingeberg on 06/04/2022.
//

#ifndef XIAFORMAT_H
#define XIAFORMAT_H
#include <array>

struct XIA_event_4_t;
struct XIA_event_6_t;
struct XIA_event_8_t;
struct XIA_event_10_t;
struct XIA_event_12_t;
struct XIA_event_14_t;
struct XIA_event_16_t;
struct XIA_event_18_t;

struct XIA_address_t {
    // First 32-bit word
    unsigned short chanID : 4;
    unsigned short slotID : 4;
    unsigned short crateID : 4;

    operator unsigned short(){ return *reinterpret_cast<unsigned short *>(this); }
};

struct XIA_base_t {
    // First 32-bit word
    unsigned chanID : 4;
    unsigned slotID : 4;
    unsigned crateID : 4;

    unsigned headerLen : 5;
    unsigned eventLen : 14;
    bool finishCode : 1;

    // Second 32-bit word
    unsigned event_time_low : 32;

    // Third 32-bit word
    unsigned event_time_high : 16;
    unsigned cfd_result : 16;

    // Forth 32-bit word
    unsigned eventEnergy : 16;
    unsigned traceLen : 15;
    bool traceOutOfRange : 1;

    [[nodiscard]] inline unsigned short index() const { return (crateID << 8) + (slotID << 4) + chanID; }
    [[nodiscard]] inline int64_t timestamp() const {
        int64_t timestamp = event_time_high;
        timestamp <<= 32;
        timestamp |= event_time_low;
        return timestamp;
    }
};

struct XIA_event_4_t : public XIA_base_t {
    uint16_t trace[];
};

struct XIA_event_6_t : public XIA_base_t {
    unsigned timestamp_low : 32;
    unsigned timestamp_high : 16;
    unsigned unused : 16;
    uint16_t trace[];
};

struct XIA_event_8_t : public XIA_base_t {
    unsigned trailing : 32;
    unsigned leading : 32;
    unsigned gap : 32;
    unsigned baseline : 32;
    uint16_t trace[];
};

struct XIA_event_10_t : public XIA_base_t {
    unsigned trailing : 32;
    unsigned leading : 32;
    unsigned gap : 32;
    unsigned baseline : 32;
    unsigned timestamp_low : 32;
    unsigned timestamp_high : 16;
    unsigned unused : 16;
    uint16_t trace[];
};

struct XIA_event_12_t : public XIA_base_t {
    unsigned qdc[8];
    uint16_t trace[];
};

struct XIA_event_14_t : public XIA_base_t {
    unsigned qdc[8];
    unsigned timestamp_low : 32;
    unsigned timestamp_high : 16;
    unsigned unused : 16;
    uint16_t trace[];
};

struct XIA_event_16_t : public XIA_base_t {
    unsigned trailing : 32;
    unsigned leading : 32;
    unsigned gap : 32;
    unsigned baseline : 32;
    unsigned qdc[8];
    uint16_t trace[];
};

struct XIA_event_18_t : public XIA_base_t {
    unsigned trailing : 32;
    unsigned leading : 32;
    unsigned gap : 32;
    unsigned baseline : 32;
    unsigned qdc[8];
    unsigned timestamp_low : 32;
    unsigned timestamp_high : 16;
    unsigned unused : 16;
    uint16_t trace[];
};

/*inline std::vector<uint32_t> getQDC(const XIA_base_t *event) {
    switch ( event->headerLen ) {
        case 12 :
            return {reinterpret_cast<const XIA_event_12_t *>(event)->qdc,
                reinterpret_cast<const XIA_event_12_t *>(event)->qdc+8};
        case 14 :
            return {reinterpret_cast<const XIA_event_14_t *>(event)->qdc,
                reinterpret_cast<const XIA_event_14_t *>(event)->qdc+8};
        case 16 :
            return {reinterpret_cast<const XIA_event_16_t *>(event)->qdc,
               reinterpret_cast<const XIA_event_16_t *>(event)->qdc+8};
        case 18 :
            return {reinterpret_cast<const XIA_event_18_t *>(event)->qdc,
               reinterpret_cast<const XIA_event_18_t *>(event)->qdc+8};
        default:
            return std::vector<uint32_t>(0);
    }
}*/

inline std::array<uint32_t, 8> getQDC(const XIA_base_t *event) {
    const uint32_t* qdc = nullptr;
    switch ( event->headerLen ) {
        case 12 :
            qdc = reinterpret_cast<const XIA_event_12_t *>(event)->qdc;
            break;
        case 14 :
            qdc = reinterpret_cast<const XIA_event_14_t *>(event)->qdc;
            break;
        case 16 :
            qdc = reinterpret_cast<const XIA_event_16_t *>(event)->qdc;
            break;
        case 18 :
            qdc = reinterpret_cast<const XIA_event_18_t *>(event)->qdc;
            break;
        default:
            return {};
    }
    return {qdc[0], qdc[1], qdc[2], qdc[3], qdc[4], qdc[5], qdc[6], qdc[7]};
}

inline std::vector<uint16_t> getTrace(const XIA_base_t *event) {
    switch ( event->headerLen ) {
        case 4 :
            return {reinterpret_cast<const XIA_event_4_t *>(event)->trace,
                reinterpret_cast<const XIA_event_4_t *>(event)->trace + event->traceLen};
        case 6 :
            return {reinterpret_cast<const XIA_event_6_t *>(event)->trace,
                reinterpret_cast<const XIA_event_6_t *>(event)->trace + event->traceLen};
        case 8 :
            return {reinterpret_cast<const XIA_event_8_t *>(event)->trace,
                reinterpret_cast<const XIA_event_8_t *>(event)->trace + event->traceLen};
        case 10 :
            return {reinterpret_cast<const XIA_event_10_t *>(event)->trace,
                reinterpret_cast<const XIA_event_10_t *>(event)->trace + event->traceLen};
        case 12 :
            return {reinterpret_cast<const XIA_event_12_t *>(event)->trace,
                reinterpret_cast<const XIA_event_12_t *>(event)->trace+event->traceLen};
        case 14 :
            return {reinterpret_cast<const XIA_event_14_t *>(event)->trace,
                reinterpret_cast<const XIA_event_14_t *>(event)->trace+event->traceLen};
        case 16 :
            return {reinterpret_cast<const XIA_event_16_t *>(event)->trace,
               reinterpret_cast<const XIA_event_16_t *>(event)->trace+event->traceLen};
        case 18 :
            return {reinterpret_cast<const XIA_event_18_t *>(event)->trace,
               reinterpret_cast<const XIA_event_18_t *>(event)->trace+event->traceLen};
        default:
            return std::vector<uint16_t>(0);
    }
}

struct XIA_esums_t {
    unsigned trailing : 32;
    unsigned leading : 32;
    unsigned gap : 32;
    unsigned baseline : 32;
};

struct XIA_qdcsums_t {
    unsigned qdc[8];
};

struct XIA_external_timestamp_t {
    unsigned timestamp_low : 32;
    unsigned timestamp_high : 16;
    unsigned unused : 16;
};


#endif // XIAFORMAT_H
