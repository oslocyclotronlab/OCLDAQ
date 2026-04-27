//
// Created by Vetle Wegner Ingeberg on 06/04/2022.
//

#ifndef ENTRY_H
#define ENTRY_H

#include "DetectorTypes.h"

#include <cstdint>
#include <vector>

struct Entry_t {

    DetectorType type;
    unsigned short detectorID; // I.e. detector number.

    unsigned short adcvalue;
    unsigned short cfdvalue;
    long long timestamp;
    bool finishflag; // Pile-up flag

    // Derived values
    double energy;
    double cfdcorr;
    bool cfdfail;

    // QDC values
    std::array<uint32_t, 8> qdc{0};

    // Traces
    std::vector<uint16_t> trace{0};
};

#endif // ENTRY_H
