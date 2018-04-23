
#include "Sort_Funct.h"

#include "sort_spectra.h"
#include "experimentsetup.h"


#include <stdlib.h>
#include <string.h>
#include <iostream>


void sort_singles(std::vector<word_t> buffer)
{

    DetectorInfo_t dinfo;
    for (size_t i = 0 ; i < buffer.size() ; ++i){

        dinfo = GetDetector(buffer[i].address);

        switch (dinfo.type) {
        case labr:
            spec_fill(LABRSP_ID, buffer[i].adcdata, dinfo.detectorNum);
            break;
        case deDet:
            spec_fill(DESP_ID, buffer[i].adcdata, dinfo.detectorNum + 8*dinfo.telNum);
            break;
        case eDet:
            spec_fill(ESP_ID, buffer[i].adcdata, dinfo.detectorNum);
            break;
        case ppac:
            spec_fill(PPAC_ID, buffer[i].adcdata, dinfo.detectorNum);
            break;
        default:
            break;
        }
    }

    return;
}

void Sort_Particle_Event(Event &event)
{
    int64_t tdiff_c;
    double tdiff_f, tdiff;

    int telNo = GetDetector(event.trigger.address).telNum;

    // We want to check if we have only one dE strip in this telescope.
    // If we see more than one, we don't continue.
    if (event.tot_dEdet_trap[telNo] == 1) {
        for (int i = 0 ; i < NUM_SI_DE_TEL ; ++i){
            for (int j = 0 ; j < event.n_dEdet[telNo][i] ; ++j){
                spec_fill(EDESP_ID, event.trigger.adcdata / 8, event.w_dEdet[telNo][i][j].adcdata / 8);

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
    case ppac:
        Sort_PPAC_Event(event);
        break;
    default:
        break;
    }

}
