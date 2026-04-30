#include "../../include/XIAReader/Format/XIA_CFD.h"

#include <stdexcept>

#include <logfault/logfault.h>

using namespace XIA;

struct XIA_CFD_base_t {
    virtual explicit operator bool() const noexcept = 0;
    virtual explicit operator double() const noexcept = 0;
};

struct XIA_CFD_100MHz_t {
    unsigned short timecfd : 15;
    bool fail : 1;

    explicit operator bool() const noexcept { return fail; }
    explicit operator double() const noexcept { return (fail) ? 0 : 10 * double(timecfd) / 32768.0; }
    double get_time() const noexcept { return ( fail ) ? 0 : 10 * double(timecfd) / 32768.0; }
    bool get_fail() const noexcept { return fail; }
};

struct XIA_CFD_250MHz_t {
    unsigned short timecfd : 14;
    unsigned short trigsource : 1;
    bool fail : 1;

    explicit operator bool() const noexcept { return fail; }
    explicit operator double() const { return (fail) ? 0 : 4 * (double(timecfd) / 16384.0 - trigsource); }

    double get_time() const noexcept { return (fail) ? 0 : 4 * (double(timecfd) / 16384.0 - trigsource); }
    bool get_fail() const noexcept { return fail; }
};

struct XIA_CFD_500MHz_t {
    unsigned short timecfd : 13;
    unsigned short trigsource : 3;

    inline explicit operator bool() const { return (trigsource >= 7); }
    inline explicit operator double() const {
        return (trigsource >= 7) ? 0 : 2 * ( timecfd/8192.0 + trigsource - 1.);
    }

    double get_time() const noexcept { return (trigsource >= 7) ? 0 : 2 * (double(timecfd) / 8192.0 + trigsource - 1.); }
    bool get_fail() const noexcept { return (trigsource >= 7); }
};

double XIA::XIA_CFD_Fraction_100MHz(const uint16_t &CFDvalue, bool &fail) {
  auto *cfd = reinterpret_cast<const XIA_CFD_100MHz_t *>(&CFDvalue);
  fail = cfd->fail;
  return double(*cfd);
}



double XIA::XIA_CFD_Fraction_250MHz(const uint16_t &CFDvalue, bool &fail) {
  auto *cfd = reinterpret_cast<const XIA_CFD_250MHz_t *>(&CFDvalue);
  fail = cfd->fail;
  return double(*cfd);
}


double XIA::XIA_CFD_Fraction_500MHz(const uint16_t &CFDvalue, bool &fail) {
  auto *cfd = reinterpret_cast<const XIA_CFD_500MHz_t *>(&CFDvalue);
  fail = bool(*cfd);
  return double(*cfd);
}

XIA::XIA_CFD_t XIA::XIA_CFD_Decode(const ADCSamplingFreq& frequency, const uint16_t& CFDvalue)
{
    switch ( frequency ) {
        case ADCSamplingFreq::f100MHz : {
            auto *cfd = reinterpret_cast<const XIA_CFD_100MHz_t *>(&CFDvalue);
            return std::make_pair(cfd->get_time(), cfd->get_fail());
        }
        case ADCSamplingFreq::f250MHz : {
            auto *cfd = reinterpret_cast<const XIA_CFD_250MHz_t *>(&CFDvalue);
            return std::make_pair(cfd->get_time(), cfd->get_fail());
        }
        case ADCSamplingFreq::f500MHz : {
            auto *cfd = reinterpret_cast<const XIA_CFD_500MHz_t *>(&CFDvalue);
            return std::make_pair(cfd->get_time(), cfd->get_fail());
        }
        default :
            throw std::invalid_argument("Unknown frequency");
    }
}