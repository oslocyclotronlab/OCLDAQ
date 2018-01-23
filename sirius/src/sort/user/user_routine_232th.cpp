

#include "user_routine_basic.h"

#include <iostream>

#include <assert.h>
#include <math.h>
#include <stdlib.h>

#include "sort_format.h"
#include "sort_spectra.h"
#include "utilities.h"

// ########################################################################

class UserRoutineMAMA : public UserRoutineBasic {
    const char* GetId();
    bool Sort(unpacked_t* u);
};

// ########################################################################

UserRoutine* user_routine_create()
{
    return new UserRoutineMAMA();
}

// ########################################################################

const char* UserRoutineMAMA::GetId()
{
    return
/******************************************************************************/
/** This comment shoud be updated to be able to identify the version of the  **/
/** user routine used in sorting.                                            **/
/** \/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/ **/

        "Sorting routine for 232Th, Alexander Buerger, 2009-12-03."

/** /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ **/
/** This comment shoud be updated to be able to identify the version of the  **/
/** user routine used in sorting.                                            **/
/******************************************************************************/
        " (file " __FILE__ ", compiled on " __DATE__ " at " __TIME__ ")";
}

// ########################################################################

inline int xsize(int specno) { return sort_spectra[specno].xdim; }

inline int calib(float raw, float gain, float shift, int mini, int maxi)
{
    int c = (int) (raw * gain + shift + 0.5);
    if( c < mini )
        c = 0;
    if( c >= maxi )
        c = maxi-1;
    return c;
}

// ########################################################################

#if 0
static float tNaI(float t, float Enai, float Esi)
{
   const float p0 =  2.12985e+02 - 200;
   const float p1 =  3.04476e+04;
   const float p2 = -5.88935e+01;
   const float p3 = -4.43951e-03;
   const float c = p0 + p1/(Enai+p2) + p3*Enai;

   const float q0 =  2.12641e+02-200;
   const float q1 = -1.08751e+05;
   const float q2 = -5.16198e+02;
   const float q3 = -9.95697e-05;
   const float d = q0 + q1/(Esi+q2) + q3*Esi;

   return t - c - d;
}

// range for particles in Si; parameter E is in keV
static float range(float E)
{
    E /= 1000;
    const int N=5;
    // for protons in Si
    //const float p[N] = { -3.67568, 15.0785, 5.84578, -0.0470111, 0.000371095 };
    // range for 3He in Si
    const float p[N] = { 0.134944, 2.50336, 0.616001, -0.00520644, 4.33841e-05 };
    float r = 0, x=1;
    for(int i=0; i<N; ++i) {
        r += p[i]*x;
        x *= E;
    }
    return r;
}
#endif

