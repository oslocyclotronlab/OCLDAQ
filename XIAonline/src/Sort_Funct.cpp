
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
        case deDet:
            spec_fill(DESP_ID, entry.adcdata, dinfo.detectorNum + 8*dinfo.telNum);
            if ( entry.cfdfail )
                spec_fill(DECFD_ID, entry.adcdata, dinfo.detectorNum + 8*dinfo.telNum);
            break;
        case eDet:
            spec_fill(ESP_ID, entry.adcdata, dinfo.detectorNum);
            if ( entry.cfdfail )
                spec_fill(ECFD_ID, entry.adcdata, dinfo.detectorNum);
            break;
        case eGuard:
            spec_fill(GUARD_ID, entry.adcdata, dinfo.detectorNum);
            break;
        case ppac:
            spec_fill(PPAC_ID, entry.adcdata, dinfo.detectorNum);
            for ( auto &gamma : buffer ){
                dinfo2 = GetDetector(gamma.address);
                if ( dinfo2.type == labr ){
                    tdiff_c = entry.timestamp - gamma.timestamp;
                    tdiff_f = entry.cfdcorr - gamma.cfdcorr;
                    tdiff = tdiff_c + tdiff_f;
                    spec_fill(TPPAC_ID, tdiff + 16384, dinfo2.detectorNum);
                }
            }
            break;
        default:
            break;
        }
    }
}

void Sort_Particle_Event(Event &event)
{
    int64_t tdiff_c;
    double tdiff_f, tdiff;

    int telNo = GetDetector(event.trigger.address).detectorNum;

    // Time-energy spectrum, x-axis energy, y-axis time
    if ( !event.trigger.cfdfail && (event.trigger.adcdata > 3275) && (event.trigger.adcdata < 3450) ) {
        for (int j = 0; j < event.n_labr[1]; ++j) {
            if ( event.w_labr[1][j].cfdfail )
                continue;
            tdiff = double(event.w_labr[1][j].timestamp - event.trigger.timestamp) +
                    (event.w_labr[1][j].cfdcorr - event.trigger.cfdcorr);
            spec_fill(TIME_ENERGY_ID, event.w_labr[1][j].adcdata * double(1000) / double(16384), tdiff + 500);
        }
    }

    // We want to check if we have only one dE strip in this telescope.
    // If we see more than one, we don't continue.
    if (event.tot_dEdet_trap[telNo] == 1) {
        for (int i = 0 ; i < NUM_SI_DE_TEL ; ++i){
            for (int j = 0 ; j < event.n_dEdet[telNo][i] ; ++j){
                spec_fill(EDESP_ID, event.trigger.adcdata / 8, event.w_dEdet[telNo][i][j].adcdata / 8);

                spec_fill(EDECC_ID,
                          calibration.gain_e[telNo]*(event.trigger.adcdata + drand48() - 0.5) + calibration.shift_e[telNo],
                          calibration.gain_de[i]*(event.w_dEdet[telNo][i][j].adcdata + drand48() - 0.5) + calibration.shift_de[i]);


                // We want a spectra for dE strip 3 with E 4... Choosen at random :p
                if (telNo == 4 && i == 3)
                    spec_fill(EDESS_ID, event.trigger.adcdata / 8, event.w_dEdet[telNo][i][j].adcdata / 8);

                // We want to make time spectra.
                for (int n = 0 ; n < NUM_LABR_DETECTORS ; ++n){
                    for (int m = 0 ; m < event.n_labr[n] ; ++m){
                        tdiff_c = event.w_labr[n][m].timestamp - event.w_dEdet[telNo][i][j].timestamp;
                        tdiff_f = event.w_labr[n][m].cfdcorr - event.w_dEdet[telNo][i][j].cfdcorr;
                        tdiff = tdiff_c + tdiff_f;
                        spec_fill(TLABRSP_ID, tdiff + 16384, n);
                    }
                }
            }
        }
    }
}

void Sort_PPAC_Event(Event &event)
{

    int64_t tdiff_c;
    double tdiff_f, tdiff;

    for (int i = 0 ; i < NUM_LABR_DETECTORS ; ++i){
        for (int j = 0 ; j < event.n_labr[i] ; ++j){
            tdiff_c = event.w_labr[i][j].timestamp - event.trigger.timestamp;
            tdiff_f = event.w_labr[i][j].cfdcorr - event.trigger.cfdcorr;
            tdiff = tdiff_c + tdiff_f;
            spec_fill(TPPAC_ID, tdiff + 16384, i);
        }
    }
}

void sort_coincidence(Event &event)
{

    switch (GetDetector(event.trigger.address).type) {
    case eDet :
        Sort_Particle_Event(event);
        break;
    case labr :
        Sort_Particle_Event(event);
        break;
    case ppac:
        Sort_PPAC_Event(event);
        break;
    default:
        break;
    }

}
