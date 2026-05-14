//
// Created by Vetle Wegner Ingeberg on 01/02/2021.
//

#ifndef OCLDAQ_CALIB_H
#define OCLDAQ_CALIB_H

#include <iosfwd>
#include <string>

struct calibration_t
{
    double gain_labr[32], shift_labr[32];
    double gain_de[64], shift_de[64];
    double gain_e[8], shift_e[8];
};

void reset_gainshifts(calibration_t& calib);
bool read_gainshifts(calibration_t& calib, std::istream& fp);
bool read_gainshifts(calibration_t& calib, const std::string& filename);
std::string format_gainshift(const calibration_t& calib);

#endif //OCLDAQ_CALIB_H