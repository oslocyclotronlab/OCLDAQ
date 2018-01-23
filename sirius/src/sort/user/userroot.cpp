
#include "userroot.h"

#include "sort_calib.h"

#include <TFile.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <iterator>
#include <algorithm>

UserROOT::UserROOT()
    : time_start(0)
    , time_now(0)
    , have_gain(false)
{
}

// ########################################################################

bool UserROOT::Init(bool)
{
    return true;
}

// ########################################################################

void UserROOT::Write()
{
    if( outfile ) {
        outfile->Write();
        outfile->Close();
        delete outfile;
    }
}

// ########################################################################

bool UserROOT::Data(const std::string& /*filename*/)
{
    if( !outfile )
        return false;

    if( !have_gain ) {
        std::cerr << "data: no gain/shift." << std::endl;
	reset_gainshifts(GetCalibration());
        //return false;
    }
    return true;
}

// ########################################################################

bool UserROOT::ReadNaIEnergyCalibration(istream& in)
{
    calibration_t& cal = GetCalibration();
    for(int i=0; i<28; ++i)
        in >> cal.shiftna[i] >> cal.gainna[i];
    return in.good();
}

// ########################################################################

bool UserROOT::Cmd(const std::string& cmd)
{
    std::istringstream icmd(cmd.c_str());

    std::string name, tmp;
    icmd >> name;

    if( name == "dump" ) {
        outfile->Write();
    } else if( name == "outfile" ) {
        Write();

        icmd >> tmp;
        outfile = new TFile(tmp.c_str(), "recreate");
        std::cout << "ROOT output file is '" << tmp << "'." << std::endl;

        CreateSpectra();

    } else if( name == "gain" ) {
        icmd >> tmp;
        if( tmp == "file" ) {
            std::string filename;
            icmd >> filename; // XXX no spaces possible
            if( !read_gainshifts(GetCalibration(), filename) ) {
                std::cerr << "gain file: Error reading '" << filename << "'.\n";
                return false;
            }
        } else if( tmp == "data" ) {
            if( !read_gainshifts(GetCalibration(), icmd) ) {
                std::cerr << "gain data: Error reading gain data.\n";
                return false;
            }
        } else {
            std::cerr << "gain: Expected 'file' or 'data', not '"<<tmp<<"'.\n";
            return false;
        }
        have_gain = true;
    } else if( name == "gain_nai_energy" ) {
        icmd >> tmp;
        if( tmp == "file" ) {
            std::string filename;
            icmd >> filename; // XXX no spaces possible
            std::ifstream ifile(filename.c_str());
            return ReadNaIEnergyCalibration(ifile);
        } else if( tmp == "data" ) {
            return ReadNaIEnergyCalibration(icmd);
        } else {
            std::cerr << "gain_nai_energy: Expected 'file' or 'data', not '"<<tmp<<"'.\n";
            return false;
        }
        have_gain = true; // XXX not completely, only nai part
    } else if( name == "parameter" ) {
        // format parameter (name = number+;)+
        while( getline(icmd, tmp, ';') ) {
            std::string par_name, equal;
            std::istringstream ipar(tmp.c_str());
            ipar >> par_name >> equal;
            if( !ipar || ipar.eof() || par_name.empty() || equal != "=" )
                break;
            std::vector<float> par_values;
            std::copy(std::istream_iterator<float>(ipar), std::istream_iterator<float>(),
                      std::back_insert_iterator<std::vector<float> >(par_values));

            PutParameter(par_name, par_values);
            std::cout << "parameter '" << par_name << "': ";
            std::copy(par_values.begin(), par_values.end(),
                      std::ostream_iterator<float>(std::cout, " "));
            std::cout << "(" << par_values.size() << " values)" << std::endl;
        }
    } else {
        return false;
    }
    return true;
}

// ########################################################################

void UserROOT::UnpackTime(unpacked_t* u)
{
    // read wall-clock time
    for( int i = 0; i<=u->nimnu; i++ ) {
        if(u->nimi[i] == 16) {
            const unsigned int wtime_hi = (unsigned int) u->nim[i];
            const unsigned int wtime_lo = (unsigned int) u->nim[i+1];
            const unsigned int wtime    = wtime_lo + ((wtime_hi & 0x0000ffff) << 16);
            time_now = wtime;
            if( time_start == 0 )
                time_start = time_now;
        }
    }
}

// ########################################################################

UserROOT::Parameter UserROOT::GetParameter(const std::string& name)
{
    parameters_t::iterator it = parameters.find(name);
    if( it != parameters.end() ) {
        return Parameter(it->second);
    } else {
        parameters[name] = par_values_t();
        return Parameter(parameters[name]);
    }
}

// ########################################################################

void UserROOT::PutParameter(const std::string& name, par_values_t& values)
{
    parameters_t::iterator it = parameters.find(name);
    if( it != parameters.end() ) {
        par_values_t& v = it->second;
        v.clear();
        v.insert(v.begin(), values.begin(), values.end());
    } else {
        par_values_t v(values.begin(), values.end());
        parameters[name] = v;
    }
}