bool UserRoutineMAMA::Sort(unpacked_t* u)
{
    const calibration_t* c = &GetCalibration();

#if 0
    std::cout << "NaI: t=" << ioprintf("%3d",u->tnanu+1) << " E=" << ioprintf("%3d",u->nanu+1)
              << " Si: E=" << ioprintf("%3d",u->pnu+1) << " dE=" << ioprintf("%3d",u->dpnu+1)
              << std::endl;
#endif

    int de = -1, e = -1, dei= -1, ei = -1, ne = 0, nde = 0;

    // guard has even id, back has odd number
    const int EMIN = 80;
    const int e_mini[16] = {  -1, EMIN,  -1, EMIN,  -1, EMIN,  -1, EMIN,
                              -1, EMIN,  -1, EMIN,  -1, EMIN,  -1, EMIN };
    const int DEMIN = 35;
    const int de_mini[8][8] = { {  DEMIN, DEMIN, DEMIN, DEMIN, DEMIN, DEMIN, DEMIN, DEMIN,  },
                                {  DEMIN, DEMIN, DEMIN, DEMIN, DEMIN, DEMIN, DEMIN, DEMIN,  },
                                {  DEMIN, DEMIN, DEMIN, DEMIN, DEMIN, DEMIN, DEMIN, DEMIN,  },
                                {  DEMIN, DEMIN, DEMIN, DEMIN, DEMIN, DEMIN, DEMIN, DEMIN,  },
                                {  DEMIN, DEMIN, DEMIN, DEMIN, DEMIN, DEMIN, DEMIN, DEMIN,  },
                                {  DEMIN, DEMIN, DEMIN, DEMIN, DEMIN, DEMIN, DEMIN, DEMIN,  },
                                {  DEMIN, DEMIN, DEMIN, DEMIN, DEMIN, DEMIN, DEMIN, DEMIN,  },
                                {  DEMIN, DEMIN, DEMIN, DEMIN, DEMIN, DEMIN, DEMIN, DEMIN,  } };

    // random floats between -0.5 and +0.5
    const float r1 = drand48() - 0.5;
    //const float r2 = drand48() - 0.5;

    int ge_t = 0, ge_e = 0;

    // make E spectra
    for( int i=0; i<=u->pnu; i++ ) {
        const int id = u->pi[i];
        const int nr = id >> 1;
        const bool is_back = ((id&1)==1);

        //if( id==14 || id == 15 ) continue;

        spec_fill( SINGLES_ID, id, 3 );
        const int val = u->p[i];
        if( id == 25 ) {
            ge_e = val;
            continue;
        } else if( id == 26 ) {
            ge_t = val;
            continue;
        }

        // only calibrate E (no ceofficients for guard rings)
        int e_cal;
        if( is_back )
            e_cal = calib((val+r1)/2., c->gaine[nr], c->shifte[nr], 0, xsize(ESP_ID));
        else
            e_cal = (int)((val+r1)/2.);

        // sort back counters in channels 0..7, guard rings in 8..15
        const int id_spec = nr + (is_back ? 0 : 8);
        spec_fill( ESP_ID, e_cal, id_spec );

        const int thresh = e_mini[id];
        if( thresh >= 0 && e_cal > thresh ) {
            if( ne == 0 || e<(e_cal-thresh) ) {
                e       = e_cal - thresh;
                ei      = id;
            }
            ne += 1;
        }
        u->p[i] = e_cal;
    }
    if( ne==1 ) {
        ei /= 2; // from now on, e id is 0..7
    } else {
        ei = -1;
        // no or >1 detectors, or guard ring
    }
    spec_fill( SINGLES_ID, ne, 2 );

    // make dE spectra
    for( int i=0; i<=u->dpnu; i++ ) {
        const int id    = u->dpi[i];
        const int id_e  = id / 8;
        const int id_de = id % 8;
        assert( id_e>=0 && id_e<8 && id_de>=0 && id_de<8 );

        //if( id_e==7 ) continue;

        const int de_cal = calib((u->dp[i]+r1)/2., c->gainde[id], c->shiftde[id], 0, xsize(DESP_ID));
        spec_fill( DESP_ID, de_cal, id);

        const int thresh = de_mini[id_e][id_de];
        if( id_e == ei && thresh >=0 && de_cal > thresh ) {
            if( nde == 0 || de < (de_cal - thresh) ) {
                de     = de_cal - thresh;
                dei    = id;
            }
            nde += 1;
        }
        u->dp[i] = de_cal;
    }
    spec_fill( SINGLES_ID, 16+nde, 2 );

#if 0
    if( nde!=1 ) {
        // no or >2 dE detectors
        return true;
    }
#endif

    bool have_particle = true;
    if( ei>=0 ) {
        spec_fill( ESP_ID, e, 20);
        if( dei >= 0 ) {
            spec_fill( EDESP_ID, e, de/32 );

            //const float curve = 37.2759 - 0.0380808*e + 1.58104e-05*(e*e);
            //have_particle = ( fabs(de/32 - curve) < 3 );
            if( have_particle && ei == 0 && dei==3 )
                spec_fill( MAT_ID, e, de/32 );
        }
    }

    // unpack NaI time and energy into an array so that time and
    // energy are 'together'
    int na_t[32], na_e[32], na_i[32], na_idx[32], na_n = 0;
    for(int i=0; i<32; ++i)
        na_idx[i] = -1;
    for( int i=0; i<=u->nanu; i++ ) {
        int id = u->nai[i];
        if( id>=28 )
            std::cout << '.' << std::flush;
        na_idx[id] = na_n;
        na_i[na_n] = id;
        na_e[na_n] = u->na[i];
        na_t[na_n] = -100;
        na_n += 1;
    }
    for( int i=0; i<=u->tnanu; i++ ) {
        int id = u->tnai[i];
        if( na_idx[id]<0 ) {
            if( id>=28 )
                std::cout << '.' << std::flush;
            na_idx[id] = na_n;
            na_i[na_n] = id;
            na_e[na_n] = -200;
            na_n += 1;
        }
        na_t[na_idx[id]] = u->tna[i];
    }

    // make NaI multiplicity spectrum
    spec_fill( SINGLES_ID, na_n, 0 );

    // make NaI energy calibration
    for( int i=0; i<na_n; i++ ) {
        const int id = na_i[i];
        if( na_e[i] > 0 )
            na_e[i] = calib( (na_e[i]+r1)/2,  c->gainna[id], c->shiftna[id],  0, xsize(NASP_ID));
        if( na_t[i] > 0 )
            na_t[i] = calib( (na_t[i]+r1)/8.0, 1,            c->shifttna[id], 0, xsize(TNASP_ID));
    }

    // make NaI energy and time spectra
    for( int i=0; i<na_n; i++ ) {
        const int id = na_i[i];
        spec_fill( TNASP_ID, na_t[i], id );
        if( na_e[i] > 1 /* && na_t[i] >= time_min && na_t[i] <= time_max*/ )
            spec_fill(  NASP_ID, na_e[i], id );

        //if( have_particle )
        //    spec_fill(  MAT_ID, na_e[i], na_t[i]/8 );
        //if( na_e[i] > 1 && na_n<5 && have_particle )
        //    spec_fill(  MAT_ID, na_e[i], id );
        if(e>400 && de>160) {
            /*if( id == 24 )
                spec_fill( ALFGE_ID, na_e[i], na_t[i] );
                else*/ if( id == 12 )
                spec_fill( ALFNA_ID, na_e[i], na_t[i] );
        }
    }

    if( ge_e>0 ) {
        spec_fill( GESP_ID,  ge_e, 3 );
        const float ge_e_cal = (ge_e+r1-139.7)*2.035; 
        spec_fill( GESP_ID,  ge_e_cal / 2, 2 );
        if( ge_t>0 ){
            spec_fill( GESP_ID,  ge_e_cal / 2, 1 );
            spec_fill( ALFGE_ID, ge_e_cal / 4, ge_t/8 );
        }
        if ( e>440 && e<550 && de>350 && de<420 ) { //&& ge_t>900 && ge_t<1110) {
            spec_fill( GESP_ID, ge_e_cal / 2, 0 );
        }
            if ( e>260 && e<360 && de>410 && de<500 ) { //&& ge_t>900 && ge_t<1110) {
            spec_fill( GESP_ID, ge_e_cal / 2, 4 );
        }
     }
    if( ge_t>0 )
        spec_fill( TGESP_ID, ge_t/8, 4 );
    
#if 0
    // gate on the 2nd peak in (d,d')
    if( have_particle && de/32>10.5 && de/32<14.5 && e>430 && e<550 ) {
        for( int i=0; i<na_n; i++ ) {
            //if( na_i[i]>=10 && na_i[i]<21 )
            //    spec_fill( ALFNA_ID, na_e[i], na_t[i] );
            spec_fill( ALFNA_ID, na_e[i], na_i[i] );
        }
    }
#endif

    /* ******************************************************************** */
    /* Making 16 scaler read out, stored in row 0 of SINGLES matrix         */
    /* Scaler value stored in chs 4000-15, and increments in chs 40032-4047 */
    /* The scaler value (32 bits) is composed of two 16-bit words           */
    /* 16 LSB are in scaler-chs 0-15, 16 MSB are in chs 16-31               */
    /* ******************************************************************** */
    for( int i=0; i<=u->scnu; i += 2 ) {
        const unsigned int scaler_hi = (unsigned int) u->sc[i];
        const unsigned int scaler_lo = (unsigned int) u->sc[i+1];
        const unsigned int scaler    = ((scaler_hi & 0x0000ffff) << 16) + scaler_lo;

        const unsigned int scaler_old = (unsigned int)spec_get(SINGLES_ID, u->sci[i]+4000,0);
        spec_set( SINGLES_ID, u->sci[i] + 4032, 0, scaler - scaler_old );
        spec_set( SINGLES_ID, u->sci[i] + 4000, 0, scaler); // store old value
    }

    /* ************************************************************* */
    /* Reading wall-clock time and store in row 0 of SINGLES matrix */
    /* Time value stored in ch 4065, and incremented time in ch 4064 */
    /* The time value (32 bits) is composed of two 16-bit words      */
    /* 16 LSB are in ch 16, 16 MSB are in ch 17                      */
    /* ************************************************************* */
    for( int i = 0; i<=u->nimnu; i++ ) {
        if(u->nimi[i] == 16) {
            const unsigned int wtime_hi = (unsigned int) u->nim[i];
            const unsigned int wtime_lo = (unsigned int) u->nim[i+1];
            const unsigned int wtime    = wtime_lo + ((wtime_hi & 0x0000ffff) << 16);

            const unsigned int wtime_old = (unsigned int)spec_get(SINGLES_ID, 4094, 1);
            spec_set( SINGLES_ID, 4093, 1, wtime - wtime_old );
            spec_set( SINGLES_ID, 4094, 1, wtime );

            // calculate/estimate event rate
            unsigned int wtime_start = (unsigned int)spec_get(SINGLES_ID, 4095, 1);
            if( wtime_start == 0 )
                spec_set( SINGLES_ID, 4095, 1, wtime_start = wtime );

            int ch = (wtime - wtime_start)/8; // 1 ch per 8 seconds
            if( ch < 0 )
                ch = 4092;
            else if( ch > 4090 )
                ch = 4091;
            spec_fill( SINGLES_ID, ch, 1 );
        }
    }

    return true;
}

