
#include "sort_spectra.h"

#include "spec_defs.h"
#include "utilities.h"

#include <fstream>
#include <sys/mman.h>

// ######################################################################## 
// ######################################################################## 

#ifndef XIA
sort_spectrum_t sort_spectra[] = {
    {  0, 0, 0, 0, 0, 0, 0, 0 },
    { SINGLES_ID,  SINGLES_KEY, SINGLES_SIZE, 0, SINGLES_XSIZE, SINGLES_YSIZE, "SINGLES", "singles" },
    { ESP_ID,      ESP_KEY,     ESP_SIZE,     0, ESP_XSIZE,     ESP_YSIZE,     "ESP",     "E" },
    { DESP_ID,     DESP_KEY,    DESP_SIZE,    0, DESP_XSIZE,    DESP_YSIZE,    "DESP",    "DE" },
    { EDESP_ID,    EDESP_KEY,   EDESP_SIZE,   0, EDESP_XSIZE,   EDESP_YSIZE,   "EDESP",   "E-DE" },
    { THICKSP_ID,  THICKSP_KEY, THICKSP_SIZE, 0, THICKSP_XSIZE, THICKSP_YSIZE, "THICKSP", "tickness" },
    { GESP_ID,     GESP_KEY,    GESP_SIZE,    0, GESP_XSIZE,    GESP_YSIZE,    "GESP",    "Ge-energy" },
    { TGESP_ID,    TGESP_KEY,   TGESP_SIZE,   0, TGESP_XSIZE,   TGESP_YSIZE,   "TGESP",   "Ge-time" },
    { NASP_ID,     NASP_KEY,    NASP_SIZE,    0, NASP_XSIZE,    NASP_YSIZE,    "NASP",    "NaI-energy" },
    { TNASP_ID,    TNASP_KEY,   TNASP_SIZE,   0, TNASP_XSIZE,   TNASP_YSIZE,   "TNASP",   "NaI-time" },
    { ALFNA_ID,    ALFNA_KEY,   ALFNA_SIZE,   0, ALFNA_XSIZE,   ALFNA_YSIZE,   "ALFNA",   "Alpha-NaI" },
    { ALFGE_ID,    ALFGE_KEY,   ALFGE_SIZE,   0, ALFGE_XSIZE,   ALFGE_YSIZE,   "ALFGE",   "Alpha-Ge" },
    { MAT_ID,      MAT_KEY,     MAT_SIZE,     0, MAT_XSIZE,     MAT_YSIZE,     "MAT",     "general purpose" },
    {  0, 0, 0, 0, 0, 0, 0, 0 }
};
#else
sort_spectrum_t sort_spectra[] = {
    {  0, 0, 0, 0, 0, 0, 0, 0 },
    { LABRSP_ID,  LABRSP_KEY,   LABRSP_SIZE,    0,  LABRSP_XSIZE,   LABRSP_YSIZE,   "LABRSP",   "LaBr-energy" },
    { DESP_ID,    DESP_KEY,     DESP_SIZE,      0,  DESP_XSIZE,     DESP_YSIZE,     "DESP",     "DE"},
    { ESP_ID,     ESP_KEY,      ESP_SIZE,       0,  ESP_XSIZE,      ESP_YSIZE,      "ESP",      "E"},
    { PPAC_ID,    PPAC_KEY,     PPAC_SIZE,      0,  PPAC_XSIZE,     PPAC_YSIZE,     "PPACSP",   "PPAC spec"},
    { EDESP_ID,   EDESP_KEY,    EDESP_SIZE,     0,  EDESP_XSIZE,    EDESP_YSIZE,    "EDESP",    "E-DE"},
    { EDECC_ID,   EDECC_KEY,    EDECC_SIZE,     0,  EDECC_XSIZE,    EDECC_YSIZE,    "EDECC",    "E-DE calibrated"},
    { EDESS_ID,   EDESS_KEY,    EDESS_SIZE,     0,  EDESS_XSIZE,    EDESS_YSIZE,    "EDESS",    "E-DE single strip"},
    { TLABRSP_ID, TLABRSP_KEY,  TLABRSP_SIZE,   0,  TLABRSP_XSIZE,  TLABRSP_YSIZE,  "TLABRSP",  "LaBr-time"},
    { TPPAC_ID,   TPPAC_KEY,    TPPAC_SIZE,     0,  TPPAC_XSIZE,    TPPAC_YSIZE,    "TPPACS",   "t(LaBr) - t(PPAC)"},
    { LABRCSP_ID, LABRCSP_KEY,  LABRCSP_SIZE,   0,  LABRCSP_XSIZE,  LABRCSP_YSIZE,  "LABRCSP",  "LaBr-energy, calibrated"},
    { LABRCFD_ID, LABRCFD_KEY,  LABRCFD_SIZE,   0,  LABRCFD_XSIZE,  LABRCFD_YSIZE,  "LABRCFD",  "LaBr-energy (CFD fail)"},
    { DECFD_ID,   DECFD_KEY,    DECFD_SIZE,     0,  DECFD_XSIZE,    DECFD_YSIZE,    "DECFD",    "DE spectra (CFD fail)"},
    { ECFD_ID,    ECFD_KEY,     ECFD_SIZE,      0,  ECFD_XSIZE,     ECFD_YSIZE,     "ECFD",     "E spectra (CFD fail)"},
    { GUARD_ID,   GUARD_KEY,    GUARD_SIZE,     0,  GUARD_XSIZE,    GUARD_YSIZE,    "GUARD",    "E guard rings"},
    {  0, 0, 0, 0, 0, 0, 0, 0 }
};
#endif // XIA

