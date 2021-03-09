//
// Created by Vetle Wegner Ingeberg on 11/02/2021.
//

#ifndef XIAPARSER_H
#define XIAPARSER_H

#include <vector>

#define XIA_HEADER_MAX_LEN 18
struct XIAEvent {
    long long timestamp;    //! Event timestamp in [ns]
    int size_raw;
    uint32_t data[XIA_HEADER_MAX_LEN];
};

class XIAParser
{
private:
    long long ts;

public:

    explicit XIAParser(const long long &ts_factor) : ts( ts_factor ){}

    std::vector<XIAEvent> Parse(uint32_t *lmdata, const size_t &size, size_t &buf_size) const;


};

#endif // XIAPARSER_H
