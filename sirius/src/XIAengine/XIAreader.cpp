#include "XIAreader.h"

#include "globals.h"
#include "event_container.h"
#include "DataMerger.h"

// Standard library C++
#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <thread>

// Linux (Unix?) C header
#include <sys/time.h>

// XIA headers
#include "pixie16app_export.h"
#include "pixie16sys_export.h"


// First we need to set up all settings for the
// XIA modules.
static unsigned short num_modules; //!< Number of modules used.
static unsigned short PXISlotMap[PRESET_MAX_MODULES];  //!< PXI mapping of the modules
static char comFPGAfile[PRESET_MAX_MODULES][1024];
static char SPFPGAfile[PRESET_MAX_MODULES][1024];
static char TrigFPGAfile[PRESET_MAX_MODULES][1024];
static char DSPCodeFile[PRESET_MAX_MODULES][1024];
static char DSPParFile[PRESET_MAX_MODULES][1024];
static char DSPVarFile[PRESET_MAX_MODULES][1024];

static char errmsg[1024];

// Private vectors for temporary storage of raw data when events are split into two
// fifos from the XIA cards.
std::vector<uint32_t> overflow[PRESET_MAX_MODULES];


// Flag that will be raised if an error was encoutered while interacting with the
// Pixie-16 API.
static bool settings_complete = false;
static bool error_pixie = false;


// Private function declarations
bool ReadWriteStats();
bool DoReadout();
bool CheckFIFOStats(unsigned int MinReadout);
void ErrorShutdown();
bool StopRun();
bool StartRun();
bool BootXIA();
void ParseAndMove(uint32_t *raw_data, int size, int module);



#define CHECK_IF_STILL_OPEN if (!infile.is_open()){ std::cerr << "Error, configuration file is to short." << std::endl; settings_complete = false; return false; }

bool ReadSettingsFile(const char *cfgfile)
{
    std::ifstream infile(cfgfile);

    if (!infile.is_open()){ // Check that we are able to read the infile.
        std::cerr << "Error, could not open configuration file." << std::endl;
        settings_complete = false;
        return false;
    }

    infile >> num_modules; // Read number of modules from file.

    if (num_modules < 0 && num_modules > PRESET_MAX_MODULES){
        std::cerr << "Error, number of modules does not make any sence, num_modules = " << num_modules << std::endl;
        settings_complete = false;
        return false;
    }

    for (int i = 0 ; i < num_modules ; ++i){
        CHECK_IF_STILL_OPEN
        infile >> PXISlotMap[i];
    }

    for (int i = 0 ; i < num_modules ; ++i){
        CHECK_IF_STILL_OPEN
        infile >> comFPGAfile[i];
        CHECK_IF_STILL_OPEN
        infile >> SPFPGAfile[i];
        CHECK_IF_STILL_OPEN
        infile >> TrigFPGAfile[i];
        CHECK_IF_STILL_OPEN
        infile >> DSPCodeFile[i];
        CHECK_IF_STILL_OPEN
        infile >> DSPParFile[i];
        CHECK_IF_STILL_OPEN
        infile >> DSPVarFile[i];
    }
    settings_complete = true;
    return true;
}

