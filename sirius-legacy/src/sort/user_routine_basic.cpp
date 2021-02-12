
#include "user_routine_basic.h"

#include "sort_format.h"
#include "sort_spectra.h"
#include "spectrum_rw.h"

#include <sstream>
#include <iostream>
#include <strings.h>

#if 0
static const char* PATH_GAINSHIFT  = "/Applications/sirius/data/gainshift.init";
static const char* PATH_TELEWIN    = "/Applications/sirius/data/telewin.init";
static const char* PATH_RANGE_DATA = "/Applications/sirius/data/RANGE.DATA";
//#else
static const char* PATH_GAINSHIFT  = "gainshift.init";
static const char* PATH_TELEWIN    = "telewin.init";
static const char* PATH_RANGE_DATA = "RANGE.DATA";
#endif

// ########################################################################

UserRoutineBasic::UserRoutineBasic()
    : have_gain( false )
    , have_telewin( false )
    , have_range( false )
{
}

// ########################################################################

bool UserRoutineBasic::Init(bool online)
{
    // attach offline spectra
    if( !spectra_attach_all(online) ) {
        std::cerr << "Failed to attach offline shm spectra." << std::endl;
        return false;
    }

    // clear all offline spectra
    for(int i=1; sort_spectra[i].name; ++i) {
        const sort_spectrum_t* s = &sort_spectra[i];
        bzero(s->ptr, s->ydim*s->xdim*sizeof(*s->ptr));
    }

    // read_gainshifts(calibration, PATH_GAINSHIFT);
    // read_telewin(calibration, PATH_TELEWIN);
    // read_range_data(calibration, PATH_RANGE_DATA);

    return true;
}

// ########################################################################

bool UserRoutineBasic::Finish()
{
    UserRoutineBasic::Cmd("dump");

    // detach shared memory
    spectra_detach_all();

    return true;
}

// ########################################################################

bool UserRoutineBasic::Cmd(const std::string& cmd)
{
    std::istringstream icmd(cmd.c_str());

    std::string name, tmp;
    icmd >> name;

    if( name == "dump" ) {
        float cal[13][6] = {
            /*         */ { 0, 1, 0, 0, 1, 0 },
            /* SINGLES */ { 0, 1, 0, 0, 1, 0 },
            /* ESP     */ { 0, 120, 0, 0, 1, 0 },
            /* DESP    */ { 0, 120, 0, 0, 1, 0 },
            /* EDESP   */ { 0, 120, 0, 0, 120, 0 },
            /* THICKSP */ { 0, 1, 0, 0, 1, 0 },
            /* GESP    */ { 0, 10, 0, 0, 1, 0 },
            /* TGESP   */ { 0, 1, 0, 0, 1, 0 },
            /* NASP    */ { 0, 10, 0, 0, 1, 0 },
            /* TNASP   */ { 0, 1, 0, 0, 1, 0 },
            /* ALFNA   */ { 0, 1, 0, 0, 1, 0 }, //{ 0, 120, 0, 0, 10, 0 },
            /* ALFGE   */ { 0, 120, 0, 0, 10, 0 },
            /* MAT     */ { 0, 1, 0, 0, 1, 0 },
        };
        for(int i=1; sort_spectra[i].specno; ++i) {
            if( !dump_spectrum(&sort_spectra[i], i<13 ? cal[i] : 0) ) {
                std::cerr << "dump: Could not write '" << sort_spectra[i].name << "'.\n";
                return false;
            }
        }
        std::cout << "dump: All spectra dumped.\n";
    } else if( name == "gain" ) {
        icmd >> tmp;
        if( tmp == "file" ) {
            std::string filename;
            icmd >> filename; // XXX no spaces possible
            if( !read_gainshifts(calibration, filename) ) {
                std::cerr << "gain file: Error reading '" << filename << "'.\n";
                return false;
            }
        } else if( tmp == "data" ) {
            if( !read_gainshifts(calibration, icmd) ) {
                std::cerr << "gain data: Error reading gain data.\n";
                return false;
            }
        } else {
            std::cerr << "gain: Expected 'file' or 'data', not '"<<tmp<<"'.\n";
            return false;
        }
        have_gain = true;
    } else if( name == "range" ) {
        icmd >> tmp;
        if( tmp == "file" ) {
            std::string filename;
            icmd >> filename; // XXX no spaces possible
            if( !read_range_data(calibration, filename) ) {
                std::cerr << "range file: Error reading '" << filename << "'.\n";
                return false;
            }
        } else {
            std::cerr << "range: Expected 'file', not '"<<tmp<<"'.\n";
            return false;
        }
        have_range = true;
    } else if( name == "telewin" ) {
        icmd >> tmp;
        if( tmp == "file" ) {
            std::string filename;
            icmd >> filename; // XXX no spaces possible
            if( !read_telewin(calibration, filename) ) {
                std::cerr << "telewin file: Error reading '" << filename << "'.\n";
                return false;
            }
        } else if( tmp == "data" ) {
            if( !read_gainshifts(calibration, icmd) ) {
                std::cerr << "telewin data: Error reading telewin data.\n";
                return false;
            }
        } else {
            std::cerr << "telewin: Expected 'file' or 'data', not '"<<tmp<<"'.\n";
            return false;
        }
        have_telewin = true;
    } else {
        return false;
    }
    return true;
}

// ########################################################################

bool UserRoutineBasic::Data(const std::string& /*filename*/)
{
    if( !have_gain ) {
        std::cerr << "data: no gain/shift." << std::endl;
	reset_gainshifts(calibration);
        //return false;
    }
    if( !have_telewin ) {
        std::cerr << "data: no telewin." << std::endl;
	reset_telewin(calibration);
        //return false;
    }
    if( !have_range ) {
        std::cerr << "data: no range data." << std::endl;
        //return false;
    }

    return true;
}
