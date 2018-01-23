
#include <iostream>

#include <assert.h>
#include <math.h>
#include <stdlib.h>

#include "sort_format.h"
#include "sort_spectra.h"

const char* user_routine_get_id()
{
    return
/******************************************************************************/
/** This comment shoud be updated to be able to identify the version of the  **/
/** user routine used in sorting.                                            **/
/** \/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/ **/

        "Sorting routine showing all, Alexander Buerger, 2009-07-02."

/** /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ **/
/** This comment shoud be updated to be able to identify the version of the  **/
/** user routine used in sorting.                                            **/
/******************************************************************************/
        " (file " __FILE__ ", compiled on " __DATE__ " at " __TIME__ ")";
}

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

int user_routine_do_sort(unpacked_t* u, const calibration_t* c)
{
    int de = -1, e = -1, dei= -1, ei = -1;

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

    // make E spectra
    for( int i=0; i<=u->pnu; i++ ) {
        const int id = u->pi[i];
        const int nr = id >> 1;
        const bool is_back = ((id&1)==1);

        // only calibrate E (no ceofficients for guard rings)
        int e_cal;
        if( is_back )
            e_cal = calib((u->p[i]+r1)/2., c->gaine[nr], c->shifte[nr], 0, xsize(ESP_ID));
        else
            e_cal = (int)((u->p[i]+r1)/2.);

        // sort back counters in channels 0..7, guard rings in 8..15
        const int id_spec = nr + (is_back ? 0 : 8);
        spec_fill( ESP_ID, e_cal, id_spec );

        const int thresh = e_mini[id];
        if( thresh >= 0 && e_cal > thresh ) {
            if( ei == -1 ) {
                e       = e_cal - thresh;
                ei      = id;
            } else {
                ei = -2;
            }
        }
        u->p[i] = e_cal;
    }
    if( ei<0 || (ei & 1)==0 ) {
        // no detector, or guard ring
    } else {
        ei /= 2; // from now on, e id is 0..7
    }

    // make dE spectra
    for( int i=0; i<=u->dpnu; i++ ) {
        const int id    = u->dpi[i];
        const int id_e  = id / 8;
        const int id_de = id % 8;
        assert( id_e>=0 && id_e<8 && id_de>=0 && id_de<8 );

        const int de_cal = calib((u->dp[i]+r1)/2., c->gainde[id], c->shiftde[id], 0, xsize(DESP_ID));
        spec_fill( DESP_ID, de_cal, id);

        const int thresh = de_mini[id_e][id_de];
        if( thresh >=0 && de_cal > thresh ) {
            if( ei >= 0 ) {
                //std::cout << "# " << de_cal << ' ' << std::flush;
                // spec_fill( EDESP_ID, e, de_cal/32 );
            }
            if( dei == -1 ) {
                de     = de_cal - thresh;
                dei    = id;
            } else {
                dei = -2;
            }
        }
        u->dp[i] = de_cal;
    }

#if 1
    if( dei<0 ) {
        // no dE detector
        //return true;
    }
#endif

    if( ei>=0 ) {
        spec_fill( ESP_ID, e, 20);
        if( dei >= 0 )
            spec_fill( EDESP_ID, e, de/32 );
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

    // make NaI multiplicity spectrum
    spec_fill( SINGLES_ID, na_n, 0 );

    // make NaI energy calibration
    for( int i=0; i<0*na_n; i++ ) {
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
        if( true /*na_t[i] >= time_min && na_t[i] <= time_max*/ )
            spec_fill(  NASP_ID, na_e[i], id );
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