const int NSPEC = sizeof(sort_spectra)/sizeof(sort_spectra[0]);

// ######################################################################## 

bool spectra_detach_one( int specno )
{
    if( specno<1 || specno>=NSPEC )
        return false;

    sort_spectrum_t *s = &sort_spectra[specno];
    const int ok = shmdt( s->ptr );
    s->ptr = 0;
    return ok==0;
}

// ######################################################################## 

int *spectra_attach_one( int specno, bool online )
{
    if( specno<1 || specno>=NSPEC )
        return 0;
    sort_spectrum_t *s = &sort_spectra[specno];
    key_t key = s->key;
    if( !online )
        key = s->key + 0x4000;
    s->ptr = attach_shared( key, s->size, true );
    return s->ptr;
}

// ######################################################################## 

bool spectra_attach_all(bool online)
{
    for(int i=1; sort_spectra[i].specno>0; ++i) {
        sort_spectrum_t *s = &sort_spectra[i];

        key_t key = s->key;
        if( !online )
            key = s->key + 0x4000;
        s->ptr = attach_shared( key, s->size, true );
        if( !s->ptr ) {
            spectra_detach_all();
            return false;
        }
    }
    return true;
}

// ######################################################################## 

/** Detach all attached spectra in shared memory.
 * 
 * @return true if all detachments went okay
 */
bool spectra_detach_all()
{
    bool all_okay = true;
    for(int i=1; sort_spectra[i].specno>0; ++i) {
        sort_spectrum_t *s = &sort_spectra[i];
        if( s->ptr && shmdt( s->ptr ) == -1 )
            all_okay = false;
        s->ptr = 0;
    }
    return all_okay;
}

// ######################################################################## 

sort_spectrum_t* spec_find(const char* name)
{
    for(int i=1; sort_spectra[i].specno>0; ++i) {
        sort_spectrum_t *s = &sort_spectra[i];
        if( strcmp(name, s->name) == 0 )
            return s;
    }
    return 0;
}

// ######################################################################## 

void spec_fill(int specno, int x, int y, int w)
{
    const sort_spectrum_t* s = &sort_spectra[specno];
    if( x<0 || x>=s->xdim || y<0 || y>=s->ydim ) // check range
        return;
    s->ptr[x + y*s->xdim] += w;
}

// ######################################################################## 

int spec_get(int specno, int x, int y)
{
    const sort_spectrum_t* s = &sort_spectra[specno];
    if( x<0 || x>=s->xdim || y<0 || y>=s->ydim )
        return 0;
    return s->ptr[x + y*s->xdim];
}

// ######################################################################## 

void spec_set(int specno, int x, int y, int value)
{
    const sort_spectrum_t* s = &sort_spectra[specno];
    if( x<0 || x>=s->xdim || y<0 || y>=s->ydim )
        return;
    s->ptr[x + y*s->xdim] = value;
}

