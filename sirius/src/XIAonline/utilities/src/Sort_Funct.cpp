#include "Sort_Funct.h"

#include "sort_spectra.h"
#include "experimentsetup.h"


#include <stdlib.h>
#include <string.h>


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
            spec_fill(DESP_ID, buffer[i].adcdata, dinfo.detectorNum);
            break;
        case eDet:
            spec_fill(ESP_ID, buffer[i].adcdata, dinfo.detectorNum);
            break;
        default:
            break;
        }
    }

    return;
}


void sort_coincidence(Event &event)
{

    // Check if only one E and one DE.
    //if (event.tot_dEdet != 1 && event.tot_Edet != 1)
    //    return;
/*
    // Fill E:DE matrix

    word_t e_word, de_word;
    for (int i = 0 ; i < NUM_SI_DE_DET ; ++i){
        for (int j = 0 ; j < event.n_dEdet[i] ; ++j)
            de_word = event.w_dEdet[i][j];
    }
    for (int i = 0 ; i < NUM_SI_E_DET ; ++i){
        for (int j = 0 ; j < event.n_Edet[i] ; ++j)
            e_word = event.w_Edet[i][j];
    }


    spec_fill(EDESP_ID, e_word.adcdata / 16, de_word.adcdata / 16);
    */

    // We use time of DE as start.

    if (event.n_labr[0] != 1)
        return;

    word_t de_word = event.w_labr[0][0];

    int64_t tdiff_c;
    double tdiff_f, tdiff;


    for (int i = 0 ; i < NUM_LABR_DETECTORS ; ++i){
        for (int j = 0 ; j < event.n_labr[i] ; ++j){
            tdiff_c = event.w_labr[i][j].timestamp - de_word.timestamp;
            tdiff_f = event.w_labr[i][j].cfdcorr - de_word.cfdcorr;
            tdiff = tdiff_c + tdiff_f;
            spec_fill(TLABRSP_ID, 1000*tdiff + 16384, i);
        }
    }
}
