//
// Created by Vetle Wegner Ingeberg on 11/02/2021.
//

#ifndef PIXIEENGINE_XIAMODULE_H
#define PIXIEENGINE_XIAMODULE_H

#include "FirmwareMap.h"
#include "XIAParser.h"

#include <Pixie16/pixie16app_defs.h>

#include <vector>

#include <cstdint>

// An XIA event has a header corresponding to up to 18 words times 32 bit/4 bytes



class XIAModule
{

public:
    struct module_info_t {
        unsigned short rev;
        unsigned int serno;
        unsigned short bits;
        unsigned short mhz;

        explicit operator FirmwareMap_t::key_t(){ return {rev, bits, mhz}; }
    };

private:

    //! Module number
    unsigned short ModNum;

    //! Store the results of module info fields.
    module_info_t info;

    //! Store the Parser object
    XIAParser parser;

    //! An internal buffer for overflows (maybe not needed??)
    size_t buf_size;

    //! Buffer for input data. Declared here to live on stack rather than in heap
    uint32_t lmdata[EXTERNAL_FIFO_LENGTH+XIA_HEADER_MAX_LEN];

public:

    explicit XIAModule(const unsigned short &num);

    //! Get module info
    inline module_info_t GetInfo() const { return info; }

    //! Boot module with firmware files
    int BootModule(const FirmwareMap_t::value_t &fw, const char *settings);

    //! Adjust baseline
    int AdjustBaselineOffset(bool keep_BLCut = false);

    //! Adjust BLCut
    int AdjustBLCut(unsigned short ch = NUMBER_OF_CHANNELS);

    //! Reset readout
    void Reset();

    //! Start list mode run
    int StartLMR(unsigned short mode = NEW_RUN);

    //! Check run status of module
    int GetRunStatus() const;

    //! End run
    int EndRun();

    //! Get the total number of bytes of data avalible on the module.
    size_t GetAvalible();

    //! Get a vector with complete events from module.
    std::vector<XIAEvent> ReadEvents();

    //! Flush buffers
    std::vector<XIAEvent> Flush();


};


#endif //PIXIEENGINE_XIAMODULE_H
