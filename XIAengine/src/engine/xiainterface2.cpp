//
// Created by Vetle Wegner Ingeberg on 02/12/2024.
//

#include "xiainterface2.h"

#include <pixie16app_export.h>

#include <iostream>
#include <cfloat>
#include <cmath>
#include <cstring>

XIAInterfaceAPI2::XIAInterfaceAPI2(const size_t &num)
    : XIAInterface( num )
{
    for ( int i = 0 ; i < GetNumModules() ; ++i ){
        auto retval = Pixie16ReadModuleInfo(i, &moduleInfo[i].revision, &moduleInfo[i].serial_number,
                                            &moduleInfo[i].adc_bits, &moduleInfo[i].adc_msps);
        if ( retval < 0 ){
            std::cerr << "*Error* (Pixie16ReadModuleInfo): Pixie16ReadModuleInfo failed, retval=" << retval << std::endl;
        }
    }
}

XIAInterface::ChanLim_t XIAInterfaceAPI2::GetChnLimits(const size_t &module, const size_t &channel, const char *ChanParName)
{
    double adcFactor;
    const double fastfilterrange = 0;
    unsigned int slowfilterrange = 0;
    if ( moduleInfo[module].adc_msps == 100 )
        adcFactor = moduleInfo[module].adc_msps;
    else if ( moduleInfo[module].adc_msps == 250 )
        adcFactor = moduleInfo[module].adc_msps / 2;
    else if ( moduleInfo[module].adc_msps == 500 )
        adcFactor = moduleInfo[module].adc_msps / 5;
    else {
        adcFactor = moduleInfo[module].adc_msps;
    }

    double QDCfactor = moduleInfo[module].adc_msps;
    if ( moduleInfo[module].adc_msps == 500 )
        QDCfactor = moduleInfo[module].adc_msps / 5;

    auto retval = Pixie16ReadSglModPar("SLOW_FILTER_RANGE", &slowfilterrange, module);
    if ( retval < 0 ){
        std::cerr << "*Error* (Pixie16ReadSglModPar): Pixie16ReadSglModPar failed, retval=" << retval << std::endl;
        return {0, 0};
    }

    if (strcmp("TRIGGER_RISETIME", ChanParName) == 0 ){
        return {0, FASTFILTER_MAX_LEN * pow(2.0, fastfilterrange) / adcFactor};
    } else if (strcmp("TRIGGER_FLATTOP", ChanParName) == 0 ){
        return {0, FASTFILTER_MAX_LEN * pow(2.0, fastfilterrange) / adcFactor};
    } else if (strcmp("TRIGGER_THRESHOLD", ChanParName) == 0 ){
        return {0, FAST_THRESHOLD_MAX};
    } else if (strcmp("ENERGY_RISETIME", ChanParName) == 0 ){
        return {0, SLOWFILTER_MAX_LEN * pow(2.0, slowfilterrange) / adcFactor};
    } else if (strcmp("ENERGY_FLATTOP", ChanParName) == 0 ){
        return {MIN_SLOWGAP_LEN * pow(2.0, slowfilterrange) / adcFactor,
                SLOWFILTER_MAX_LEN * pow(2.0, slowfilterrange) / adcFactor};
    } else if (strcmp("TAU", ChanParName) == 0 ){
        return {0, DBL_MAX};
    } else if (strcmp("TRACE_LENGTH", ChanParName) == 0 ){
        if ( moduleInfo[module].adc_msps == 500 )
            return {TRACELEN_MIN_500MHZADC / ( moduleInfo[module].adc_msps * pow(2., slowfilterrange) ), 100};
        else
            return {TRACELEN_MIN_250OR100MHZADC / ( moduleInfo[module].adc_msps * pow(2., slowfilterrange) ), 100};
    } else if (strcmp("TRACE_DELAY", ChanParName) == 0 ){
        if ( moduleInfo[module].adc_msps == 250 )
            return {0, TRACEDELAY_MAX / ( (moduleInfo[module].adc_msps / 2 ) * pow(2.0, fastfilterrange) )};
        else if ( moduleInfo[module].adc_msps == 500 )
            return {0, TRACEDELAY_MAX / ( (moduleInfo[module].adc_msps / 5 ) * pow(2.0, fastfilterrange) )};
        else
            return {0, TRACEDELAY_MAX / ( moduleInfo[module].adc_msps  * pow(2.0, fastfilterrange) )};
    } else if (strcmp("VOFFSET", ChanParName) == 0 ){
        return {-DAC_VOLTAGE_RANGE/2, DAC_VOLTAGE_RANGE/2};
    } else if (strcmp("XDT", ChanParName) == 0 ){
        return {0, DBL_MAX}; // I don't know the actual limits. The DSP/API will determine if the value is ok or not
    } else if (strcmp("BASELINE_PERCENT", ChanParName) == 0 ){
        return {1, 99}; // Percent is between 0 and 100.
    } else if (strcmp("EMIN", ChanParName) == 0 ){
        return {0, DBL_MAX};
    } else if (strcmp("BINFACTOR", ChanParName) == 0 ){
        return {1, 6};
    } else if (strcmp("BASELINE_AVERAGE", ChanParName) == 0 ){
        return {0, 16};
    } else if (strcmp("BLCUT", ChanParName) == 0 ){
        return {0, 10000};
    } else if (strcmp("FASTTRIGBACKLEN", ChanParName) == 0 ){
        if ( moduleInfo[module].adc_msps == 250 )
            return {FASTTRIGBACKLEN_MIN_125MHZFIPCLK / adcFactor, FASTTRIGBACKLEN_MAX / adcFactor};
        else
            return {FASTTRIGBACKLEN_MIN_100MHZFIPCLK / adcFactor, FASTTRIGBACKLEN_MAX / adcFactor};
    } else if (strcmp("CFDDelay", ChanParName) == 0 ){
        if ( moduleInfo[module].adc_msps == 500 ){
            double cfd_delay;
            auto r = Pixie16ReadSglChanPar("CFDDelay", &cfd_delay, module, channel); // TODO: Remove once we know what the API reports for 500 MHz.
            return {cfd_delay, cfd_delay}; // CFDDelay is fixed for 500 MHz
        } else {
            return {CFDDELAY_MIN / adcFactor, CFDDELAY_MAX / adcFactor};
        }
    } else if (strcmp("CFDScale", ChanParName) == 0 ){
        if ( moduleInfo[module].adc_msps == 500 ){
            double CFDScale;
            auto r = Pixie16ReadSglChanPar("CFDScale", &CFDScale, module, channel); // TODO: Remove once we know what the API reports for 500 MHz.
            return {CFDScale, CFDScale}; // CFDDelay is fixed for 500 MHz
        } else {
            return {0, CFDSCALE_MAX};
        }
    } else if (strcmp("CFDThresh", ChanParName) == 0 ){
        return {CFDTHRESH_MIN, CFDDELAY_MAX};
    } else if (strcmp("QDCLen0", ChanParName) == 0 ){
        return {QDCLEN_MIN / QDCfactor, QDCLEN_MAX / QDCfactor};
    } else if (strcmp("QDCLen1", ChanParName) == 0 ){
        return {QDCLEN_MIN / QDCfactor, QDCLEN_MAX / QDCfactor};
    } else if (strcmp("QDCLen2", ChanParName) == 0 ){
        return {QDCLEN_MIN / QDCfactor, QDCLEN_MAX / QDCfactor};
    } else if (strcmp("QDCLen3", ChanParName) == 0 ){
        return {QDCLEN_MIN / QDCfactor, QDCLEN_MAX / QDCfactor};
    } else if (strcmp("QDCLen4", ChanParName) == 0 ){
        return {QDCLEN_MIN / QDCfactor, QDCLEN_MAX / QDCfactor};
    } else if (strcmp("QDCLen5", ChanParName) == 0 ){
        return {QDCLEN_MIN / QDCfactor, QDCLEN_MAX / QDCfactor};
    } else if (strcmp("QDCLen6", ChanParName) == 0 ){
        return {QDCLEN_MIN / QDCfactor, QDCLEN_MAX / QDCfactor};
    } else if (strcmp("QDCLen7", ChanParName) == 0 ){
        return {QDCLEN_MIN / QDCfactor, QDCLEN_MAX / QDCfactor};
    } else if (strcmp("ExtTrigStretch", ChanParName) == 0 ){
        return {EXTTRIGSTRETCH_MIN / adcFactor, EXTTRIGSTRETCH_MAX / adcFactor};
    } else if (strcmp("VetoStretch", ChanParName) == 0 ){
        return {VETOSTRETCH_MIN / adcFactor, VETOSTRETCH_MAX / adcFactor};
    } else if (strcmp("ExternDelayLen", ChanParName) == 0 ){
        if ( moduleInfo[module].revision == 0xF ){
            return {EXTDELAYLEN_MIN / adcFactor, EXTDELAYLEN_MAX_REVF / adcFactor};
        } else {
            return {EXTDELAYLEN_MIN / adcFactor, EXTDELAYLEN_MAX_REVBCD / adcFactor};
        }
    } else if (strcmp("FtrigoutDelay", ChanParName) == 0 ){
        if ( moduleInfo[module].revision == 0xF ){
            return {FASTTRIGBACKDELAY_MIN / adcFactor, FASTTRIGBACKDELAY_MAX_REVF / adcFactor};
        } else {
            return {FASTTRIGBACKDELAY_MIN / adcFactor, FASTTRIGBACKDELAY_MAX_REVBCD / adcFactor};
        }
    } else if (strcmp("ChanTrigStretch", ChanParName) == 0 ){
        return {CHANTRIGSTRETCH_MIN / adcFactor, CHANTRIGSTRETCH_MAX / adcFactor};
    } else if (strcmp("ResetDelay", ChanParName) == 0 ){
        return {0, RESET_DELAY_MAX};
    } else {
        std::cerr << "Unknown parameter '" << ChanParName << "'" << std::endl;
        return {0, 0}; // Not sure how to handle other...
    }

}

