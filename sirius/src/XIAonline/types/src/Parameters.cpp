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

/*#include "Parameters.h"

#include <fstream>

Parameters::Parameters(){ }


Parameters::~Parameters(){ }

bool Parameters::next_line(std::istream &in, std::string &cmd_line)
{
    cmd_line = "";
    std::string line;
    while ( getline(in, line) ){
        size_t ls = line.size();
        if (ls == 0)
            break;
        else if ( line[ls-1] != '\\' ){
            cmd_line += line;
            break;
        } else
            cmd_line += line.substr(0, ls-1);
    }
    return in || !cmd_line.empty();
}*/

#include "Parameters.h"

#include <sstream>
#include <iostream>
#include <iterator>
#include <algorithm>


// ########################################################################

Parameter* Parameters::Find(const std::string& name)
{
    names_t::iterator it = names.find(name);
    if( it != names.end() )
        return it->second;
    else
        return 0;
}

// ########################################################################

void Parameters::Add(const std::string& name, Parameter* param)
{
#if 0
    names_t::iterator it = names.find(name);
    if( it != names.end() )
        ; // TODO warning/exception;
#endif
    names[name] = param;
}

// ########################################################################

void Parameters::Remove(const std::string& name)
{
    names_t::iterator it = names.find(name);
    if( it != names.end() )
        names.erase( it );
}

// ########################################################################

void Parameters::Remove(Parameter* param)
{
    for( names_t::iterator it = names.begin(); it != names.end(); ++it ) {
        if( it->second == param ) {
            names.erase( it );
            break;
        }
    }
}

// ########################################################################

bool Parameters::SetAll(std::istringstream& icmd)
{
    // format parameter (name = number+;)+
    std::string tmp;
    while( getline(icmd, tmp, ';') ) {
        // search for non-whitespace, that should be the start of the
        // parameter name
        size_t start_name = tmp.find_first_not_of(" \t");
        if( start_name == std::string::npos )
            return false;

        // search for whitespace or '=' after the name start; that
        // should be the end of the parameter name
        size_t end_name = tmp.find_first_of(" \t=", start_name);
        if( end_name == std::string::npos )
            return false;

        std::string par_name = tmp.substr(start_name, end_name-start_name);

        // search for '=' after the name end
        size_t pos_equal = tmp.find("=", end_name);
        if( pos_equal == std::string::npos )
            return false;

        Parameter* param = Find(par_name);
        if( param ) {
            param->Set(tmp.substr(pos_equal+1));
        } else {
            std::cerr << "Unknown parameter '" << par_name << "'" << std::endl;
            return false;
        }
    }
    return true;
}

// ########################################################################
// ########################################################################

Parameter::Parameter(Parameters& prmtrs, const std::string& nm, int std_sz, param_t dflt_value)
    : parameters( prmtrs )
    , name( nm )
    , std_size( std_sz )
{
    //DBGL;
    parameters.Add( name, this );
    if( std_sz>0 )
        values = std::vector<param_t>(std_size, dflt_value);
    //DBGL;
}

// ########################################################################

Parameter::~Parameter()
{
    parameters.Remove( this );
}

// ########################################################################

void Parameter::Set(const std::vector<param_t>& nvalues)
{
    values = nvalues;

    const unsigned int MAXPRINT = 4; // do not show more than MAXPRINT values
    std::cout << "Parameter '" << name << "':";
    for(unsigned int i=0; i<GetSize() && i<MAXPRINT; ++i)
        std::cout << ' ' << Get(i);
    if( GetSize()>MAXPRINT )
        std::cout << " ...";
    std::cout << " (" << GetSize() << " values)" << std::endl;
    if( std_size>0 && GetSize() != GetStandardSize() ) {
        std::cout << "  Warning: parameter should have size "
                  << GetStandardSize() << "." << std::endl;
    }
}

// ########################################################################

void Parameter::Set(const std::string& values)
{
    std::istringstream ipar(values.c_str());
    std::vector<float> par_values;
    std::copy(std::istream_iterator<float>(ipar), std::istream_iterator<float>(),
              std::back_insert_iterator<std::vector<float> >(par_values));

    Set(par_values);
}

// ########################################################################

Parameter::param_t Parameter::Poly(param_t E) const
{
    float r = values[0], x = E;
    for(unsigned int i=1; i<GetSize(); ++i) {
        r += values[i]*x;
        x *= E;
    }
    return r;
}

// ########################################################################

Parameter::param_t Parameter::Poly(param_t E, unsigned int startidx, unsigned int count) const
{
    float r = values[startidx], x = E;
    for(unsigned int i=startidx+1; i<startidx+count; ++i) {
        r += values[i]*x;
        x *= E;
    }
    return r;
}

// ########################################################################
// ########################################################################

#ifdef TEST_PARAMETERS
int main(int argc, char* argv[])
{
    Parameters p;
    std::istringstream icmd( argv[1] );
    if( !p.Set(icmd) ) {
        std::cerr << "Error parsing '" << argv[1] << "'" << std::endl;
        return -1;
    }

    for(int i=2; i<argc; ++i) {
        Parameters::Parameter pp = p.Get(argv[i]);
        std::cout << "param '" << argv[i] << "' has " << pp.size() << " values." << std::endl;
    }
    return 0;
}
#endif
