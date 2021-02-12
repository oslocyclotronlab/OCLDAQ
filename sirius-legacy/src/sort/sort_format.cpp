
#include "sort_format.h"

#include <stdio.h>

inline unsigned int boe(unsigned int x) { return ((x&0xC0000000)>>28); }
inline unsigned int ndw(unsigned int x) { return  (x&0x000000ff);      }
inline unsigned int box(unsigned int x) { return ((x&0x3f800000)>>23); }
inline unsigned int chn(unsigned int x) { return ((x&0x007f0000)>>16); }
inline unsigned int dta(unsigned int x) { return  (x&0x0000ffff);      }

static const unsigned int EOB = 0x80000000;

static bool unpack_one_event(const volatile unsigned int* event_data, unsigned int n_data, unpacked_t* u)
{
    // go through all data of this event
    for( unsigned int i=1; i<=n_data; i++ ) {
        const unsigned int l_word = event_data[i];
        const int nbox = box(l_word); // the box that has fired

        if( boe(l_word) != 0 )
            // somehow a header word came into the event, maybe engine has overwritten the buffer?
            return false;
        
        if(nbox == 0x00) {        /* Pattern ch 0 - 3, NIM ADCs ch 4 - 15*/
            u->nimnu++;
            u->nimi[u->nimnu] = chn(l_word);
            u->nim [u->nimnu] = dta(l_word);
        } else if(nbox == 0x01) { /* Wall-clock time  ch 16 - 17*/
            u->nimnu++;        
            u->nimi[u->nimnu] = chn(l_word);
            u->nim [u->nimnu] = dta(l_word);
        } else if( nbox == 0x02) { /* VME scaler 1151N ch 0-31*/
            u->scnu++;
            u->sci[u->scnu]   = chn(l_word);
            u->sc [u->scnu]   = dta(l_word);
        } else if( nbox == 0x10) { /* Time of NaI ch 0-31*/
            u->tnanu++;
            u->tnai[u->tnanu] = chn(l_word);
            u->tna [u->tnanu] = dta(l_word);
        } else if( nbox == 0x11) { /* Time of Ge ch 0-31*/
            u->tgenu++;
            u->tgei[u->tgenu] = chn(l_word);
            u->tge [u->tgenu] = dta(l_word);
        } else if( nbox == 0x20       /* Energy of NaI ADC Caen    ch 0-31 */
                   || nbox == 0x24) { /* Energy of NaI ADC Mesytec ch 0-31 */
            u->nanu++;
            u->nai[u->nanu]   = chn(l_word);
            u->na [u->nanu]   = dta(l_word);
        } else if( nbox == 0x25) {     /* Energy of Ge ADC Mesytec ch 0-31 */
            u->genu++;
            u->gei[u->genu]   = chn(l_word);
            u->ge [u->genu]   = dta(l_word);
        } else if( nbox == 0x21) { /* Energy E ch 0-32 */
            u->pnu++;                        /* Only 8 E counters in */
            u->pi[u->pnu]     = chn(l_word);    /* ch 0, 2,..., 16      */
            u->p [u->pnu]     = dta(l_word);
        } else if( nbox == 0x22) { /* Energy dE1 ch 0-31 */
            u->dpnu++;
            u->dpi[u->dpnu]   = chn(l_word);
            u->dp [u->dpnu]   = dta(l_word);
        } else if( nbox == 0x23) { /* Energy dE2 ch 32-61 */
            u->dpnu++;
            u->dpi[u->dpnu]   = chn(l_word) + 32;
            u->dp [u->dpnu]   = dta(l_word);
        }
        
#if 0
        else if( nbox == 6) { /* Camac ADC1 ch 0-7*/
            u->cadcnu++;
            u->cadci[u->cadcnu] = chn(l_word);
            u->cadc[u->cadcnu]  = dta(l_word);
        } else if( nbox == 7) { /* Camac ADC2 ch 0-7*/
            u->cadcnu++;
            u->cadci[u->cadcnu] = chn(l_word) + 8;
            u->cadc[u->cadcnu]  = dta(l_word);
        } else if( nbox == 8) { /* Camac ADC3 ch 0-7*/
            u->cadcnu++;
            u->cadci[u->cadcnu] = chn(l_word) + 16;
            u->cadc[u->cadcnu]  = dta(l_word);
        } else if( nbox == 9) { /* Camac ADC4 ch 0-7*/
            u->cadcnu++;
            u->cadci[u->cadcnu] = chn(l_word) + 24;
            u->cadc[u->cadcnu]  = dta(l_word);
        } else if( nbox == 10) { /* Camac TDC1 ch 0-7*/
            u->ctdcnu++;
            u->ctdci[u->ctdcnu] = chn(l_word);
            u->ctdc[u->ctdcnu]  = dta(l_word) ;
        } else if( nbox == 11) { /* Camac TDC2 ch 0-7*/
            u->ctdcnu++;
            u->ctdci[u->ctdcnu] = chn(l_word) + 8;
            u->ctdc[u->ctdcnu]  = dta(l_word);
        } else if( nbox == 12) { /* Camac TDC3 ch 0-7*/
            u->ctdcnu++;
            u->ctdci[u->ctdcnu] = chn(l_word) + 16;
            u->ctdc[u->ctdcnu]  = dta(l_word);
        } else if( nbox == 13) { /* Camac TDC4 ch 0-7*/
            u->ctdcnu++;
            u->ctdci[u->ctdcnu] = chn(l_word) + 24;
            u->ctdc[u->ctdcnu]  = dta(l_word);
        } else if( nbox == 14) { /* Camac Pile-up ch 0-1*/
            u->punu++;
            u->pui[u->punu]   = chn(l_word);
            u->pu[u->punu]    = dta(l_word);
        }
#endif
        else {
            return false;
        }
    }
    return true;
}

static unsigned int idx = 0;
static float eventlength_sum = 0, event_count = 0;

void unpack_next_buffer()
{
    idx = 0;
    eventlength_sum = 0;
    event_count = 0;
}

int unpack_next_event(const volatile unsigned int* bufp, unsigned int bufferlength, unpacked_t* u)
{
    if( idx >= bufferlength )
        return 1; // end of buffer

    // reset numbers
    u->dpnu = u->pnu = u->nanu = u->tnanu = u->genu = u->tgenu = u->scnu = -1;
    u->nimnu = u->cadcnu = u->ctdcnu = u->punu = -1;

    const unsigned int event_header = bufp[idx];
    if( boe(event_header) != 0xC ) {
        // not begin of event; then it should be the end of the buffer
        if( event_header != EOB )
            // ouch, bad buffer
            return 2; 

        // okay, the end-of-buffer marker was there
        return 1;
    }

    const int n_data = ndw(event_header);
    eventlength_sum += n_data;
    event_count += 1;

    if( !unpack_one_event(&bufp[idx], n_data, u) )
        // unpacking error
        return 3;

    idx += n_data + 1;

    return 0; // next event might be there
}

float unpack_get_avelen()
{
    if( event_count>0 )
        return eventlength_sum / event_count;
    else
        return 0;
}
