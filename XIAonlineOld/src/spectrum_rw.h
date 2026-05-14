// -*- c++ -*-

#ifndef SPECTRUM_RW_H
#define SPECTRUM_RW_H 1

#include "sort_spectra.h"

#include <iosfwd>
#include <string>

int norw1dim(std::ostream& fp, const std::string& comment, const int xdim,
             const float *ax, float cal[6]);

int norw2dim(std::ostream& fp, const std::string& comment, const int xdim, const int ydim,
             const float *mx, float cal[6]);
int norw2dim(std::ostream& fp, const std::string& comment, const int xdim, const int ydim,
             const int* mx, float cal[6]);

int norr1dim(std::istream& fp, std::string& comment, int& xdim,
             float* ax, float cal[6]);

int norr2dim(std::istream& fp, std::string& comment, int& xdim, int& ydim,
             float* mx, float cal[6]);
int norr2dim(std::istream& fp, std::string& comment, int& xdim, int& ydim,
             int* mx, float cal[6]);

bool dump_spectrum(const sort_spectrum_t* s, float* cal=0, const char* filename=0);

#endif /* SPECTRUM_RW_H */