XIAInterface::ChanPar_t XIAInterfaceAPI2::GetChnParam(const size_t &module, const size_t &channel, const char *ChanParName)
{
    XIAInterface::ChanPar_t param;
    auto retval = Pixie16ReadSglChanPar(ChanParName, &param, module, channel);
    if ( retval < 0 ){
        std::cerr << "*Error* (Pixie16ReadSglChanPar): Pixie16ReadSglChanPar failed, retval=" << retval << std::endl;
    }
    return param;
}

void XIAInterfaceAPI2::SetChnParam(const size_t &module, const size_t &channel, const char *ChanParName,
                                   const XIAInterface::ChanPar_t &parameter)
{
    auto retval = Pixie16WriteSglChanPar(ChanParName, parameter, module, channel);
    if ( retval < 0 ){
        std::cerr << "*Error* (Pixie16WriteSglChanPar): Pixie16WriteSglChanPar failed, retval=" << retval << std::endl;
    }
}

XIAInterface::ModLim_t XIAInterfaceAPI2::GetModLimits(const size_t &module, const char *ModParName)
{
    if ( strcmp(ModParName, "SLOW_FILTER_RANGE") == 0 ){
        return {SLOWFILTERRANGE_MIN, SLOWFILTERRANGE_MAX};
    } else {
        throw std::runtime_error("Module parameter does not have limits");
    }
}


