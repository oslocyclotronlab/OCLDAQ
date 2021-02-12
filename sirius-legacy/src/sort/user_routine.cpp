
#include <stdlib.h>
#include <math.h>

#include "sort_format.h"
#include "sort_spectra.h"

const char* get_user_routine_id()
{
    return
/******************************************************************************/
/** This comment shoud be updated to be able to identify the version of the  **/
/** user routine used in sorting.                                            **/
/** \/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/ **/

	"SIRIUS sorting routine, Magne Guttormsen, 2009-03-05, Exp: CAEN TDC's and ADC's."

/** /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ **/
/** This comment shoud be updated to be able to identify the version of the  **/
/** user routine used in sorting.                                            **/
/******************************************************************************/
	" (file " __FILE__ ", compiled on " __DATE__ " at " __TIME__ ")";
}

static float dT_start(int ch);
static float dT_stop(int ch);

inline int xsize(int specno) { return sort_spectra[specno].xdim; }

inline int calib(float raw, float gain, float shift, int mini, int maxi)
{
    int c = (int) (raw * gain + shift + 0.5);
    if( c<mini || c>=maxi)
        c = 0;
    return c;
}

int user_routine(unpacked_t* u, const calibration_t* c)
{
    int de = -1, e = -1, ede = -1,  t = -1, t0 = -1, ti = -1, dei= -1;

    // random floats between -0.5 and +0.5
    const float r1 = drand48() - 0.5;
    //const float r2 = drand48() - 0.5;

    // make NaI energy and time spectra
    for( int i=0; i<=u->nanu; i++ ) {
        const int id = u->nai[i];
        u->na[i] = calib( (u->na[i]+r1)/2, c->gainna[id], c->shiftna[id], 0, xsize(NASP_ID));
        spec_fill( NASP_ID, u->na[i], id );
    }
    for( int i=0; i<=u->tnanu; i++) {
        const int id = u->tnai[i];
        u->tna[i]  = calib( u->tna[i]/8.0, 1, c->shifttna[id], 0, xsize(TNASP_ID));
        spec_fill( TNASP_ID, u->tna[i], id );

        t0      = u->tna[i];
        ti      = id;
    }

    // make dE and E energy spectra
    for( int i=0; i<=u->dpnu; i++ ) {
        const int id = u->dpi[i];
        u->dp[i] = calib((u->dp[i]+0)/2., c->gainde[id], c->shiftde[id], 0, xsize(DESP_ID));
        spec_fill( DESP_ID, u->dp[i], id);

        if(u->dp[i] > 45 && u->dpi[i] == 6){
            de = u->dp[i];
            dei= u->dpi[i];
        }
    }
    for( int i=0; i<=u->pnu; i++ ) {
        const int id = u->pi[i];
        u->p[i] = calib((u->p[i] + 0)/2., c->gaine[id], c->shifte[id], 0, xsize(ESP_ID));
        spec_fill( ESP_ID, u->p[i], id);

        if(u->p[i] > 77 && u->pi[i] == 1)
            e = u->p[i];
    }

    // make EDESP  and  ALFNA banana matrix
    if(de > 45 && e > 77 && dei == 6 && t0 > 15){
        de      = (int)(0.52 * ((float)de - 45.) + r1 + 0.5);
        e       = e  - 77;
        t       = t0 - (int)(dT_stop(de) + dT_start(e) +r1 + 0.5);
        ede = de + e;


        if (ti == dei + 16 &&  t0 > 0 &&  t0 <  512
            &&   t > 0 &&   t <  512
            && ede > 0 && ede < 2048
            &&  de > 0 &&  de <  512
            &&   e > 0 &&   e < 2048)
        {
            spec_fill( ALFNA_ID, e, de );
            spec_fill( EDESP_ID, ede, dei );
            spec_fill( TNASP_ID, t0, 2 );   //old LE times
            spec_fill( TNASP_ID, t, 1 );    //new E-corrected times
            if (de > 20 && de < 40 && e > 60 && e < 70 )
                spec_fill( TNASP_ID, t, 0 );   //new E-corrected times at appr 500 keV
            
            if (de > 13 && de < 17  )
                spec_fill( ALFGE_ID, e, t0 );
            
            if (e > 65 && e < 70 && t0 < 256)
                spec_fill( ALFGE_ID, de, (t0 + 256));
        }
    }

    /* ************************************************************ */
    /* Making 16 scaler read out, stored in row 9 of SINGLES matrix */
    /* Scaler value stored in chs 0-15, and increments in chs 16-31 */
    /* The scaler value (32 bits) is composed of two 16-bit words   */
    /* 16 LSB are in scaler-chs 0-15, 16 MSB are in chs 16-31       */
    /* ************************************************************ */
    for( int i=0; i<=u->scnu; i += 2 ) {
        const unsigned int scaler_hi = (unsigned int) u->sc[i];
        const unsigned int scaler_lo = (unsigned int) u->sc[i+1];
        const unsigned int scaler    = ((scaler_hi & 0x0000ffff) << 16) + scaler_lo;

        const unsigned int scaler_old = (unsigned int)spec_get(SINGLES_ID, u->sci[i]+9,0);
        spec_set( SINGLES_ID, u->sci[i] + 16, 9, scaler - scaler_old );
        spec_set( SINGLES_ID, u->sci[i] + 9,  0, scaler); // store old value
    }

    /* ************************************************************ */
    /* Reading wall-clock time and store in row 9 of SINGLES matrix */
    /* Time value stored in ch 32, and incremented time in ch 33    */
    /* The time value (32 bits) is composed of two 16-bit words     */
    /* 16 LSB are in ch 16, 16 MSB are in ch 17                     */
    /* ************************************************************ */
    for( int i = 0; i<=u->nimnu; i++ ) {
        if(u->nimi[i] == 16) {
            const unsigned int wtime_lo = (unsigned int) u->nim[i];
            const unsigned int wtime_hi = (unsigned int) u->nim[i+1];
            const unsigned int wtime    = wtime_lo + ((wtime_hi & 0x0000ffff) << 16);

            const unsigned int wtime_old = (unsigned int)spec_get(SINGLES_ID, 32, 9);
            spec_set( SINGLES_ID, 33, 9, wtime - wtime_old );
            spec_set( SINGLES_ID, 32, 9, wtime );
        }
    }

    /* **************************************** */
    /* Making Ge energy and time spectra            */
    /* bit 4 = ADC0(e), bit 5 = ADC1(t),            */
    /* bit 6 = ADC2(e), bit 7 = ADC3(t), etc        */
    /* **************************************** */
    /*
      for (i = 0; i <= nimnu; i++){
      if(u->nimi[i] > 3){
      if((u->nimi[i]/2) * 2 == u->nimi[i]){
      x               = (u->nim[i] + r1);
      u->nim[i]  = (int) (x * c->gainge[(u->nimi[i]/2) * 2] + c->shiftge[(u->nimi[i]/2) * 2] + 0.5);
      if (u->nim[i] < 0 || u->nim[i] > 4095) u->nim[i] = 0;
      (*(pgesp + u->nim[i] + ((u->nimi[i]/2) * 2) * 4096))++;
      }else{
      x               = (u->nim[i] + r1)/8;
      u->nim[i]  = (int) (x + c->shiftge[((u->nimi[i]/2) * 2)+1] + 0.5);
      if (u->nim[i] < 0 || u->nim[i] > 511) u->nim[i] = 0;
      (*(ptgesp + u->nim[i] + ((u->nimi[i]/2) * 2 + 1) * 512))++;
      }
      }
      }
    */
    return true;
}

static float dT_stop(int ch)
{
    float a = 41., b = -0.23;
    return (float)a*exp(b*(float)ch);
}

static float dT_start(int ch)
{
    float a = -150., b = -0.04;
    return (float)a*exp(b*(float)ch);
}
