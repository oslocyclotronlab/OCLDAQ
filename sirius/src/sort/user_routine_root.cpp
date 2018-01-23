
#include "user_routine.h"

#include "sort_calib.h"
#include "sort_format.h"

#include <TFile.h>
#include <TH2.h>
#include <TH1.h>

#include <sstream>
#include <string>
#include <iostream>
#include <stdlib.h>
#include <strings.h>

#define NDEBUG
#include "debug.h"

static calibration_t cal;
static bool have_gain = false;

static TFile* outfile = 0;

typedef class TH1* TH1p;
typedef class TH2* TH2p;

TH2p m_back=0, m_front=0, m_e_de_individual[8][8], m_e_de=0;
TH2p m_nai_e_t[28], m_alfna_individual[8][8], m_alfna;

// ########################################################################

const char* user_routine_get_id()
{
    return
/******************************************************************************/
/** This comment shoud be updated to be able to identify the version of the  **/
/** user routine used in sorting.                                            **/
/** \/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/ **/

        "ROOT sorting routine, Alexander Buerger, 2009-06-06."

/** /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ **/
/** This comment shoud be updated to be able to identify the version of the  **/
/** user routine used in sorting.                                            **/
/******************************************************************************/
        " (file " __FILE__ ", compiled on " __DATE__ " at " __TIME__ ")";
}

// ########################################################################

std::string fmt(const char *fmt, ...) __attribute__ ((format (printf, 1, 2)));
std::string fmt(const char *fmt, ...)
{
    char msgbuf[8192];
    va_list ap;
    va_start (ap, fmt);
    (void) vsnprintf(msgbuf, sizeof(msgbuf), fmt, ap);
    va_end (ap);

    return msgbuf;
}

// ########################################################################

TH1p Spec( const std::string& name, const std::string& title,
           int channels, double left, double right, const std::string& xtitle )
{
    TH1* h = new TH1I( name.c_str(), title.c_str(), channels, left, right );

    TAxis* xax = h->GetXaxis();
    xax->SetTitle(xtitle.c_str());
    xax->SetTitleSize(0.03);
    xax->SetLabelSize(0.03);

    TAxis* yax = h->GetYaxis();
    yax->SetLabelSize(0.03);

    return h;
}

// ----------------------------------------------------------------------

TH2p Mat( const std::string& name, const std::string& title,
          int ch1, double l1, double r1, const std::string& xtitle, 
          int ch2, double l2, double r2, const std::string& ytitle)
{
    TH2p mat = new TH2F( name.c_str(), title.c_str(), ch1, l1, r1, ch2, l2, r2 );
    mat->SetOption( "colz" );
    mat->SetContour( 64 );

    TAxis* xax = mat->GetXaxis();
    xax->SetTitle(xtitle.c_str());
    xax->SetTitleSize(0.03);
    xax->SetLabelSize(0.03);

    TAxis* yax = mat->GetYaxis();
    yax->SetTitle(ytitle.c_str());
    yax->SetTitleSize(0.03);
    yax->SetLabelSize(0.03);
    yax->SetTitleOffset(1.3);

    TAxis* zax = mat->GetZaxis();
    zax->SetLabelSize(0.025);

    return mat;
}

// ########################################################################

bool user_routine_init(bool)
{
    return true;
}

// ########################################################################

bool user_routine_finish()
{
    if( outfile ) {
        outfile->Write();
        outfile->Close();
        delete outfile;
    }

    return true;
}

// ########################################################################

bool user_routine_cmd(const std::string& cmd)
{
    std::istringstream icmd(cmd.c_str());

    std::string name, tmp;
    icmd >> name;

    if( name == "dump" ) {
        outfile->Write();
    } else if( name == "outfile" ) {
        if( outfile ) {
            outfile->Write();
            outfile->Close();
            delete outfile;
        }

        icmd >> tmp;
        outfile = new TFile(tmp.c_str(), "recreate");
        std::cout << "ROOT output file is '" << tmp << "'." << std::endl;

        m_back = Mat( "m_back", "back detector energies",
                      2000, 0, 20, "energy [MeV]", 8, 0, 8, "detector nr." );
        m_front = Mat( "m_front", "front detector energies",
                       2000, 0, 8, "energy [MeV]", 64, 0, 64, "detector nr." );
        for(int b=0; b<8; ++b ) {
            for(int f=0; f<8; ++f ) {
                m_e_de_individual[b][f] = 
                    Mat( fmt("m_e_de_b%df%d", b, f), fmt("#DeltaE : E detector %d strip %d", b, f),
                         500, 0, 20, "energy [MeV]", 500, 0, 8, "energy [MeV]" );
            }
        }
        m_e_de = Mat( "m_e_de", "#DeltaE : E for all detectors together",
                      500, 0, 20, "energy [MeV]", 500, 0, 8, "energy [MeV]" );

        for(int n=0; n<28; ++n ) {
            m_nai_e_t[n] = Mat( fmt("m_nai_e_t_%02d", n), fmt("t : E NaI %d", n),
                                500, 0, 20, "energy [MeV]", 500, 0, 500, "time [a.u.]" );
        }

        for(int b=0; b<8; ++b ) {
            for(int f=0; f<8; ++f ) {
                m_alfna_individual[b][f] = 
                    Mat( fmt("m_alfna_b%df%d", b, f), fmt("E(NaI) : E(Si) detector %d strip %d", b, f),
                         500, 0, 20, "energy [MeV]", 500, 0, 20, "energy [MeV]" );
            }
        }
        m_alfna = Mat( "m_alfna", "E(NaI) : E(Si) detector %d strip %d",
                       500, 0, 20, "energy [MeV]", 500, 0, 20, "energy [MeV]" );
    } else if( name == "gain" ) {
        icmd >> tmp;
        if( tmp == "file" ) {
            std::string filename;
            icmd >> filename; // XXX no spaces possible
            if( !read_gainshifts(cal, filename) ) {
                std::cerr << "gain file: Error reading '" << filename << "'.\n";
                return false;
            }
        } else if( tmp == "data" ) {
            if( !read_gainshifts(cal, icmd) ) {
                std::cerr << "gain data: Error reading gain data.\n";
                return false;
            }
        } else {
            std::cerr << "gain: Expected 'file' or 'data', not '"<<tmp<<"'.\n";
            return false;
        }
        have_gain = true;
    } else {
        return false;
    }
    return true;
}