XIAInterface::ModPar_t XIAInterfaceAPI2::GetModParam(const size_t &module, const char *ModParName)
{
    XIAInterface::ModPar_t param;
    auto retval = Pixie16ReadSglModPar(ModParName, &param, module);
    if ( retval < 0 ){
        std::cerr << "*Error* (Pixie16ReadSglModPar): Pixie16ReadSglModPar failed, retval=" << retval << std::endl;
    }
    return param;
}

void XIAInterfaceAPI2::SetModParam(const size_t &module, const char *ModParName,
                                   const XIAInterface::ModPar_t &parameter)
{
    auto retval = Pixie16WriteSglModPar(ModParName, parameter, module);
    if ( retval < 0 ){
        std::cerr << "*Error* (Pixie16WriteSglModPar): Pixie16ReadSglModPar failed, retval=" << retval << std::endl;
    }
}

unsigned int XIAInterfaceAPI2::MeasureBLCut(const unsigned short &module, const unsigned short &channel)
{
    unsigned int blcut = 0;
    auto retval = Pixie16BLcutFinder(module, channel, &blcut);
    if ( retval < 0 ){
        std::cerr << "*Error* (Pixie16BLcutFinder): Pixie16ReadSglModPar failed, retval=" << retval << std::endl;
    }
    return blcut;
}

void XIAInterfaceAPI2::MeasureBaseline(const unsigned short &module)
{
    auto retval = Pixie16AdjustOffsets(module);
    if ( retval < 0 ){
        std::cerr << "*Error* (Pixie16AdjustOffsets): Pixie16AdjustOffsets failed, retval=" << retval << std::endl;
    }
}

bool XIAInterfaceAPI2::WriteSettings(const char *fname)
{
    // First we need to read the entire settings file as it currently is on disk (as a backup copy)
    char filename[1024];
    FILE *file = fopen(fname, "rb");
    bool have_bck = true;
    unsigned int config_raw[N_DSP_PAR * PRESET_MAX_MODULES];
    if (fread(config_raw, sizeof(unsigned int), N_DSP_PAR*PRESET_MAX_MODULES, file) != N_DSP_PAR*PRESET_MAX_MODULES){
        std::cout << "Warning: settings.set is not valid. All current settings may be lost if saving .set file fails." << std::endl;
        have_bck = false;
    }

    fclose(file); // We are done with the file.

    std::cout << "Trying to save settings to file '" << fname << "'" << std::endl;
    snprintf(filename, 1024, "%s", fname);
    int retval = Pixie16SaveDSPParametersToFile(filename);
    if (retval == 0){
        std::cout << "... Done" << std::endl;
        return true;
    } else if (retval == -1) {
        std::cerr << "... Failed, unable to read DSP parameter value from modules. Please restart engine and check the 'Pixie16msg.txt' file." << std::endl;
        if (!have_bck){
            std::cerr << "Warning: Unable to restore old '" << filename << "' file" << std::endl;
            return false;
        }
    } else {
        std::cerr << "... Failed, unable to write to disk." << std::endl;
        if (!have_bck){
            std::cerr << "Warning: Unable to restore old '" << filename << "' file" << std::endl;
            return false;
        }
    }

    file = fopen(filename, "wb");

    // Writing to file...
    if (fwrite(config_raw, sizeof(unsigned int), N_DSP_PAR*PRESET_MAX_MODULES, file) != N_DSP_PAR*PRESET_MAX_MODULES){
        std::cerr << "Warning: Unable to restore old '" << filename << "' file. This may cause an error when restarting engine." << std::endl;
    }

    fclose(file); // Done!
    return true;
}