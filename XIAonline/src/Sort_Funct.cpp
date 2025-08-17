
#include "Sort_Funct.h"

#include "sort_spectra.h"
#include "experimentsetup.h"
#include "Calib.h"


#include <cstdlib>
#include <cstring>
#include <iostream>

static calibration_t calibration;

calibration_t *GetCalibration(){ return &calibration; }

void sort_singles(const std::vector<word_t> &buffer)
{

    DetectorInfo_t dinfo, dinfo2;
    double tdiff_c, tdiff_f, tdiff;
    for (auto &entry : buffer){

        dinfo = GetDetector(entry.address);

        switch (dinfo.type) {
        case labr:
            spec_fill(LABRSP_ID, entry.adcdata, dinfo.detectorNum);
            spec_fill(LABRCSP_ID,
                      calibration.gain_labr[dinfo.detectorNum]*(entry.adcdata + drand48() - 0.5) + calibration.shift_labr[dinfo.detectorNum],
                      dinfo.detectorNum);
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
            auto timediff = double(event.w_labr[i][j].timestamp - ref->timestamp);
            timediff += double(event.w_labr[i][j].cfdcorr - ref->cfdcorr);

            timediff *= 10; // This is in 0.1 ns
            timediff += 32768; // Move 0 to the middle of the spectrum.

            spec_fill(TLABRSP_ID, timediff, i);
        }
    }

}
