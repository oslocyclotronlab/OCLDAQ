/*******************************************************************************
 * Copyright (C) 2016 Vetle W. Ingeberg                                        *
 * Author: Vetle Wegner Ingeberg, v.w.ingeberg@fys.uio.no                      *
 *                                                                             *
 * --------------------------------------------------------------------------- *
 * This program is free software; you can redistribute it and/or modify it     *
 * under the terms of the GNU General Public License as published by the       *
 * Free Software Foundation; either version 3 of the license, or (at your      *
 * option) any later version.                                                  *
 *                                                                             *
 * This program is distributed in the hope that it will be useful, but         *
 * WITHOUT ANY WARRANTY; without even the implied warranty of                  *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General   *
 * Public License for more details.                                            *
 *                                                                             *
 * You should have recived a copy of the GNU General Public License along with *
 * the program. If not, see <http://www.gnu.org/licenses/>.                    *
 *                                                                             *
 *******************************************************************************/

/*!
 * \file Unpacker.cpp
 * \brief Implementation of Unpacker.
 * \author Vetle W. Ingeberg
 * \version 0.8.0
 * \date 2015-2016
 * \copyright GNU Public License v. 3
 */

#include "Unpacker.h"
#include "experimentsetup.h"
#include "XIA_CFD.h"

#include <algorithm>
#include <iostream>

#define FINISHCODE              0xC0000000  ///< Bitmask for the Finish code bit [31]
#define FINISHCODE_OFFSET       30          ///< Finish code offset
#define EVENTLENGTH             0x7FFE0000  ///< Bitmask for the Event length [30:17]
#define EVENTLENGTH_OFFSET      17          ///< Event length offset
#define HEADERSIZE              0x1F000     ///< Header size [16:12]
#define HEADERSIZE_OFFSET       12          ///< Header size offset
#define CHANNELID               0x00000FFF  ///< Channel ID [11:0]
#define CHANNELID_OFFSET        0           ///< Channel ID offset
#define EVENTTIMELOW            0xFFFFFFFF  ///< Least significant bits of the timestamp [31:0]
#define EVENTTIMELOW_OFFSET     0           ///< Least siginificant bits of the timestamp offset
#define EVENTTIMEHIGH           0x0000FFFF  ///< Most significant bits of the timestamp [15:0]
#define EVENTTIMEHIGH_OFFSET    0           ///< Most significant bits of the timestamp offset

inline bool sort_func(word_t a, word_t b){ return a.timestamp < b.timestamp; }

