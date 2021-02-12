#include "Functions.h"

#include "pixie16app_export.h"
#include "pixie16app_defs.h"


int AdjustBaselineOffset(unsigned short modNo)
{
    // First we "back up" all the baseline cut values for the module.

    double tmp;
    unsigned int baselineCut[PRESET_MAX_MODULES][NUMBER_OF_CHANNELS];

    for (int module = 0 ; module < modNo ; ++module){
        for (int channel = 0 ; channel < NUMBER_OF_CHANNELS ; ++channel){
            Pixie16ReadSglChanPar(const_cast<char *>("BLCUT"), &tmp, module, channel);
            baselineCut[module][channel] = tmp;
        }
    }

    // Find new DC baseline offset.
    int retval = Pixie16AdjustOffsets(modNo);

    // Reset the baseline cut values.
    for (int module = 0 ; module < modNo ; ++module){
        for (int channel = 0 ; channel < NUMBER_OF_CHANNELS ; ++channel){
            Pixie16WriteSglChanPar(const_cast<char *>("BLCUT"), baselineCut[module][channel], module, channel);
        }
    }

    return retval;
}
