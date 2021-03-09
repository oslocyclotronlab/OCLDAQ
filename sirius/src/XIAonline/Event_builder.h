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

#ifndef EVENT_BUILDER_H
#define EVENT_BUILDER_H

#include <vector>
#include <cstddef>

#include "Event.h"

/*!
 * \class Unpacker
 * \brief Determines events from a buffer of data words.
 * \details This class recives a buffer of data words and creates events from some spesific requirement specified by the implementation.
 * \author Vetle W. Ingeberg
 * \version 0.8.0
 * \date 2015-2016
 * \copyright GNU Public License v. 3
 * \todo The process of determining an event should be more user customizable, say through a batch file or similar.
 */
class EventBuilder {
public:
	typedef enum {
		OKAY,		//!< Event unpacked without problems.
		END,		//!< End of buffer reached, no event unpacked.
		ERROR		//!< Error while unpacking event.
	} Status;

	//! Initilize the unpacker.
    EventBuilder();

	//! Set the buffer from which the events shall be extracted.
	/*! This also resets all counters.
	 */
    void SetBuffer(std::vector<word_t> buffr /*!< The buffer to extract from. */);

	//! Unpack the next event.
	/*! \return the status after unpacking.
	 */
    Status Next(Event& event /*!< The event structure to unpack into. */);

	//! Retrive the average event length seen so far.
	/*! \return The average event length.
	 */
	float GetAverageLength() const
		{ return event_count > 0 ? eventlength_sum/float(event_count) : 0; }

private:
	//! Actual implementation of the unpacking.
    /*! Iterates the buffer until it finds a data word from a strip in the back detector in one of the
     *  particle telescopes. It then calls ExtractEvent which fills the event structure based on its implementation.
     *  \return True if event identified, false if no event is found (or the end of the buffer is reached).
	 */
    bool UnpackOneEvent(Event& event /*!< The event structure to fill.  */);


    //! Pack the data into the event structure.
    bool PackEvent(Event& event, int start, int stop);

	//! The buffer to read from.
    std::vector<word_t> buffer;

    //! Current position in the buffer.
    size_t curr_pos;

    //! Sum of the event length extracted.
	int eventlength_sum;

    //! Number of events extracted.
	int event_count;
};

#endif // EVENT_BUILDER_H
