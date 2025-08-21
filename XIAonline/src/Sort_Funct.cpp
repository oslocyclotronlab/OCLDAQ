
#include "Sort_Funct.h"

#include "sort_spectra.h"
#include "experimentsetup.h"
#include "Calib.h"


#include <cstdlib>
#include <cstring>
#include <iostream>

static calibration_t calibration;

calibration_t *GetCalibration(){ return &calibration; }

constexpr double quad[] = {4.00289e-07, 1.29459e-05, 3.06429e-06, 3.86188e-06, 9.18136e-07, 7.95006e-06, 5.14033e-06, 2.35207e-06, 1.44163e-06, 5.61474e-06, 4.18093e-06, -1.93542e-07};
constexpr double gain[] = {0.622018, 0.738189, 0.597014, 0.760821, 0.694717, 0.79484, 0.711484, 0.704746, 0.667042, 0.683395, 0.696762, 0.737378};
constexpr double shift[] = {14.3494, 44.3913, 19.3134, 40.8409, 50.3031, 36.7511, 51.5865, 51.1231, 67.1972, 66.605, 40.4937, 37.883};

inline double calibrate(const int& dnum, const int& adcdata) {
    double ch = adcdata + drand48() - 0.5;
    return quad[dnum] * ch*ch  + gain[dnum] * ch + shift[dnum];
}

void sort_singles(const std::vector<word_t> &buffer)
{

    DetectorInfo_t dinfo, dinfo2;
    double tdiff_c, tdiff_f, tdiff;
    for (auto &entry : buffer){

        dinfo = GetDetector(entry.address);

        switch (dinfo.type) {
        case labr:
            spec_fill(LABRSP_ID, entry.adcdata, dinfo.detectorNum);
            spec_fill(LABRCSP_ID,calibrate(dinfo.detectorNum, entry.adcdata),dinfo.detectorNum);
            if ( entry.cfdfail )
                spec_fill(LABRCFD_ID, entry.adcdata, dinfo.detectorNum);
            break;
        default:
            break;
        }
    }

}

struct labr_evt_t {
    int n_evt;
    word_t* events[1024]{};

    labr_evt_t() : n_evt( 0 ){ for (auto & event : events) event = nullptr; }
};

void sort_coincidence(Event &event)
{
    // For now, we have nothing we will do...
    word_t* ref = nullptr;

    if ( event.n_labr[0] == 1 )
        ref = &event.w_labr[0][0];

    for ( size_t i = 0 ; i < NUM_LABR_DETECTORS ; ++i) {
        for ( size_t j = 0 ; j < event.n_labr[i] ; ++j) {
            if ( ref ) {
                auto timediff = double(event.w_labr[i][j].timestamp - ref->timestamp);
                timediff += double(event.w_labr[i][j].cfdcorr - ref->cfdcorr);

                timediff *= 10; // This is in 0.1 ns
                timediff += 32768; // Move 0 to the middle of the spectrum.

                spec_fill(TLABRSP_ID, timediff, i);
            }
        }
    }

}
