//
// Created by Vetle Wegner Ingeberg on 01/02/2021.
//

#include "Calib.h"

#include <fstream>
#include <iostream>
#include <sstream>

void reset_gainshifts(calibration_t& calib)
{
    for (int i = 0 ; i < 32 ; ++i){
        calib.gain_labr[i] = 1.0;
        calib.shift_labr[i] = 0.0;
    }

    for ( int i = 0 ; i < 64 ; ++i ){
        calib.gain_de[i] = 1.0;
        calib.shift_de[i] = 0.0;
    }

    for ( int i = 0 ; i < 8 ; ++i ){
        calib.gain_e[i] = 1.0;
        calib.shift_e[i] = 0.0;
    }
}


bool read_gainshifts(calibration_t& calib, std::istream& fp)
{
    reset_gainshifts(calib);

    // read gain and shift values from stream
    if( !fp )
        return false;
    int j = 0;
    for(double & i : calib.gain_labr) {
        fp >> i;
        //std::cout << "Reading gain #" << j++ << ": " << i << std::endl;
    }
    j = 0;
    for(double & i : calib.gain_de){
        fp >> i;
        //std::cout << "Reading gain #" << j++ << ": " << i << std::endl;
    }
    j = 0;
    for(double & i : calib.gain_e){
        fp >> i;
        //std::cout << "Reading gain #" << j++ << ": " << i << std::endl;
    }

    j = 0;
    for(double & i : calib.shift_labr){
        fp >> i;
        //std::cout << "Reading gain #" << j++ << ": " << i << std::endl;
    }
    j = 0;
    for(double & i : calib.shift_de) {
        fp >> i;
        //std::cout << "Reading gain #" << j++ << ": " << i << std::endl;
    }
    j = 0;
    for(double & i : calib.shift_e){
        fp >> i;
        //std::cout << "Reading gain #" << j++ << ": " << i << std::endl;
    }


    return bool(fp);
}

bool read_gainshifts(calibration_t& calib, const std::string& filename)
{
    std::ifstream fp(filename.c_str());
    return read_gainshifts(calib, fp);
}

// ########################################################################

std::string format_gainshift(const calibration_t& calib)
{
    std::ostringstream o;

    for(double i : calib.gain_labr)
        o << i << ' ';
    for(double i : calib.gain_de)
        o << i << ' ';
    for(double i : calib.gain_e)
        o << i << ' ';

    for(double i : calib.shift_labr)
        o << i << ' ';
    for(double i : calib.shift_de)
        o << i << ' ';
    for(double i : calib.shift_e)
        o << i << ' ';

    return o.str();
}