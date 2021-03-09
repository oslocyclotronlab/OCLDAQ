
#include "sort_calib.h"

#include "spectrum_rw.h"
#include "sort_format.h"
#include "utilities.h"

#include <fstream>
#include <iostream>
#include <sstream>

// ########################################################################
// ########################################################################

void reset_gainshifts(calibration_t& calib)
{
    /* Initialization of gain and shift variables (written by sirius_gui) */
    for(int i=0; i<64; i++) {
        calib.gaine [i] = calib.gainde [i] = 1.0;
        calib.shifte[i] = calib.shiftde[i] = 0.0;
    }

    for(int i=0; i<6; i++) {
        calib.gainge[i]  = 1.0;
        calib.shiftge[i] = calib.shifttge[i] = 0.0;
    }

    for(int i=0; i<32; i++) {
        calib.gainna[i]  = 1.0;
        calib.shiftna[i] = calib.shifttna[i] = 0.0;
    }
}

// ########################################################################

bool read_gainshifts(calibration_t& calib, std::istream& fp)
{
    reset_gainshifts(calib);

    // read gain and shift values from stream
    if( !fp )
        return false;

    for(int i=0; i<64; i++)
        fp >> calib.gaine[i];
    for(int i=0; i<64; i++)
        fp >> calib.gainde[i];
    for(int i=0; i<6; i++)
        fp >> calib.gainge[i];
    for(int i=0; i<32; i++)
        fp >> calib.gainna[i];

    for(int i=0; i<64; i++)
        fp >> calib.shifte[i];
    for(int i=0; i<64; i++)
        fp >> calib.shiftde[i];
    for(int i=0; i<6; i++)
        fp >> calib.shiftge[i];
    for(int i=0; i<32; i++)
        fp >> calib.shiftna[i];

    for(int i=0; i<6; i++)
        fp >> calib.shifttge[i];
    for(int i=0; i<32; i++)
        fp >> calib.shifttna[i];

    for(int i=0; i<6; i++)
        fp >> calib.gaintge[i];
    for(int i=0; i<32; i++)
        fp >> calib.gaintna[i];

    return bool(fp);
}

// ########################################################################

bool read_gainshifts(calibration_t& calib, const std::string& filename)
{
    std::ifstream fp(filename.c_str());
    return read_gainshifts(calib, fp);
}

// ########################################################################

std::string format_gainshift(const calibration_t& calib)
{
    std::ostringstream o;

    for(int i=0; i<64; i++)
        o << calib.gaine[i] << ' ';
    for(int i=0; i<64; i++)
        o << calib.gainde[i] << ' ';
    for(int i=0; i<6; i++)
        o << calib.gainge[i] << ' ';
    for(int i=0; i<32; i++)
        o << calib.gainna[i] << ' ';

    for(int i=0; i<64; i++)
        o << calib.shifte[i] << ' ';
    for(int i=0; i<64; i++)
        o << calib.shiftde[i] << ' ';
    for(int i=0; i<6; i++)
        o << calib.shiftge[i] << ' ';
    for(int i=0; i<32; i++)
        o << calib.shiftna[i] << ' ';

    for(int i=0; i<6; i++)
        o << calib.shifttge[i] << ' ';
    for(int i=0; i<32; i++)
        o << calib.shifttna[i]  << ' ';

    return o.str();
}

// ########################################################################

void reset_telewin(calibration_t& calib)
{
    // initialization of low and high markers on thickness spectra
    for(int i=0; i<64; i++) {
        calib.ml[i] = 0;
        calib.mh[i] = 511;
    }
}

// ########################################################################

bool read_telewin(calibration_t& calib, std::istream& fp)
{
    reset_telewin(calib);

    // read low and high markers from stream
    if( !fp )
        return false;

    for(int i=0; i<64; i++) {
        skipsymbols(fp) >> calib.ml[i];
        skipsymbols(fp) >> calib.mh[i];
    }
    return bool(fp);
}

// ########################################################################

bool read_telewin(calibration_t& calib, const std::string& fname)
{
    std::ifstream fp(fname.c_str());
    return read_telewin(calib, fp);
}

// ########################################################################

std::string format_telewin(calibration_t& calib)
{
    std::ostringstream o;
    for(int i=0; i<64; i++)
        o << calib.ml[i] << ' ' << calib.mh[i] << ' ';
    return o.str();
}

// ########################################################################

bool read_range_data(calibration_t& calib, const std::string& fname)
{
    // read range curve from file
    std::ifstream fp(fname.c_str());
    if( !fp )
        return false;

    std::string comment;
    float cal[6];
    int xdim = 0;
    norr1dim(fp, comment, xdim, calib.range, cal);

    if(xdim != 2048)
        std::cerr << "Warning, range data has dimension " << xdim << "(should have 2048)\n";
    return !(!fp);
}
