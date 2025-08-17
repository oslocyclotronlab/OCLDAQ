
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


void sort_coincidence(Event &event)
{

}