bool BootXIA()
{

    int retval;

    // This is the first step in
    // staring a run. The errorflag
    // will be set to false as no error
    // have yet occured!
    error_pixie = false;

    if (!settings_complete)
        return false;

    // Initilizing
    retval = Pixie16InitSystem(num_modules, PXISlotMap, 0);
    termWrite.Write("Initilizing...");
    if (retval < 0){
        sprintf(errmsg, "*ERROR* Pixie16InitSystem failed, retval = %d\n", retval);
        error_pixie = true;
        termWrite.WriteError(errmsg);
        Pixie_Print_MSG(errmsg);
        return false;
    }
    termWrite.Write("OK\n");

    // Booting all modules
    for (unsigned int i = 0 ; i < num_modules ; ++i){
        sprintf(errmsg, "Booting module %d ...", i);
        termWrite.Write(errmsg);
        retval = Pixie16BootModule(comFPGAfile[i], SPFPGAfile[i], TrigFPGAfile[i],
                                   DSPCodeFile[i], DSPParFile[i], DSPVarFile[i], i, 0x7F);
        if (retval < 0){
            sprintf(errmsg, "*ERROR* Pixie16BootModule failed, retval = %d\n", retval);
            error_pixie = true;
            termWrite.WriteError(errmsg);
            Pixie_Print_MSG(errmsg);
            return false;
        } else {
            sprintf(errmsg, "Pixie16BootModule succeeded, retval = %d\n", retval);
            Pixie_Print_MSG(errmsg);
        }
        sprintf(errmsg, "OK\n");
        termWrite.Write(errmsg);
    }

    // Adjusting offset for the baseline.
    termWrite.Write("Adjusting baselines...");
    retval = Pixie16AdjustOffsets(num_modules);
    if (retval < 0){
        sprintf(errmsg, "*ERROR* Pixie16AdjustOffsets failed, retval = %d\n", retval);
        Pixie_Print_MSG(errmsg);
        termWrite.Write(" Failed\n");
        error_pixie = true;
        return false;
    }
    termWrite.Write(" OK\n");


    // Adjusting the baseline cut
    for (int i = 0 ; i < num_modules ; ++i){
        sprintf(errmsg, "Adjusting BLcut of module %d\n", i);
        termWrite.Write(errmsg);
        unsigned int blcut;
        for (int j = 0 ; j < num_modules ; ++j){
            sprintf(errmsg, "Channel %d: ", j);
            termWrite.Write(errmsg);
            retval = Pixie16BLcutFinder(i, j, &blcut);
            if (retval < 0){
                sprintf(errmsg, "*ERROR* Pixie16BLcutFinder failed, retval = %d\n", retval);
                Pixie_Print_MSG(errmsg);
                sprintf(errmsg, "Failed, retval = %d\n", retval);
                termWrite.Write(errmsg);
            } else {
                sprintf(errmsg, " OK, set to %d\n", blcut);
                termWrite.Write(errmsg);
            }
        }
    }

    // Synchronizing the modules.
    for (int i = 0 ; i < num_modules ; ++i){

        sprintf(errmsg, "Writing SYNCH_WAIT to module %d", i);
        termWrite.Write(errmsg);
        retval = Pixie16WriteSglModPar("SYNCH_WAIT", 1, i);
        if (retval < 0){
            sprintf(errmsg, "*ERROR* Pixie16WriteSglModPar SYNCH_WAIT failed, retval = %d", retval);
            Pixie_Print_MSG(errmsg);
            termWrite.Write("... Failed\n");
        } else {
            termWrite.Write(" ... OK\n");
        }

        sprintf(errmsg, "Writing IN_SYNCH to module %d", i);
        termWrite.Write(errmsg);
        retval = Pixie16WriteSglModPar("SYNCH_WAIT", 0, i);
        if (retval < 0){
            sprintf(errmsg, "*ERROR* Pixie16WriteSglModPar IN_SYNCH failed, retval = %d", retval);
            Pixie_Print_MSG(errmsg);
            termWrite.Write("... Failed\n");
        } else {
            termWrite.Write(" ... OK\n");
        }
    }

    return true;
}

bool StartRun()
{
    int retval = Pixie16StartListModeRun(num_modules, 0x100, NEW_RUN);

    if (retval < 0){
        sprintf(errmsg, "*ERROR* Pixie16StartListModeRun failed, retval = %d", retval);
        Pixie_Print_MSG(errmsg);
    }

    return true;
}

bool StopRun()
{
    int retval = Pixie16EndRun(0);
    if (retval < 0){
        sprintf(errmsg, "*ERROR* Pixie16EndRun failed, retval = %d", retval);
        Pixie_Print_MSG(errmsg);
        return false;
    }

    std::this_thread::sleep_for(std::chrono::seconds(2));

    if (!DoReadout()){
        termWrite.WriteError("Error reading the data from FIFO after shutdown\n");
        return false;
    }

    return true;
}

void ErrorShutdown()
{
    int retval = Pixie16ExitSystem(num_modules);
    if (retval < 0){
        sprintf(errmsg, "*ERROR* Pixie16ExitSystem failed, retval = %d", retval);
        Pixie_Print_MSG(errmsg);
    }
    termWrite.Write("Shutdown finished\n");
    return;
}


bool CheckFIFOStats(unsigned int MinReadout)
{
    unsigned int numFIFOwords;
    int retval;

    for (int i = 0 ; i < num_modules ; ++i){
        retval = Pixie16CheckExternalFIFOStatus(&numFIFOwords, i);
        if (retval == -1){
            sprintf(errmsg, "*ERROR* Pixie16CheckExternalFIFOStatus failed, retval = %d\n", i);
            termWrite.WriteError(errmsg);
            Pixie_Print_MSG(errmsg);
            error_pixie = true;
            return false;
        }

        if (numFIFOwords >= MinReadout)
            return true;
    }

    return false;
}

