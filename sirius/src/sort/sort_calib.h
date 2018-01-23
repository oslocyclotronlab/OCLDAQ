// -*- c++ -*-

#ifndef SORT_CALIB_H
#define SORT_CALIB_H 1

#include <iosfwd>
#include <string>

struct calibration_t;

void reset_gainshifts(calibration_t& calib);
bool read_gainshifts(calibration_t& calib, std::istream& fp);
bool read_gainshifts(calibration_t& calib, const std::string& filename);
std::string format_gainshift(const calibration_t& calib);

void reset_telewin(calibration_t& calib);
bool read_telewin(calibration_t& calib, std::istream& fp);
bool read_telewin(calibration_t& calib, const std::string& fname);
std::string format_telewin(calibration_t& calib);

bool read_range_data(calibration_t& calib, const std::string& fname);

#endif /* SORT_CALIB_H */
