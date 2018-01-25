
#include "userroot.h"

#include "userutil.h"
#include "utilities.h"

#include <iostream>
#include <math.h>
#include <stdlib.h>

#define NDEBUG
#include "debug.h"

// ########################################################################

class User_XY : public UserROOT {

    const char* GetId();
    bool Sort(unpacked_t* u);
    void CreateSpectra();
    
private:
    TH2p m_back, m_front, m_e_de_individual[8][8], m_e_de_strip[8], m_e_de, m_e_de_thick;
    TH2p m_nai_e_t[28], m_alfna_individual[8][8], m_alfna, m_nai_tmp;
    TH2p m_nai_e_t_all, m_nai_e_t_c, m_siri_e_t[8], m_siri_e_t_all;
    TH1p h_na_n, h_thick, h_ede, h_ede_r[8], h_ex, h_ex_r[8];
    TH2p m_nai_t_evol[28], m_nai_e_evol[28], m_e_evol[8], m_de_evol[8][8], m_ede_r_evol[8];

    Parameter tnai_corr_enai, tnai_corr_esi, ex_from_ede, ex_corr_exp;
    float tNaI(float t, float Enai, float Esi);
};

// ########################################################################

UserRoutine* user_routine_create()
{
    return new User_XY();
}

// ########################################################################

const char* User_XY::GetId()
{
    return
/******************************************************************************/
/** This comment shoud be updated to be able to identify the version of the  **/
/** user routine used in sorting.                                            **/
/** \/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/ **/

        "ROOT sorting routine, Alexander Buerger, 2010-01-21."

/** /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ **/
/** This comment shoud be updated to be able to identify the version of the  **/
/** user routine used in sorting.                                            **/
/******************************************************************************/
        " (file " __FILE__ ", compiled on " __DATE__ " at " __TIME__ ")";
}

// ########################################################################

