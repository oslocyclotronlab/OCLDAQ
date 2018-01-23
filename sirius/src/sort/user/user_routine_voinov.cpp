

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

        "Sorting routine for A. Voinov experiment, 2009-10-27."

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

    // random floats between -0.5 and +0.5
    const float r1 = drand48() - 0.5;

    // unpack time and energy into an array so that time and
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

    int ei   = na_idx[0];
    int dei  = na_idx[16];
    int moni = na_idx[2];

    int e   = (ei  >=0) ? na_e[ei  ]/2 : -1;
    int de  = (dei >=0) ? na_e[dei ]/2 : -1;
    int dt  = (dei >=0) ? na_t[dei ]/8 : -1;
    int mon = (moni>=0) ? na_e[moni]/2 : -1;

    if( ei>=0 )
        spec_fill( NASP_ID,  e,  0 );
    if( dei>=0 ) {
        spec_fill( NASP_ID,  de, 1 );
        spec_fill( TNASP_ID, dt, 1 );
    }
    if( moni>=0 )
        spec_fill( NASP_ID, mon, 2 );

    // dE-E matrix in EDESP
    if( ei>=0 && dei>=0 ) {
        spec_fill( EDESP_ID, e, de/32 );
        spec_fill( ALFNA_ID, e, de/4 );
    }

    // dE time-energy matrix in MAT
    if( dei>=0 ) {
        spec_fill( MAT_ID,   de, dt/8 );
        spec_fill( ALFGE_ID, de, dt );
    }

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

