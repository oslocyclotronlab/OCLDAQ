#pragma once

#include <cereal/archives/portable_binary.hpp>
#include <cereal/types/vector.hpp>

#include <cstdint>

#include <string>
#include <vector>

#include <sstream>

#ifdef HAVE_WORD_T
    #include "Event.h"
    #include <cstring>
#else
typedef struct {
    uint16_t address;		//!< Holds the address of the ADC.
    uint16_t adcdata;		//!< Data read out from the ADC.
    uint16_t cfddata;       //!< Fractional difference of before/after zero-crossing.
    double cfdcorr;         //!< Correction from the CFD.
    int64_t timestamp;		//!< Timestamp in [ns].
    bool cfdfail;           //!< Flag to tell if the CFD was forced or not.
} word_t;
#endif // HAVE_WORD_T

template<class Archive>
void serialize(Archive &archive, word_t &word)
{
    archive(word.address, word.adcdata, word.cfddata, word.cfdcorr, word.timestamp, word.cfdfail);
}

std::string to_string(std::vector<word_t> &words)
{
    std::ostringstream stream;
    {
        cereal::PortableBinaryOutputArchive ar( stream );

        ar(words);

    }

    return stream.str();
}

std::vector<word_t> from_string(const std::string &str)
{
    std::vector<word_t> words;
    std::istringstream stream(str);
    {
        cereal::PortableBinaryInputArchive ar(stream);

        ar(words);
    }
    return words;
}