
#include "Sort_Funct.h"

#include "sort_spectra.h"
#include "experimentsetup.h"
#include "Calib.h"


#include <cstdlib>
#include <cstring>
#include <iostream>

static calibration_t calibration;

calibration_t *GetCalibration(){ return &calibration; }


static const double gain[] = {0.532786, 0.593626, 0.606949, 0.483961, 0.32795, 0.800924, 0.306407, 0.335072, 0.264426, 0.411669, 0.325721, 0.403113};
static const double shift[] = {54.1865, 13.2793, 13.2547, 24.4136, 97.8915, 54.1035, 18.9814, 82.467, 51.2308, 44.687, 44.8648, 60.4771};

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
                      gain[dinfo.detectorNum]*(entry.adcdata + drand48() - 0.5) + shift[dinfo.detectorNum],
                      //calibration.gain_labr[dinfo.detectorNum]*(entry.adcdata + drand48() - 0.5) + calibration.shift_labr[dinfo.detectorNum],
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
