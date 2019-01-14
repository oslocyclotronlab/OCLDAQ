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
 * \file XIARoutine.cpp
 * \brief Implementation of XIARoutine.
 * \author Vetle W. Ingeberg
 * \version 0.8.0
 * \date 2018
 * \copyright GNU Public License v. 3
 */


#include "XIARoutine.h"

#include "Event.h"
#include "Histogram1D.h"
#include "Histogram2D.h"

#include <sstream>
#include <iostream>


XIARoutine::XIARoutine()
{ }

bool XIARoutine::Start()
{
    CreateSpectra();
	return true;
}

bool XIARoutine::End()
{
	return true;
}

bool XIARoutine::Command(const std::string& cmd)
{
    std::istringstream icmd(cmd.c_str());

    std::string name;

    icmd >> name;

    if (name == "range" || name == "RANGE" ) {
        std::string fname;
        icmd >> fname;
        range.Read(fname);
        return true;
    } else {
        //std::cerr << "TDRRoutine: Unknown command '" << cmd << "'\n";
        return UserCommand(cmd);
    }

    return false;
}


