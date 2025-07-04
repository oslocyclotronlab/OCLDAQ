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
    int16_t address = ( buf[current_position] & 0xFFF );
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
        error = true;
        //return word_t();
    }

    word_t result;
    result.address = address;
    result.adcdata = event_energy;
    result.cfddata = cfddata;
    result.timestamp = (int64_t(evttime_hi) << 32) | int64_t(evttime_lo);
    //result.trace = trace_t(trace_length);

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
            result.cfdcorr = 0;
            result.timestamp *= 10;
            break;
    }

    while ( current_position < size )
        current_position++;
    return result;

    /*for ( size_t i = 0 ; i < event_length - header_length ; ++i ){
        result.trace.trace[2*i] = ( buf[current_position] & 0xFFFF );
        result.trace.trace[2*i+1] = ( buf[current_position++] & 0xFFFF0000 ) >> 16;
        current_position++;
    }
    error = false;
    return result;*/
}

std::vector<word_t> Unpacker::ParseBuffer(const volatile uint32_t *buffer, const int &size, bool &error)
{
    int current_position = 0;
    std::vector<word_t> found;
    found.reserve(size); // Make sure we have enough space!
    int event_length, header_length;
    int64_t event_t_low, event_t_high;
    word_t curr_w;


    while ( current_position < size ){

        header_length = ( buffer[current_position] & 0x1F000 ) >> 12;
        event_length = ( buffer[current_position] & 0x7FFE0000 ) >> 17;
        //std::cout << current_position << " " << header_length << " " << event_length << std::endl;
        if ( current_position + event_length > size )
            break;

        found.push_back(Extract_word(buffer+current_position, event_length, error));
        current_position += event_length;
        if ( error )
            break;
    }

    std::sort(found.begin(), found.end(), sort_func);
    return found;
}



