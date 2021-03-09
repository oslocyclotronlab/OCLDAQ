// -*- c++ -*-

#ifndef SORT_FORMAT_H
#define SORT_FORMAT_H 1

struct calibration_t {
    float range[2048];
    
    float gaine[64],  shifte[64];
    float gainde[64], shiftde[64];
    float gainge[6],  shiftge[6];
    float gainna[32], shiftna[32];
    float gaintge[6], shifttge[6];
    float gaintna[32], shifttna[32];
    
    int ml[64], mh[64];
};

struct unpacked_t {
    int dp[64],  p[64],  na[32],  tna[32],  ge[32],   tge[32], nim[32],  cadc[32],  ctdc[32],  pu[32],  sc[32];
    int dpi[64], pi[64], nai[32], tnai[32], gei[32], tgei[32], nimi[32], cadci[32], ctdci[32], pui[32], sci[32];
    int dpnu, pnu, nanu, tnanu, genu, tgenu, nimnu, cadcnu, ctdcnu, punu, scnu;
};

//int alfa_low[64], alfa_high[64], h_low, h_high;

void unpack_next_buffer();
int unpack_next_event(const volatile unsigned int* bufp, unsigned int bufferlength, unpacked_t* unpacked);
float unpack_get_avelen();

#endif /* SORT_FORMAT_H */