void User_XY::CreateSpectra()
{
    tnai_corr_enai = GetParameter("tnai_corr_enai");
    tnai_corr_esi  = GetParameter("tnai_corr_esi");
    ex_from_ede    = GetParameter("ex_from_ede");
    ex_corr_exp    = GetParameter("ex_corr_exp");

    m_back = Mat( "m_back", "back detector energies",
                  2000, 0, 20000, "E(Si) [keV]", 8, 0, 8, "detector nr." );
    m_front = Mat( "m_front", "front detector energies",
                   2000, 0, 8000, "#DeltaE(Si) [keV]", 64, 0, 64, "detector nr." );
    for(int b=0; b<8; ++b ) {
        for(int f=0; f<8; ++f ) {
            m_e_de_individual[b][f] = 
                Mat( ioprintf("m_e_de_b%df%d", b, f), ioprintf("#DeltaE : E detector %d strip %d", b, f),
                     500, 0, 20000, "E(Si) [keV]", 500, 0, 8000, "#DeltaE(Si) [keV]" );
        }
    }
    for(int f=0; f<8; ++f ) {
        m_e_de_strip[f] = Mat( ioprintf("m_e_de_f%d", f), ioprintf("E(NaI) : E(Si) strip %d", f),
                               500, 0, 20000, "E(Si) [keV]", 500, 0, 8000, "#DeltaE(Si) [keV]" );
    }

    m_e_de = Mat( "m_e_de", "#DeltaE : E for all detectors together",
                  500, 0, 20000, "E(Si) [keV]", 500, 0, 8000, "#DeltaE(Si) [keV]" );
    m_e_de_thick = Mat( "m_e_de_thick", "#DeltaE : E for all detectors together, gated on thickness",
                  500, 0, 20000, "E(Si) [keV]", 500, 0, 8000, "#DeltaE(Si) [keV]" );
    
    for(int n=0; n<28; ++n ) {
        m_nai_e_t[n] = Mat( ioprintf("m_nai_e_t_%02d", n), ioprintf("t : E NaI %d", n),
                            500, 0, 20000, "E(NaI) [keV]", 500, 0, 500, "t(NaI) [a.u.]" );
    }
    m_nai_e_t_all = Mat( "m_nai_e_t", "t : E NaI all together",
                         500, 0, 20000, "E(NaI) [keV]", 500, 0, 500, "t(NaI) [a.u.]" );
    m_nai_e_t_c   = Mat( "m_nai_e_t_c", "t : E NaI all together, corrected",
                         500, 0, 20000, "E(NaI) [keV]", 500, 0, 500, "t(NaI) [a.u.]" );

    for(int n=0; n<8; ++n ) {
        m_siri_e_t[n] = Mat( ioprintf("m_siri_e_t_%d", n), ioprintf("t(NaI) : E(Si) detector %d", n),
                             500, 0, 20000, "E(Si) [keV]", 500, 0, 500, "t(NaI) corr. [a.u.]" );
    }
    m_siri_e_t_all = Mat( "m_siri_e_t", "t(NaI) : E(Si) all detectors",
                          500, 0, 20000, "E(Si) [keV]", 500, 0, 500, "t(NaI) corr. [a.u.]" );

    for(int b=0; b<8; ++b ) {
        for(int f=0; f<8; ++f ) {
            m_alfna_individual[b][f] = 
                Mat( ioprintf("m_alfna_b%df%d", b, f), ioprintf("E(NaI) : E_{x} detector %d strip %d", b, f),
                     500, -1000, 19000, "E(NaI) [keV]", 500, -1000, 19000, "E_{x} [keV]" );
        }
    }

    m_alfna = Mat( "m_alfna", "E(NaI) : E_{x}",
                   500, -1000, 19000, "E(NaI) [keV]", 500, -1000, 19000, "E_{x} [keV]" );

    h_na_n = Spec("h_na_n", "NaI multiplicity", 32, 0, 32, "multiplicity");

    m_nai_tmp = Mat( "m_nai_tmp", "NaI tmp matrix",
                     500, 0, 20000, "? [a.u.]", 28,0,28, "det. id.");

    h_thick = Spec("h_thick", "apparent #DeltaE thickness", 500, 0, 250, "#DeltaE 'thickness' [um]");

    for(int f=0; f<8; ++f ) {
        h_ede_r[f] = Spec(ioprintf("h_ede_%d", f), ioprintf("E+#DeltaE ring %d", f),
                          500, 0, 20000, "E+#DeltaE [keV]");
        h_ede_r[f]->SetLineColor(f+1);

        h_ex_r[f] = Spec(ioprintf("h_ex_%d", f), ioprintf("E_{x} ring %d", f),
                          500, -1000, 19000, "E_{x} [keV]");
        h_ex_r[f]->SetLineColor(f+1);
    }
    h_ede = Spec("h_ede", "E+#DeltaE all detectors", 500, 0, 20000, "E+#DeltaE [keV]");
    h_ex  = Spec("h_ex", "E_{x} all detectors", 500, -1000, 19000, "E_{x} [keV]");

    // time evolution plots
    const double MT = 3e5;
    for(int n=0; n<28; ++n ) {
        m_nai_t_evol[n] = Mat( ioprintf("m_nai_t_evol%02d", n), ioprintf("time : t NaI %d", n),
                            500, 0, 500, "t(NaI) [a.u.]", 500, 0, MT, "wall clock time [s]" );
        m_nai_e_evol[n] = Mat( ioprintf("m_nai_e_evol%02d", n), ioprintf("time : e NaI %d", n),
                            500, -1000, 19000, "e(NaI) [keV]", 500, 0, MT, "wall clock time [s]" );
    }
    
    for(int b=0; b<8; ++b ) {
        m_e_evol[b] = Mat( ioprintf("m_e_evol_b%d", b), ioprintf("time : E detector %d", b),
                           500, 0, 20000, "E(Si) [keV]", 500, 0, MT, "wall clock time [s]" );
        for(int f=0; f<8; ++f ) {
            m_de_evol[b][f] = 
                Mat( ioprintf("m_de_evol_b%df%d", b, f), ioprintf("time : #DeltaE detector %d strip %d", b, f),
                     500, 0, 8000, "#DeltaE(Si) [keV]", 500, 0, MT, "wall clock time [s]" );
        }
        m_ede_r_evol[b] = Mat( ioprintf("m_ede_evol_r%d", b), ioprintf("time : E+#DeltaE ring %d", b),
                               500, 0, 20000, "E+#DeltaE(Si) [keV]", 500, 0, MT, "wall clock time [s]" );
    }
}

// ########################################################################
// ########################################################################

static float _rando = 0;
static float calib(unsigned int raw, float gain, float shift)
{
    return shift + (raw+_rando) * gain;
}

float User_XY::tNaI(float t, float Enai, float Esi)
{
   const float c = tnai_corr_enai[0] + tnai_corr_enai[1]/(Enai+tnai_corr_enai[2]) + tnai_corr_enai[3]*Enai;
   const float d = tnai_corr_esi [0] + tnai_corr_esi [1]/(Esi +tnai_corr_esi [2]) + tnai_corr_esi [3]*Esi;
   return t - c - d;
}

static float range(float E /* keV */)
{
    E /= 1000; // now E is in MeV
    // range for protons in Si, fitted output from the zrange program
    const int N=5;
    const float p[N] = { -3.67568, 15.0785, 5.84578, -0.0470111, 0.000371095 };
    float r = 0, x=1;
    for(int i=0; i<N; ++i) {
        r += p[i]*x;
        x *= E;
    }
    return r;
}

// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool User_XY::Sort(unpacked_t* u)
{
    DBGL;
    const calibration_t& cal = GetCalibration();
    UnpackTime(u);

    // random floats between -0.5 and +0.5
    _rando = drand48() - 0.5;

    // ..................................................

    // calibrate E detectors
    unsigned int si_e_raw[8] = { 0 };
    for( int i=0; i<=u->pnu; i++ ) {
        int id = u->pi[i];
        if( !(id&1) )
            continue; // ignore guard rings

        id >>= 1; // detector number 0..7

        // only keep raw E here, we don't know which front detector
        // has fired, so we also don't know which coefficients to use
        // to calibrate the back detector
        const unsigned int raw = u->p[i];
        si_e_raw[id] = raw;

        // approximate calibration
        m_back->Fill( calib( raw, cal.gaine[8*id], cal.shifte[8*id] ), id );
    }

    // ..................................................

    // calibrate dE detectors, reject event if more than one over threshold
    float si_de[8][8] =  { {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0} };
    for( int i=0; i<=u->dpnu; i++ ) {
        const int id   = u->dpi[i];
        const int id_b = id / 8;
        const int id_f = id % 8;

        const unsigned int raw = u->dp[i];
        const int de_cal = (int)calib( raw, cal.gainde[id], cal.shiftde[id] );
        si_de[id_b][id_f] = de_cal;

        m_front->Fill( de_cal, id );

    }

    // ..................................................

    int si_goodcount = 0, dei=-1, ei=-1;
    for(int b=0; b<8 && si_goodcount<2; ++b) {
        if( si_e_raw[b] == 0 )
            continue;
        for(int f=0; f<8 && si_goodcount<2; ++f) {
            if( si_de[b][f] == 0 )
                continue;

            if( (si_e_raw[b]>500 && si_de[b][f]>250)
                || (si_de[b][f]>500 && si_e_raw[b]>30) )
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

    const float e  = calib( si_e_raw[ei], cal.gaine[8*ei+dei], cal.shifte[8*ei+dei] );
    const float de = si_de[ei][dei];

    // ..................................................

    // unpack NaI time and energy into an array so that time and
    // energy are 'together'
    nai_data_t nai[32];
    int na_idx[32], na_n;
    na_n = unpack_nai(u, nai, na_idx);
    h_na_n->Fill(na_n);

    // ..................................................

    // make NaI energy calibration
    for( int i=0; i<na_n; i++ ) {
        const int id = nai[i].id;
        if( nai[i].e > 0 )
            nai[i].e = calib( (int)nai[i].e, cal.gainna[id], cal.shiftna[id] );
        if( nai[i].t > 0 )
            nai[i].t = calib( (int)(nai[i].t/8), cal.gaintna[id], cal.shifttna[id] );
    }

    // ..................................................

    // make DE:E matrices
    m_e_de_individual[ei][dei]->Fill( e, de );
    m_e_de_strip[dei]->Fill( e, de );
    m_e_de->Fill( e, de );
    const float thick = range(e+de)-range(e);
    h_thick->Fill( thick );
    const float thick_dev = 12.8+1.07e-3*e;
    const bool have_pp = fabs(thick-128)<thick_dev;
    if( !have_pp )
        return true;

    m_e_de_thick->Fill( e, de );
    const float ede = e+de;
    h_ede->Fill( ede );
    h_ede_r[dei]->Fill( ede );

    // fit of kinz Ex(E+DE)
    const float ex_theo = ex_from_ede[3*dei+0] + (ede)*(ex_from_ede[3*dei+1] + (ede)*ex_from_ede[3*dei+2]);

    // make experimental corrections
    const float ex = ex_corr_exp[2*dei]+ex_corr_exp[2*dei+1]*ex_theo;

    h_ex->Fill( ex );
    h_ex_r[dei]->Fill( ex );

    // ..................................................
    
    {   // make NaI time-energy matrix for testing
        for( int i=0; i<na_n; i++ ) {
            const int id = nai[i].id;
            if( nai[i].e>0 && nai[i].t>0 ) {
                m_nai_e_t[id]->Fill( nai[i].e, nai[i].t );
                m_nai_e_t_all->Fill( nai[i].e, nai[i].t );
                m_nai_tmp->Fill( nai[i].t*50, id );

                float t = tNaI(nai[i].t, nai[i].e, e);
                m_nai_e_t_c->Fill( nai[i].e, t );
                m_siri_e_t[ei]->Fill( e, t );
                m_siri_e_t_all->Fill( e, t );
            }
        }
    }

    // ..................................................
    
    for( int i=0; i<na_n; i++ ) {
        if( nai[i].e>0 ) {
            int w = 0;
            if( nai[i].t>175 && nai[i].t<250 )
                w = 1;
            else if( nai[i].t>75 && nai[i].t<175 )
                w = -1;
            if( w != 0 ) {
                m_alfna_individual[ei][dei]->Fill( nai[i].e, ex, w );
                m_alfna->Fill( nai[i].e, ex, w );
            }
        }
    }

    // time evolution plots
    const int timediff = time_now - time_start;
    for( int i=0; i<na_n; i++ ) {
        const int id = nai[i].id;
        m_nai_t_evol[id]->Fill(nai[i].t, timediff);
        if( nai[i].e>0 ) {
            int w = 0;
            if( nai[i].t>175 && nai[i].t<250 )
                w = 1;
            else if( nai[i].t>75 && nai[i].t<175 )
                w = -1;
            if( w != 0 )
                m_nai_e_evol[id]->Fill( nai[i].e, timediff, w );
        }
    }
    m_e_evol[ei]->Fill(e, timediff );
    m_de_evol[ei][dei]->Fill(de, timediff);
    m_ede_r_evol[dei]->Fill(ede, timediff);

    return true;
}
