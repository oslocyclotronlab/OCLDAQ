// -*- c++ -*-

#ifndef SORT_SPECTRA_H
#define SORT_SPECTRA_H 1

#include <sys/shm.h>

#define XIA 1

#ifndef XIA
enum {
    SINGLES_ID =  1,
    ESP_ID     =  2,
    DESP_ID    =  3,
    EDESP_ID   =  4,
    THICKSP_ID =  5,
    GESP_ID    =  6,
    TGESP_ID   =  7,
    NASP_ID    =  8,
    TNASP_ID   =  9,
    ALFNA_ID   = 10,
    ALFGE_ID   = 11,
    MAT_ID     = 12
};
#else
enum {
    LABRSP_ID   =   1,
    DESP_ID     =   2,
    ESP_ID      =   3,
    PPAC_ID     =   4,
    EDESP_ID    =   5,
    EDECC_ID    =   6,
    EDESS_ID    =   7,
    TLABRSP_ID  =   8,
    TPPAC_ID    =   9,
    LABRCSP_ID  =  10,
    LABRCFD_ID  =  11,
    DECFD_ID    =  12,
    ECFD_ID     =  13,
    GUARD_ID    =  14,
    TIME_ENERGY_ID = 15
};
#endif // XIA

struct sort_spectrum_t {
    int specno;
    key_t  key;
    size_t size;
    int* ptr;
    int xdim, ydim;
    const char* name;
    const char* description;
};

extern sort_spectrum_t sort_spectra[];

bool spectra_attach_all(bool online);
bool spectra_detach_all();

sort_spectrum_t* get_raw();
sort_spectrum_t* spec_find(const char* name);

void spec_fill(int specno, int x, int y, int w=1);
int spec_get(int specno, int x, int y);
void spec_set(int specno, int x, int y, int value);

#endif /* SORT_SPECTRA_H */
