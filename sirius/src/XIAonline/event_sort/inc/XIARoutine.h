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

#ifndef TDRROUTINE_H
#define TDRROUTINE_H

#include "UserRoutine.h"
#include "ParticleRange.h"


#include <string>
#include <cstdint>
#include <cstdlib>


/*!
 * \class TDRRoutine
 * \brief Interface for sorting routines for inverse kinematics.
 * \details This interface defines the basic functionality that is needed for all sorting routines for inverse kinematics experiments recorded
 * in the TDR binary file format. The user need to implement a class "Telescope" that describes the experimental setup of the telescopes.
 * \author Vetle W. Ingeberg
 * \version 0.8.0
 * \date 2015-2016
 * \copyright GNU Public License v. 3
 * \todo Implement support for ROOT trees.
 */
class XIARoutine : public UserRoutine {
public:

    //! Constructor.
    XIARoutine();

    //! Start of sorting session.
    /*! \return true if successful, false otherwise.
     */
	bool Start();

    //! Pass a command to the sorting routine.
    /*! \return true if successful, false otherwise.
     */
    bool Command(const std::string& cmd /*!< String containing the command to be executed. */);

    //! User class commands.
    /*! \return true if successful, false otherwise.
     */
    virtual bool UserCommand(const std::string& cmd)=0;

    //! Signal to end the sorting session.
    /*! \return true if successful, false otherwise.
     */
	bool End();

protected:

    //! Range curve
    ParticleRange range;

    //! Create all spectra.
    /*! This method must be implemented in a class deriving from TDRRoutine.
     */
    virtual void CreateSpectra() = 0;


	//! Create a 1D histogram.
    /*! \return a pointer to a new 1D histogram.
     */
	Histogram1Dp Spec( const std::string& name,		/*!< The name of the new histogram.		*/
					   const std::string& title,	/*!< The title of the new histogram.	*/
					   int channels,				/*!< The number of regular bins.		*/
					   Axis::bin_t left,			/*!< The lower edge of the lowest bin.	*/
					   Axis::bin_t right,			/*!< The upper edge of the highest bin.	*/
                       const std::string& xtitle	/*!< The title of the x axis.			*/)
    {
        return GetHistograms().Create1D(name, title, channels, left, right, xtitle);
    }

	//! Create a 2D histogram.
    /*! \return a pointer to a new 2D histogram.
     */
	Histogram2Dp Mat( const std::string& name,		/*!< The name of the new histogram.						*/
					  const std::string& title,		/*!< The title of the new histogram.					*/
					  int ch1,						/*!< The number of regular bins of the x axis.			*/
                      Axis::bin_t l1,				/*!< The lower edge of the lowest bin on the x axis.	*/
                      Axis::bin_t r1,				/*!< The upper edge of the highest bin on the x axis.	*/
					  const std::string& xtitle,	/*!< The title of the x axis.							*/
					  int ch2,						/*!< The number of regular bins of the y axis.			*/
                      Axis::bin_t l2,				/*!< The lower edge of the lowest bin on the y axis.	*/
                      Axis::bin_t r2,				/*!< The upper edge of the highest bin on the y axis.	*/
                      const std::string& ytitle		/*!< The title of the y axis.							*/)
    {
        return GetHistograms().Create2D(name, title, ch1, l1, r1, xtitle, ch2, l2, r2, ytitle);
    }
};

#endif // TDRROUTINE_H
