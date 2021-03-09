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

#include "Event_builder.h"
#include "Event.h"

#include "experimentsetup.h"

#include <iostream>

#define GAP_SIZE 500
#define MAX_TDIFF 1600

#define SINGLES 0

EventBuilder::EventBuilder()
	: buffer( 0 )
    , curr_pos( 0 )
	, eventlength_sum( 0 )
	, event_count ( 0 )
{
}

void EventBuilder::SetBuffer(std::vector<word_t> buffr)
{
    buffer.clear();
    for (size_t i = 0 ; i < buffr.size() ; ++i)
        buffer.push_back(buffr[i]);

    curr_pos = 0; // Reset buffer position.

    // Resting the length of the buffer.
	event_count = eventlength_sum = 0;
}

EventBuilder::Status EventBuilder::Next(Event &event)
{
    if ( curr_pos >= buffer.size() )
        return END; // End of buffer reached.

	int n_data = 0;
    if ( !UnpackOneEvent(event) ){
        curr_pos = n_data + 1;
        return END; // If no event found, end of buffer.
	}

    eventlength_sum += event.length;
	event_count += 1;

	return OKAY;
}

#if SINGLES
bool EventBuilder::UnpackOneEvent(Event &event)
{
    event.Reset();

    if ( curr_pos >= buffer.size() )
        return false;

    int64_t timediff;
    int start = curr_pos;
    int stop = curr_pos+1;
    for (size_t i = curr_pos + 1 ; i < buffer.size() ; ++i){
        timediff = buffer[i].timestamp - buffer[i-1].timestamp;
        if (timediff > GAP_SIZE){
            stop = i;
            curr_pos = stop;
            return PackEvent(event, start, stop);
        }
    }

    stop = buffer.size();
    curr_pos = stop;
    return PackEvent(event, start, stop);
}

#else

bool EventBuilder::UnpackOneEvent(Event &event)
{
    event.Reset();
    int factor = 10;
    if ( curr_pos >= buffer.size() )
        return false;

    int64_t timediff;
    int start = curr_pos;
    int stop = curr_pos + 1;
    word_t curr_w;
    for (size_t i = curr_pos ; i < buffer.size() ; ++i){
        curr_w = buffer[i];
        if ( ( GetDetector(curr_w.address).type == eDet ) ){

            // We find all events that are within MAX_TDIFF before the
            // E event.
            for (int j = curr_pos ; j > 0 ; --j){

                timediff = buffer[j-1].timestamp - curr_w.timestamp;


                if (std::abs(timediff) > 0/*MAX_TDIFF*/){
                    start = j;
                    break;
                }
            }

            // We find all events that are withn MAX_TDIFF after the
            // E event
            for (int j = curr_pos ; j < buffer.size() - 1 ; ++j){
                timediff = buffer[j+1].timestamp - curr_w.timestamp;
                if (std::abs(timediff) > MAX_TDIFF){
                    stop = j+1;
                    break;
                }
            }
            curr_pos = i+1;
            event.trigger = curr_w;
            return PackEvent(event, start, stop);
        }
    }
    return false;
}

#endif // SINGLES

bool EventBuilder::PackEvent(Event& event, int start, int stop)
{
    event.length = stop - start;
    DetectorInfo_t dinfo;
    for (int i = start ; i < stop ; ++i){
        dinfo = GetDetector(buffer[i].address);

        switch (dinfo.type) {
        case labr: {
            if ( event.n_labr[dinfo.detectorNum] < MAX_WORDS_PER_DET &&
                 dinfo.detectorNum < NUM_LABR_DETECTORS){
                event.w_labr[dinfo.detectorNum][event.n_labr[dinfo.detectorNum]++] = buffer[i];
                ++event.tot_labr;
            } else {
                std::cerr << __PRETTY_FUNCTION__ << ": Could not populate LaBr word, run debugger with appropriate break point for more details" << std::endl;
            }
            break;
        }
        /*case deDet: {
            if ( event.n_dEdet[dinfo.detectorNum] < MAX_WORDS_PER_DET &&
                 dinfo.detectorNum < NUM_SI_DE_DET){
                event.w_dEdet[dinfo.detectorNum][event.n_dEdet[dinfo.detectorNum]++] = buffer[i];
                ++event.tot_dEdet;
            } else {
                std::cerr << __PRETTY_FUNCTION__ << ": Could not populate dEdet word, run debugger with appropriate break point for more details" << std::endl;
            }
            break;
        } */
        case deDet: {
            if ( event.n_dEdet[dinfo.telNum][dinfo.detectorNum] < MAX_WORDS_PER_DET /*&&
                 dinfo.detectorNum < NUM_SI_DE_TEL*/){
                event.w_dEdet[dinfo.telNum][dinfo.detectorNum][event.n_dEdet[dinfo.telNum][dinfo.detectorNum]++] = buffer[i];
                ++event.tot_dEdet;
                ++event.tot_dEdet_trap[dinfo.telNum];
            } else {
                std::cerr << __PRETTY_FUNCTION__ << ": Could not populate dEdet word, run debugger with appropriate break point for more details" << std::endl;
            }
            break;
        }
        case eDet: {
            if ( event.n_Edet[dinfo.detectorNum] < MAX_WORDS_PER_DET &&
                 dinfo.detectorNum < NUM_SI_E_DET){
                event.w_Edet[dinfo.detectorNum][event.n_Edet[dinfo.detectorNum]++] = buffer[i];
                ++event.tot_Edet;
            } else {
                std::cerr << __PRETTY_FUNCTION__ << ": Could not populate eDet word, run debugger with appropriate break point for more details" << std::endl;
            }
            break;
        }
        case eGuard: {
            if ( event.n_Eguard[dinfo.detectorNum] < MAX_WORDS_PER_DET &&
                 dinfo.detectorNum < NUM_SI_E_GUARD){
                event.w_Eguard[dinfo.detectorNum][event.n_Eguard[dinfo.detectorNum]++] = buffer[i];
                ++event.tot_Eguard;
            } else {
                std::cerr << __PRETTY_FUNCTION__ << ": Could not populate eGuard word, run debugger with appropriate break point for more details" << std::endl;
            }
            break;
        }
        case rfchan: {
            if ( event.n_RFpulse < MAX_WORDS_PER_DET )
                event.w_RFpulse[event.n_RFpulse++] = buffer[i];
            else
                std::cerr << __PRETTY_FUNCTION__ << ": Could not populate rfchan word, run debugger with appropriate break point for more details" << std::endl;
            break;
        }
        case ppac: {
            if ( event.n_PPAC[dinfo.detectorNum] < MAX_WORDS_PER_DET &&
                 dinfo.detectorNum < NUM_PPAC){
                event.w_PPAC[dinfo.detectorNum][event.n_PPAC[dinfo.detectorNum]++] = buffer[i];
                ++event.tot_PPAC;
            } else {
                std::cerr << __PRETTY_FUNCTION__ << ": Could not populate ppac word, run debugger with appropriate break point for more details" << std::endl;
            }
            break;
        }
        default:
            break;
        }
    }
    return true;
}
