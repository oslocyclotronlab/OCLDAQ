// -*- c++ -*-

#ifndef USER_UTILITIES_H
#define USER_UTILITIES_H 1

#include <TH1.h>
#include <TH2.h>

#include <string>

typedef class TH1* TH1p;
typedef class TH2* TH2p;

TH1p Spec( const std::string& name, const std::string& title,
           int channels, double left, double right, const std::string& xtitle );

TH2p Mat( const std::string& name, const std::string& title,
          int ch1, double l1, double r1, const std::string& xtitle, 
          int ch2, double l2, double r2, const std::string& ytitle);

// ########################################################################

struct nai_data_t {
    float t, e;
    int id;
};

struct unpacked_t;
int unpack_nai(unpacked_t* u, nai_data_t* nai, int* idx);

#endif /* USER_UTILITIES_H */
