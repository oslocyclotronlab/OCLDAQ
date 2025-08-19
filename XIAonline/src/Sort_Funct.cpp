
#include "Sort_Funct.h"

#include "sort_spectra.h"
#include "experimentsetup.h"
#include "Calib.h"


#include <cstdlib>
#include <cstring>
#include <iostream>

static calibration_t calibration;

calibration_t *GetCalibration(){ return &calibration; }


static const double gain[] = {0.6288783494986332, 0.7956641719979691, 0.6172571366371751, 0.7971825061525968, 0.7245546862887766, 0.8381771858019077, 0.7594280747947241, 0.7378786582895114, 0.7121254009010106, 0.7402751489689681, 0.7323357300447191, 0.7575549130191689};
static const double shift[] = {4.016291343391087, -7.147747249360786, -5.601328532813391, 0.2708355023382007, 9.737559271633943, -3.9778044743941923, -3.874662043011627, 9.683988148425646, 4.717137462443582, -1.556264450278487, -1.137287956866454, 9.992029322586438};

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
