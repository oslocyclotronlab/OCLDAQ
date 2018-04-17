
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
    int64_t tdiff_c;
    double tdiff_f, tdiff;

    for (int i = 0 ; i < NUM_SI_DE_DET ; ++i){
        for (int j = 0 ; j < event.n_dEdet[i] ; ++j){

            if (GetDetector(event.trigger.address).telNum == GetDetector(event.w_dEdet[i][j].address).telNum){

                if ( i == 23 )
                    spec_fill(EDESS_ID, event.trigger.adcdata / 8, event.w_dEdet[i][j].adcdata / 8);

                spec_fill(EDESP_ID, event.trigger.adcdata / 8, event.w_dEdet[i][j].adcdata / 8);

                for (int n = 0 ; n < NUM_LABR_DETECTORS ; ++n){
                    for (int m = 0 ; m < event.n_labr[n] ; ++m){
                        tdiff_c = event.w_labr[n][m].timestamp - event.w_dEdet[i][j].timestamp;
                        tdiff_f = event.w_labr[n][m].cfdcorr - event.w_dEdet[i][j].cfdcorr;
                        tdiff = tdiff_c + tdiff_f;
                        spec_fill(TLABRSP_ID, tdiff + 16384, n);
                    }
                }

            }
        }
    }
}