word_t Extract_word(const volatile uint32_t *buf, const int &size, bool &error)
{
    if ( size < 4 ){
        error = true;
        return word_t();
    }

    int current_position = 0;

    int16_t chanID = ( buf[current_position] & 0xF ) >> 0;
    int16_t slotID = ( buf[current_position] & 0xF0 ) >> 4;
    int16_t crateID = ( buf[current_position] & 0xF00 ) >> 8;
    size_t header_length = ( buf[current_position] & 0x1F000 ) >> 12;
    size_t event_length = ( buf[current_position] & 0x7FFE0000 ) >> 17;
    bool finish_code = ( buf[current_position++] & 0x80000000 ) >> 30;

    if ( size < event_length ){
        error = true;
        return word_t();
    }

    uint32_t evttime_lo = buf[current_position++];

    uint16_t evttime_hi = ( buf[current_position] & 0xFFFF ) >> 0;
    uint16_t cfddata = ( buf[current_position++] & 0xFFFF0000 ) >> 16;

    uint16_t event_energy = ( buf[current_position] & 0xFFFF ) >> 0;
    uint16_t trace_length = ( buf[current_position] & 0x7FFF0000 ) >> 16;
    bool trace_out_of_range = ( buf[current_position++] & 0x80000000 ) >> 30;

    uint32_t ext_TS_lo;
    uint16_t ext_TS_hi;
    uint32_t esum[4];
    uint32_t QDCsum[8];

    if ( header_length == 6 ){
        ext_TS_lo = buf[current_position++];
        ext_TS_hi = (buf[current_position++] & 0xFFFF);
    } else if ( header_length == 8 ){
        for (size_t i = 0 ; i < 4 ; ++i){
            esum[i] = buf[current_position++];
        }
    } else if ( header_length == 10 ){
        for (size_t i = 0 ; i < 4 ; ++i){
            esum[i] = buf[current_position++];
        }
        ext_TS_lo = buf[current_position++];
        ext_TS_hi = (buf[current_position++] & 0xFFFF);
    } else if ( header_length == 12 ){
        for (size_t i = 0 ; i < 8 ; ++i){
            QDCsum[i] = buf[current_position++];
        }
    } else if ( header_length == 14 ){
        for (size_t i = 0 ; i < 8 ; ++i){
            QDCsum[i] = buf[current_position++];
        }
        ext_TS_lo = buf[current_position++];
        ext_TS_hi = (buf[current_position++] & 0xFFFF);
    } else if ( header_length == 16 ){
        for (size_t i = 0 ; i < 4 ; ++i){
            esum[i] = buf[current_position++];
        }
        for (size_t i = 0 ; i < 8 ; ++i){
            QDCsum[i] = buf[current_position++];
        }
    } else if ( header_length == 18 ){
        for (size_t i = 0 ; i < 4 ; ++i){
            esum[i] = buf[current_position++];
        }
        for (size_t i = 0 ; i < 8 ; ++i){
            QDCsum[i] = buf[current_position++];
        }
        ext_TS_lo = buf[current_position++];
        ext_TS_hi = (buf[current_position++] & 0xFFFF);
    } else if ( header_length != 4 ){
        std::cerr << "Wrong header length = " << header_length << std::endl;
    }

    word_t result;
    result.address = ( crateID >> 8 ) | ( slotID >> 4 ) | chanID;
    result.adcdata = event_energy;
    result.cfddata = cfddata;
    result.timestamp = (int64_t(evttime_hi) << 32) | int64_t(evttime_lo);
    result.tracelen = trace_length;

    switch (GetSamplingFrequency(result.address)) {
        case f100MHz:
            result.cfdcorr = XIA_CFD_Fraction_100MHz(result.cfddata, &result.cfdfail);
            result.timestamp *= 10;
            break;
        case f250MHz:
            result.cfdcorr = XIA_CFD_Fraction_250MHz(result.cfddata, &result.cfdfail);
            result.timestamp *= 8;
            break;
        case f500MHz:
            result.cfdcorr = XIA_CFD_Fraction_500MHz(result.cfddata, &result.cfdfail);
            result.timestamp *= 10;
            break;
        default:
            result.timestamp *= 10;
            break;
    }


    for ( size_t i = 0 ; i < event_length - header_length ; ++i ){
        result.trace[2*i] = ( buf[current_position] & 0xFFFF );
        result.trace[2*i+1] = ( buf[current_position++] & 0xFFFF0000 ) >> 16;
    }
    error = false;
    return result;
}

std::vector<word_t> Unpacker::ParseBuffer(const volatile uint32_t *buffer, const int &size, bool &error)
{
    int current_position = 0;
    std::vector<word_t> found;
    found.reserve(size); // Make sure we have enough space!
    int event_length, header_length;
    int64_t event_t_low, event_t_high;
    word_t curr_w;

    // First overflow!
    if ( overflow.size() > 0 ){
        header_length = ( overflow[0] & 0x1F000 ) >> 12;
        event_length = ( overflow[0] & 0x7FFE0000 ) >> 17;

        uint32_t *tmp = new uint32_t[event_length];

        for (size_t i = 0 ; i < overflow.size() ; ++i){
            tmp[i] = overflow[i];
        }

        int diff = event_length - overflow.size();
        for (int i = 0 ; i < diff ; ++i){
            tmp[overflow.size() + i] = buffer[current_position++];
        }
        found.push_back(Extract_word(tmp, event_length, error));
        overflow.clear();
        delete[] tmp;
    }

    while ( current_position < size ){

        header_length = ( buffer[current_position] & 0x1F000 ) >> 12;
        event_length = ( buffer[current_position] & 0x7FFE0000 ) >> 17;

        if ( current_position + event_length > size ){
            while( current_position < size )
                overflow.push_back(buffer[current_position]);
            }
            break;

        found.push_back(Extract_word(buffer+current_position, event_length));
    }

    std::sort(found.begin(), found.end(), sort_func);
    error = false;
    return found;

}


#undef FINISHCODE
#undef FINISHCODE_OFFSET
#undef EVENTLENGTH
#undef EVENTLENGTH_OFFSET
#undef HEADERSIZE
#undef HEADERSIZE_OFFSET
#undef CHANNELID
#undef CHANNELID_OFFSET
#undef EVENTTIMELOW
#undef EVENTTIMELOW_OFFSET
#undef EVENTTIMEHIGH
#undef EVENTTIMEHIGH_OFFSET