// ########################################################################

bool user_routine_data(const std::string& /*filename*/)
{
    if( !outfile )
        return false;

    if( !have_gain ) {
        std::cerr << "data: no gain/shift." << std::endl;
	reset_gainshifts(cal);
        //return false;
    }
    return true;
}

// ########################################################################
// ########################################################################

static float _rando = 0;
static float calib(unsigned int raw, float gain, float shift)
{
    return shift + (raw+_rando) * gain;
}

// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool user_routine_sort(unpacked_t* u)
{
    DBGL;

    // random floats between -0.5 and +0.5
    _rando = drand48() - 0.5;

    // ..................................................

    // calibrate E detectors
    float si_e[8] = { 0 };
    for( int i=0; i<=u->pnu; i++ ) {
        int id = u->pi[i];
        if( !(id&1) )
            continue; // ignore guard rings

        id >>= 1; // detector number 0..7

        // only calibrate E (no ceofficients for guard rings)
        const unsigned int raw = u->p[i];
        const float e_cal = calib( raw, cal.gaine[id], cal.shifte[id] );
        si_e[id] = e_cal;

        m_back->Fill( e_cal/200, id );
    }

    // ..................................................

    // calibrate dE detectors, reject event if more than one over threshold
    float si_de[8][8] =  { {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0} };
    for( int i=0; i<=u->dpnu; i++ ) {
        const int id   = u->dpi[i];
        const int id_b = id / 8;
        const int id_f = id % 8;

        const unsigned int raw = u->dp[i];
        const int de_cal = calib( raw, cal.gainde[id], cal.shiftde[id] );
        si_de[id_b][id_f] = de_cal;

        m_front->Fill( de_cal/500, id );

    }

    // ..................................................

    int si_goodcount = 0, dei=-1, ei=-1;
    for(int b=0; b<8 && si_goodcount<2; ++b) {
        if( si_e[b] == 0 )
            continue;
        for(int f=0; f<8 && si_goodcount<2; ++f) {
            if( si_de[b][f] == 0 )
                continue;

            if( (si_e[b]/200>4 && si_de[b][f]/500>0.5)
                || (si_de[b][f]/500>1 && si_e[b]/200>0.5) )
            {
                si_goodcount += 1;
                ei  = b;
                dei = f;
            }
        }
    }
            
    if( si_goodcount != 1 )
        // no detector above threshold, reject event
        return true;

    const float e = si_e[ei], de = si_de[ei][dei];

    // ..................................................

    // unpack NaI time and energy into an array so that time and
    // energy are 'together'
    float na_t[32], na_e[32];
    int na_i[32], na_idx[32], na_n = 0;
    for(int i=0; i<32; ++i)
        na_idx[i] = -1;
    for( int i=0; i<=u->nanu; i++ ) {
        int id = u->nai[i];
        na_idx[id] = na_n;
        na_i[na_n] = id;
        na_e[na_n] = u->na[i];
        na_t[na_n] = 0;
        na_n += 1;
    }
    for( int i=0; i<=u->tnanu; i++ ) {
        int id = u->tnai[i];
        if( na_idx[id]<0 ) {
            if( id>=28 )
                std::cout << '.' << std::flush;
            na_idx[id] = na_n;
            na_i[na_n] = id;
            na_e[na_n] = 0;
            na_n += 1;
        }
        na_t[na_idx[id]] = u->tna[i];
    }

    // ..................................................

    // make NaI energy calibration
    for( int i=0; i<na_n; i++ ) {
        const int id = na_i[i];
        if( na_e[i] > 0 )
            na_e[i] = calib( na_e[i]/2, cal.gainna[id], cal.shiftna[id] )/100;
        if( na_t[i] > 0 )
            na_t[i] = calib( na_t[i]/8, 1, cal.shifttna[id] );
    }

    // ..................................................

    // make DE:E matrices
    m_e_de_individual[ei][dei]->Fill( e/200, de/500 );
    m_e_de->Fill( e/200, de/500 );

    // ..................................................
    
    {   // make NaI time-energy matrix for testing
        for( int i=0; i<na_n; i++ ) {
            const int id = na_i[i];
            if( na_e[i]>0 && na_t[i]>0 )
                m_nai_e_t[id]->Fill( na_e[i], na_t[i] );
        }
    }

    // ..................................................
    
    for( int i=0; i<na_n; i++ ) {
        if( na_e[i]>0 ) {
            int w = 0;
            if( na_t[i]>150 && na_t[i]<300 )
                w = 1;
            else if( na_t[i]>0 && na_t[i]<150 )
                w = -1;
            if( w != 0 ) {
                m_alfna_individual[ei][dei]->Fill( na_e[i], e/200 );
                m_alfna->Fill( na_e[i], e/200 );
            }
        }
    }

    return true;
}

// ########################################################################
