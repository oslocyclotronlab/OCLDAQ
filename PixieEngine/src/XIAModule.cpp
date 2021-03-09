//
// Created by Vetle Wegner Ingeberg on 11/02/2021.
//

#include "XIAModule.h"

#include <Pixie16/pixie16app_export.h>

#include <string>
#include <cstring>

#define MIN_READOUT 4


XIAModule::XIAModule(const unsigned short &num)
    : ModNum( num ), parser( 10 ), buf_size( 0 )
{
    auto res = Pixie16ReadModuleInfo(ModNum, &info.rev, &info.serno, &info.bits, &info.mhz);
    if ( res < 0 || !FirmwareMap_t::key_t(info).is_valid() ){
        std::string errmsg = "*ERROR* Pixie16ReadModuleInfo failed, module=" + std::to_string(ModNum);
        errmsg += ", retval = " + std::to_string(res);
        throw std::runtime_error(errmsg);
    }
    if ( info.mhz == 100 || info.mhz == 500 )
        parser = XIAParser( 10 );
    else if ( info.mhz == 250 )
        parser = XIAParser( 8 );
    else
        throw std::runtime_error("Unknown ADC MHz");
    memset(lmdata, 0, sizeof(uint32_t)*(EXTERNAL_FIFO_LENGTH+XIA_HEADER_MAX_LEN));
}

int XIAModule::BootModule(const FirmwareMap_t::value_t &fw, const char *settings)
{
    return Pixie16BootModule(fw.comFPGA.c_str(), fw.SPFPGA.c_str(), "",
                             fw.DSPcode.c_str(), settings, fw.DSPvar.c_str(), ModNum, 0x7F);
}

int XIAModule::AdjustBaselineOffset(bool keep_BLCut)
{
    unsigned int baselineCut[NUMBER_OF_CHANNELS];
    if ( keep_BLCut ){
        for ( unsigned short ch = 0 ; ch < NUMBER_OF_CHANNELS ; ++ch ){
            if ( Pixie16ReadSglChanPar(const_cast<char *>("BLCUT"), reinterpret_cast<double *>(baselineCut+ch), ModNum, ch) ){
                throw std::runtime_error("Unable to read BLcut");
            }
        }
    }

    auto res = Pixie16AdjustOffsets(ModNum);
    if ( res < 0 ){
        return res;
    }

    if ( keep_BLCut ){
        for ( unsigned short ch = 0 ; ch < NUMBER_OF_CHANNELS ; ++ch ){
            if ( Pixie16WriteSglChanPar(const_cast<char *>("BLCUT"), baselineCut[ch], ModNum, ch) ){
                throw std::runtime_error("Unable to write BLcut");
            }
        }
    }
    return res;
}

int XIAModule::AdjustBLCut(unsigned short ch)
{

}

void XIAModule::Reset()
{
    buf_size = 0;
    memset(lmdata, 0, sizeof(uint32_t)*(EXTERNAL_FIFO_LENGTH+XIA_HEADER_MAX_LEN));
}

int XIAModule::StartLMR(unsigned short mode)
{
    if ( mode == NEW_RUN )
        Reset();
    return Pixie16StartListModeRun(ModNum, LIST_MODE_RUN, mode);
}

int XIAModule::GetRunStatus() const
{
    return Pixie16CheckRunStatus(ModNum);
}

int XIAModule::EndRun()
{
    return Pixie16EndRun(ModNum);
}

size_t XIAModule::GetAvalible()
{
    unsigned int nFIFO;
    auto res = Pixie16CheckExternalFIFOStatus(&nFIFO, ModNum);
    if ( res < 0 ){
        std::string errmsg = "*ERROR* Pixie16CheckExternalFIFOStatus failed, retval = ";
        errmsg += std::to_string(res);
        throw std::runtime_error(errmsg);
    }
    return nFIFO;
}

std::vector<XIAEvent> XIAModule::ReadEvents()
{
    // First step is to ensure that we have enough data to read out from module.
    unsigned int size;
    auto res = Pixie16CheckExternalFIFOStatus(&size, ModNum);
    if ( res < 0 ){
        std::string errmsg = "*ERROR* Pixie16CheckExternalFIFOStatus failed, retval = ";
        errmsg += std::to_string(res);
        throw std::runtime_error(errmsg);
    }

    if ( size > EXTERNAL_FIFO_LENGTH ){
        std::string errmsg = "*ERROR* FIFO size larger than maximum";
        throw std::runtime_error(errmsg);
    }

    if ( size < MIN_READOUT )
        return std::vector<XIAEvent>();


    res = Pixie16ReadDataFromExternalFIFO(lmdata+buf_size, size, ModNum);
    if ( res < 0 ){
        std::string errmsg = "*ERROR* Pixie16ReadDataFromExternalFIFO failed, retval = ";
        errmsg += std::to_string(res);
        throw std::runtime_error(errmsg);
    }

    return parser.Parse(lmdata, size+buf_size, buf_size);
}