bool DoReadout()
{
    uint32_t *FIFOdata[PRESET_MAX_MODULES];
    unsigned int fifoSize[PRESET_MAX_MODULES];
    int retval;
    for (int i = 0 ; i < num_modules ; ++i){
        retval = Pixie16CheckExternalFIFOStatus(&fifoSize[i], i);
        if (retval == -1){
            sprintf(errmsg, "*ERROR* Pixie16CheckExternalFIFOStatus failed, retval = %d\n", i);
            termWrite.WriteError(errmsg);
            Pixie_Print_MSG(errmsg);
            error_pixie = true;
            return false;
        }

        FIFOdata[i] = new uint32_t[fifoSize[i]];
        retval = Pixie16ReadDataFromExternalFIFO(FIFOdata[i], fifoSize[i], i);
        if (retval == -1){
            sprintf(errmsg, "*ERROR* Pixie16ReadDataFromExternalFIFO failed, retval = %d\n", i);
            termWrite.WriteError(errmsg);
            Pixie_Print_MSG(errmsg);
            error_pixie = true;
            return false;
        }
    }

    for (int i = 0 ; i < num_modules ; ++i){
        ParseAndMove(FIFOdata[i], fifoSize[i], i);
        delete[] FIFOdata[i];
    }

    return true;
}

bool ReadWriteStats()
{
    return true;
}

// This function will be the main loop
// that readout will be running.
void StartReadout(unsigned int MinReadout)
{

    bool ReadyForReadout = false;
    stopXIA = BootXIA();
    stopXIA = (stopXIA && !StartRun());


    struct timeval tim;
    double t1=0, t2=0;


    while (!stopXIA){

        // Check if data is ready for readout.
        ReadyForReadout = CheckFIFOStats(MinReadout);

        if (error_pixie){
            ErrorShutdown();
            stopXIA = true;
            return;
        }

        if (ReadyForReadout){
            if (!DoReadout()){
                ErrorShutdown();
                stopXIA=true;
                return;
            }
            ReadyForReadout = false;
        }

        // Update time and read out
        gettimeofday(&tim, NULL);
        t2 = tim.tv_sec + (tim.tv_sec * 1e-6);

        if (t2 - t1 > 5){
            if (!ReadWriteStats()){
                ErrorShutdown();
                stopXIA=true;
                return;
            }
            t1 = t2;
        }
    }

    StopRun();
    ErrorShutdown();

    return;
}


void ParseAndMove(uint32_t *raw_data, int size, int module)
{
    int event_length, header_length;
    int current_position;
    int64_t tlow, thigh;
    Event_t evt;

    std::vector<Event_t> result;

    if (overflow[module].size() > 0){
        event_length = (overflow[module][0] & 0x3FFE0000) >> 17;
        header_length = (overflow[module][0] & 0x0001F000) >> 12;

        if (event_length - overflow[module].size() > size) { // Event spans several FIFOs :O
            for (int i = 0 ; i < size ; ++i){
                overflow[module].push_back(raw_data[i]);
            }
            return;
        }

        uint32_t *tmp = new uint32_t[event_length];
        for (size_t i = 0 ; i < overflow[module].size() ; ++i){
            tmp[i] = overflow[module][i];
        }

        for (size_t i = 0 ; i < event_length - overflow[module].size() ; ++i){
            tmp[i+overflow[module].size()] = raw_data[i];
        }

        tlow = tmp[1];
        thigh = (tmp[2] & 0x0000FFFF);
        evt.timestamp = thigh << 32;
        evt.timestamp |= tlow;

        for (int i = 0 ; i < header_length ; ++i){
            evt.raw_data[i] = tmp[i];
        }
        evt.size_raw = header_length;
        delete[] tmp;

        // Striping away the list mode data.
        if (event_length != header_length){
            uint32_t new_f_head = header_length << 17;
            new_f_head |= (evt.raw_data[0] & 0x8001FFFF);
            evt.raw_data[0] = new_f_head;
            evt.raw_data[3] = (evt.raw_data[3] & 0x0000FFFF);
        }
        result.push_back(evt);
        current_position = event_length - overflow[module].size();
        overflow[module].clear();
    }

    while (current_position < size){
        event_length = (raw_data[current_position] & 0x3FFE0000) >> 17;
        header_length = (raw_data[current_position] & 0x0001F000) >> 12;

        if (current_position + event_length > size){
            for (int i = current_position ; i < size ; ++i){
                overflow[module].push_back(raw_data[i]);
            }
            break;
        }

        for (int i = 0 ; i < header_length ; ++i){
            evt.raw_data[i] = raw_data[current_position+i];
        }
        evt.size_raw = header_length;

        // Remove trace
        if (event_length != header_length){
            uint32_t new_f_head = header_length << 17;
            new_f_head |= (evt.raw_data[0] & 0x8001FFFF);
            evt.raw_data[0] = new_f_head;
            evt.raw_data[3] = (evt.raw_data[3] & 0x0000FFFF);
        }
        result.push_back(evt);
        current_position += event_length;
    }

    merger.AddData(result);
}
