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

#define FINISHCODE              0xC0000000  ///< Bitmask for the Finish code bit [31]
#define FINISHCODE_OFFSET       30          ///< Finish code offset
#define EVENTLENGTH             0x3FFE0000  ///< Bitmask for the Event length [30:17]
#define EVENTLENGTH_OFFSET      17          ///< Event length offset
#define HEADERSIZE              0x0001F000  ///< Header size [16:12]
#define HEADERSIZE_OFFSET       12          ///< Header size offset
#define CHANNELID               0x00000FFF  ///< Channel ID [11:0]
#define CHANNELID_OFFSET        0           ///< Channel ID offset
#define EVENTTIMELOW            0xFFFFFFFF  ///< Least significant bits of the timestamp [31:0]
#define EVENTTIMELOW_OFFSET     0           ///< Least siginificant bits of the timestamp offset
#define EVENTTIMEHIGH           0x0000FFFF  ///< Most significant bits of the timestamp [15:0]
#define EVENTTIMEHIGH_OFFSET    0           ///< Most significant bits of the timestamp offset

inline bool sort_func(word_t a, word_t b){ return a.timestamp < b.timestamp; }

std::vector<word_t> Unpacker::ParseBuffer(const volatile uint32_t *buffer, const int &size, bool error)
{
    int current_position = 0;
    std::vector<word_t> found;
    found.reserve(size); // Make sure we have enough space!
    int event_length, header_length;
    int64_t event_t_low, event_t_high;
    word_t curr_w;
    if (overflow.size() > 0){
        event_length = (overflow[current_position] & EVENTLENGTH) >> EVENTLENGTH_OFFSET;
        header_length = (overflow[current_position] & HEADERSIZE) >> HEADERSIZE_OFFSET;
        if (event_length != header_length){ // Huston, we have a problem...
            overflow.clear();
            error=true;
            return std::vector<word_t>(0);
        }

        uint32_t *tmp = new uint32_t[event_length];

        for (size_t i = 0 ; i < overflow.size() ; ++i){
            tmp[i] = overflow[i];
        }

        // Take data from buffer
        int diff = event_length - overflow.size();
        for (int i =0 ; i < diff ; ++i){
            tmp[i+overflow.size()] = buffer[i];
        }

        curr_w.address = (tmp[0] & CHANNELID) >> CHANNELID_OFFSET;
        event_t_low = (tmp[1] & EVENTTIMELOW) >>  EVENTTIMELOW_OFFSET;
        event_t_high = (tmp[2] & EVENTTIMEHIGH) >> EVENTTIMEHIGH_OFFSET;
        curr_w.cfddata = (tmp[2] & 0xFFFF0000) >> 16;
        curr_w.adcdata = (tmp[3] & 0xFFFF) >> 0;
        curr_w.timestamp = event_t_high << 32;
        curr_w.timestamp |= event_t_low;



        switch (GetSamplingFrequency(curr_w.address)) {
        case f100MHz:
            curr_w.cfdcorr = XIA_CFD_Fraction_100MHz(curr_w.cfddata, &curr_w.cfdfail);
            curr_w.timestamp *= 10;
            break;
        case f250MHz:
            curr_w.cfdcorr = XIA_CFD_Fraction_250MHz(curr_w.cfddata, &curr_w.cfdfail);
            curr_w.timestamp *= 8;
            break;
        case f500MHz:
            curr_w.cfdcorr = XIA_CFD_Fraction_500MHz(curr_w.cfddata, &curr_w.cfdfail);
            curr_w.timestamp *= 10;
            break;
        default:
            curr_w.timestamp *= 10;
            error = true;
            break;
        }

        overflow.clear();
        found.push_back(curr_w);
        delete[] tmp;

    }



    while (current_position < size){

        event_length = (buffer[current_position] & EVENTLENGTH) >> EVENTLENGTH_OFFSET;
        header_length = (buffer[current_position] & HEADERSIZE) >> HEADERSIZE_OFFSET;

        if (event_length != header_length){ // Huston, we have a problem...
            overflow.clear();
            error=true;
            return std::vector<word_t>(0);
        }

        if ( !(event_length+current_position < size) ){
            for (int i = current_position ; i < size ; ++i){
                int tmp = buffer[i];
                overflow.push_back(tmp);
            }
            current_position = size;
            break;
        }

        curr_w.address = (buffer[current_position] & CHANNELID) >> CHANNELID_OFFSET;
        event_t_low = (buffer[current_position+1] & EVENTTIMELOW) >>  EVENTTIMELOW_OFFSET;
        event_t_high = (buffer[current_position+2] & EVENTTIMEHIGH) >> EVENTTIMEHIGH_OFFSET;
        curr_w.cfddata = (buffer[current_position+2] & 0xFFFF0000) >> 16;
        curr_w.adcdata = (buffer[current_position+3] & 0xFFFF) >> 0;
        curr_w.timestamp = event_t_high << 32;
        curr_w.timestamp |= event_t_low;



        switch (GetSamplingFrequency(curr_w.address)) {
        case f100MHz:
            curr_w.cfdcorr = XIA_CFD_Fraction_100MHz(curr_w.cfddata, &curr_w.cfdfail);
            curr_w.timestamp *= 10;
            break;
        case f250MHz:
            curr_w.cfdcorr = XIA_CFD_Fraction_250MHz(curr_w.cfddata, &curr_w.cfdfail);
            curr_w.timestamp *= 8;
            break;
        case f500MHz:
            curr_w.cfdcorr = XIA_CFD_Fraction_500MHz(curr_w.cfddata, &curr_w.cfdfail);
            curr_w.timestamp *= 10;
            break;
        default:
            curr_w.timestamp *= 10;
            error = true;
            break;
        }

        found.push_back(curr_w);
        current_position += event_length;
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
