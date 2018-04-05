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
            spec_fill(DESP_ID, buffer[i].adcdata, dinfo.detectorNum);
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


void sort_coincidence(Event &event)
{

    // Check if only one E and one DE.
    /*if (event.tot_dEdet != 1 && event.tot_Edet != 1)
        return;

    // Fill E:DE matrix
    */
    word_t e_word, de_word;
    for (int i = 0 ; i < NUM_SI_DE_DET ; ++i){
        for (int j = 0 ; j < event.n_dEdet[i] ; ++j)
            de_word = event.w_dEdet[i][j];
    }
    for (int i = 0 ; i < NUM_SI_E_DET ; ++i){
        for (int j = 0 ; j < event.n_Edet[i] ; ++j)
            e_word = event.w_Edet[i][j];
    }

    if (event.tot_Edet != 1)
        return;

    /*if (GetDetector(de_word.address).telNum != GetDetector(e_word.address).detectorNum)
        return;

    if (GetDetector(de_word.address).detectorNum != 0)
        return;
    
    spec_fill(EDESP_ID, e_word.adcdata / 8, de_word.adcdata / 1);
    
    spec_fill(TLABRSP_ID, e_word.adcdata / 2 + de_word.adcdata / 2, 5);
    */
    // We use time of DE as start.
    
    //if (event.n_labr[0] != 1 && event.w_labr[0][0].cfdfail != 0)
    //    return;

    //word_t de_word = event.w_labr[0][0];

    int64_t tdiff_c;
    double tdiff_f, tdiff;

    for (int i = 0 ; i < NUM_LABR_DETECTORS ; ++i){
        for (int j = 0 ; j < event.n_labr[i] ; ++j){
            tdiff_c = event.w_labr[i][j].timestamp - e_word.timestamp;
            tdiff_f = event.w_labr[i][j].cfdcorr - e_word.cfdcorr;
            tdiff = tdiff_c + tdiff_f;
            //std::cout << tdiff_c << std::endl;
            spec_fill(TLABRSP_ID, tdiff + 16384, i);
        }
    }
}
