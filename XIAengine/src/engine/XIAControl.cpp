#include "XIAControl.h"

#include "WriteTerminal.h"
#include "utilities.h"
#include "functions.h"

#include <algorithm>
#include <thread>
#include <chrono>
#include <fstream>
#include <iostream>
#include <sstream>

#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <sys/time.h>

#include <pixie16app_export.h>
#include <pixie16sys_export.h>

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))


// Define some addresses...
#define LIVETIMEA_ADDRESS 0x0004a37f
#define LIVETIMEB_ADDRESS 0x0004a38f
#define FASTPEAKSA_ADDRESS 0x0004a39f
#define FASTPEAKSB_ADDRESS 0x0004a3af
#define RUNTIMEA_ADDRESS 0x0004a342
#define RUNTIMEB_ADDRESS 0x0004a343
#define CHANEVENTSA_ADDRESS 0x0004a41f
#define CHANEVENTSB_ADDRESS 0x0004a42f

bool next_line(std::istream &in, std::string &line)
{
    line = "";

    std::string tmp;
    while ( std::getline(in, tmp) ){
        size_t ls = tmp.size();
        if ( ls == 0 ){
            break;
        } else if ( tmp[ls-1] != '\\' ){
            line += tmp;
            break;
        } else {
            line += tmp.substr(0, ls-1);
        }
    }
    return in || !line.empty();
}



XIAControl::XIAControl(WriteTerminal *writeTerm,
                       const unsigned short PXImap[PRESET_MAX_MODULES],
                       const std::string &FWname,
                       const std::string &SETname)
    : termWrite( writeTerm )
    , data_avalible( 0 )
    , is_initialized( false )
    , is_booted( false )
    , is_running( false )
    , settings_file( SETname )
{
    ReadConfigFile(FWname.c_str());
    num_modules = 0;
    for (int i = 0 ; i < PRESET_MAX_MODULES ; ++i){
        if (PXImap[i] > 0)
            PXISlotMap[num_modules++] = PXImap[i];
    }

    lmdata = (unsigned int *)malloc(sizeof(unsigned int) * EXTERNAL_FIFO_LENGTH);
}

XIAControl::~XIAControl()
{
    free(lmdata);
    ExitXIA();
}


bool XIAControl::XIA_check_buffer(int bufsize)
{
    // Check that we are actually running.
    if (!is_running)
        return false;
    double t1=last_time.tv_sec + 1e-6*last_time.tv_usec;
    double t2;
    if (CheckFIFO(XIA_FIFO_MIN_READOUT)){

        if ( !ReadFIFO() )
            StopRun();
    }
    timeval tmp;
    gettimeofday(&tmp, NULL);
    t2 = tmp.tv_sec + 1e-6*tmp.tv_usec;
    if (t2 - t1 > 1.0 ){
        if (!WriteScalers())
            StopRun();
        last_time = tmp;
    }

    // Here we decide if we have one or more buffers for the engine to process.
    int have_data = data_avalible + overflow_queue.size();
    have_data -= XIA_MIN_READOUT;
    if ( have_data < bufsize) // First test to determine if we have enough data to make an actual buffer.
        return false;
    return true;
}



bool XIAControl::XIA_fetch_buffer(uint32_t *buffer, int bufsize, unsigned int *first_header)
{
    int current_pos = 0;
    int have_data = data_avalible + overflow_queue.size();
    have_data -= XIA_MIN_READOUT;

    // This function should NEVER be called unless we have
    // enough data. However, in the case it happends.
    if ( have_data < bufsize ){
        return false;
    }

    for (auto &i : overflow_queue){
        buffer[current_pos++] = i;
    }
    overflow_queue.clear();
    Event_t current_word;

    *first_header = current_pos;

    while (current_pos < bufsize){
        current_word = sorted_events.top();
        data_avalible -= current_word.size_raw;
        sorted_events.pop();

        for (int i = 0 ; i < current_word.size_raw ; ++i){
            if (current_pos < bufsize){
                buffer[current_pos++] = current_word.raw_data[i];
            } else {
                overflow_queue.push_back(current_word.raw_data[i]);
            }
        }
    }
    return true;
}

bool XIAControl::XIA_boot_all(const bool &offline)
{
    if (is_running)
        return true;

    // Check if initialized, if not, initialize.
    if (!is_initialized)
        is_initialized = InitializeXIA(offline);

    // Check that we successfully initialized.
    if ( !is_initialized )
        return is_initialized; // We failed :'(

    // Check if the module is booted.
    if ( !is_booted )
        is_booted = BootXIA();

    // Check that we successfully booted.
    if ( !is_booted )
        return is_booted;

    // Now we need to adjust the baseline.
    AdjustBaseline();

    // Done booting :D
    return true;
}

bool XIAControl::XIA_start_run()
{
    // First we will check if there is a run currently going.
    // If so, return true?
    if (is_running){
        return true;
    }

    // Check if the modules are initialized.
    if (!is_initialized){
        is_initialized = InitializeXIA();
    }

    // Check again. If we got false, then we return false.
    if (!is_initialized)
        return false;

    // Check if modules are booted. If not, boot them
    if (!is_booted){
        is_booted = BootXIA();
    }

    // Check that the module was in fact booted
    if (!is_booted)
        return false;

    for (int i = 0 ; i < num_modules ; ++i){
        most_recent_t[i] = 0;
    }

    // Adjust baseline.
    //AdjustBaseline(); // At the moment, this isn't a critical error. We can keep on running.
    //AdjustBlCut();

    // We will wait a second before moving on.
    termWrite->Write("Sleeping for 1 second");
    std::this_thread::sleep_for(std::chrono::seconds(1));
    termWrite->Write("... Awake again\n");

    // Write synch to the modules.
    //if (!SynchModules()) // We won't start running unless all modules are in synch and ready for action...
    //    return false;

    // Now we start the list mode for realz!
    is_running = StartLMR();

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    return is_running; // OMG!!!
}

bool XIAControl::XIA_check_status()
{
    is_running = CheckIsRunning();
    return is_running;
}

bool XIAControl::XIA_end_run(FILE *output_file, const char *fname)
{

    // Check if run is ongoing. If not, return true.
    if (!is_running)
        return true;

    // Now we will end the list mode run.
    is_running = !StopRun();

    // We will wait for 0.5 seconds to make sure that all data
    // has been processed by the XIA DSP/FPGA.
    termWrite->Write("Sleeping for 0.5 seconds...");
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    termWrite->Write("... Awake again.\n");

    // Then we will run a readout of the FIFO.
    if ( !ReadFIFO() ) // We had an error. I'm not sure how this will be fixed, if fixed...
        return true;

    // Allocate memory where we will dump the FIFO contents.
    unsigned int current_pos = 0;
    unsigned int size = overflow_queue.size() + data_avalible;
    auto *buf = new uint32_t[size];
    for (auto &i : overflow_queue)
        buf[current_pos++] = overflow_queue[i];
    overflow_queue.clear();
    Event_t evt;
    while (current_pos < size){
        evt = sorted_events.top();
        for (int i = 0 ; i < evt.size_raw ; ++i){
            buf[current_pos++] = evt.raw_data[i];
        }
        data_avalible = data_avalible - evt.size_raw;
        sorted_events.pop();
    }

    // Write to disk
    if ( output_file ){
        if ( fwrite(buf, sizeof(uint32_t), size, output_file) != size ){
            termWrite->WriteError("Error while writing to file...\n");
        }

        // Lastly we will save run statistics from each module
        unsigned int run_statistics[448];
        std::string outname = std::string(fname) + ".stats";
        auto stat_file = fopen(outname.c_str(), "w");
        for ( int mod = 0 ; mod < num_modules ; ++mod ){
            auto retval = Pixie16ReadStatisticsFromModule(run_statistics, mod);
            if ( retval < 0 ){
                std::cerr << "*Error* (Pixie16SaveHistogramToFile): Pixie16ReadHistogramFromFile failed, retval=";
                std::cerr << retval << std::endl;
            }
            fprintf(stat_file, "mod %d: %u", mod, run_statistics[0]);
            for ( int i = 1 ; i < 448 ; ++i ){
                fprintf(stat_file, ", %u", run_statistics[i]);
            }
            fprintf(stat_file, "\n");
        }
        fclose(stat_file);
    }
    delete[] buf;

    return true;
}

bool XIAControl::XIA_reload()
{
    // Check if run is active.
    is_running = CheckIsRunning();
    if ( is_running )
        return false;

    // Exit
    is_initialized = ExitXIA();
    is_booted = is_initialized;
    return !is_initialized;
}


bool XIAControl::ReadConfigFile(const char *config)
{
    // We expect the file to have the following setup.
    /*
     * # - Indicates a comment
     * \\ - Indicates that the input continues on the next line
     * "comFPGAConfigFile_Rev<R>_<S>MHz_<B>Bit = /path/to/com/syspixie16_xx.bin" - R: Revision, S: ADC freqency and B: ADC bits
     * "SPFPGAConfigFile_Rev<R>_<S>MHz_<B>Bit = /path/to/SPFPGA/fippixie16_xx.bin" - R: Revision, S: ADC freqency and B: ADC bits
     * "DSPCodeFile_Rev<R>_<S>MHz_<B>Bit = /path/to/DSPCode/Pixie16DSP_xx.ldr" - R: Revision, S: ADC freqency and B: ADC bits
     * "DSPVarFile_Rev<R>_<S>MHz_<B>Bit = /path/to/DSPVar/Pixie16DSP_xx.var" - R: Revision, S: ADC freqency and B: ADC bits
     */

    std::ifstream input(config);
    std::string line;

    std::map<std::string, std::string> fw;
    termWrite->Write("Reading firmware file... \n");

    if ( !input.is_open() ){
        termWrite->WriteError("Error: Couldn't read firmware config. file\n");
        return false;
    }

    while ( next_line(input, line) ){
        if ( line.empty() || line[0] == '#' )
            continue; // Ignore empty lines or comments.

        // Search for "=" sign on the line.
        std::string::size_type pos_eq = line.find('=');

        // If not found, write a warning and continue to next line.
        if ( pos_eq == std::string::npos ){
            snprintf(errmsg, sizeof(errmsg), "Could not understand line '%s', continuing...\n", line.c_str());
            termWrite->WriteError(errmsg);
            continue;
        }

        std::string key = strip(line.substr(0, pos_eq));
        std::string val = strip(line.substr(pos_eq+1));

        // If the key have already been entered.
        if ( fw.find(key) != fw.end() ){
            snprintf(errmsg, sizeof(errmsg), "Mutiple definitions of '%s'\n", key.c_str());
            termWrite->WriteError(errmsg);
            return false;
        }

        fw[key] = val;
    }
    termWrite->Write("Done reading firmware files\n");
    firmwares.swap(fw);
    return true;
}

bool XIAControl::InitializeXIA(const bool &offline)
{
    int retval = Pixie16InitSystem(num_modules, PXISlotMap, offline ? 1 : 0);

    if (retval < 0){
        snprintf(errmsg, sizeof(errmsg), "*ERROR* Pixie16InitSystem failed, retval = %d\n", retval);
        termWrite->WriteError(errmsg);
        Pixie_Print_MSG(errmsg);
        return false;
    }
    return true;
}

bool XIAControl::GetFirmwareFile(const unsigned short &revision, const unsigned short &ADCbits, const unsigned short &ADCMSPS,
                                 char *ComFPGA, char *SPFPGA, char *DSPcode, char *DSPVar)
{
    std::string key_Com, key_SPFPGA, key_DSPcode, key_DSPVar;

    // First, if Rev 11, 12 or 13.
    if ( (revision == 11 || revision == 12 || revision == 13) ){

        // We set the keys.
        key_Com = "comFPGAConfigFile_RevBCD";
        key_SPFPGA = "SPFPGAConfigFile_RevBCD";
        key_DSPcode = "DSPCodeFile_RevBCD";
        key_DSPVar = "DSPVarFile_RevBCD";

    } else if ( revision == 15 ){

        key_Com = "comFPGAConfigFile_RevF_" + std::to_string(ADCMSPS) + "MHz_" + std::to_string(ADCbits) + "Bit";
        key_SPFPGA = "SPFPGAConfigFile_RevF_" + std::to_string(ADCMSPS) + "MHz_" + std::to_string(ADCbits) + "Bit";
        key_DSPcode = "DSPCodeFile_RevF_" + std::to_string(ADCMSPS) + "MHz_" + std::to_string(ADCbits) + "Bit";
        key_DSPVar = "DSPVarFile_RevF_" + std::to_string(ADCMSPS) + "MHz_" + std::to_string(ADCbits) + "Bit";

    } else {
        snprintf(errmsg, sizeof(errmsg), "Unknown Pixie-16 revision, rev=%d\n", revision);
        termWrite->WriteError(errmsg);
        return false;
    }

    // Search our map for the firmware files.
    if ( firmwares.find(key_Com) == firmwares.end() ){
        snprintf(errmsg, sizeof(errmsg), "Missing firmware file '%s'\n", key_Com.c_str());
        termWrite->WriteError(errmsg);
        return false;
    }

    if ( firmwares.find(key_SPFPGA) == firmwares.end() ){
        snprintf(errmsg, sizeof(errmsg), "Missing firmware file '%s'\n", key_SPFPGA.c_str());
        termWrite->WriteError(errmsg);
        return false;
    }

    if ( firmwares.find(key_DSPcode) == firmwares.end() ){
        snprintf(errmsg, sizeof(errmsg), "Missing firmware file '%s'\n", key_DSPcode.c_str());
        termWrite->WriteError(errmsg);
        return false;
    }

    if ( firmwares.find(key_DSPVar) == firmwares.end() ){
        snprintf(errmsg, sizeof(errmsg), "Missing firmware file '%s'\n", key_DSPVar.c_str());
        termWrite->WriteError(errmsg);
        return false;
    }

    // If we reach this point, we know that we have all the firmwares!
    strcpy(ComFPGA, firmwares[key_Com].c_str());
    strcpy(SPFPGA, firmwares[key_SPFPGA].c_str());
    strcpy(DSPcode, firmwares[key_DSPcode].c_str());
    strcpy(DSPVar, firmwares[key_DSPVar].c_str());

    return true;
}

bool XIAControl::BootXIA()
{
    int retval;
    char ComFPGA[2048], SPFPGA[2048], DSPCode[2048], DSPVar[2048];
    char TrigFPGA[] = "trig";
    char DSPSet[2048];
    strcpy(DSPSet, settings_file.c_str());

    unsigned short rev[PRESET_MAX_MODULES], bit[PRESET_MAX_MODULES], MHz[PRESET_MAX_MODULES];
    unsigned int sn[PRESET_MAX_MODULES];


    termWrite->Write("Reading hardware information\n");
    for (int i = 0 ; i < num_modules ; ++i){
        retval = Pixie16ReadModuleInfo(i, &rev[i], &sn[i], &bit[i], &MHz[i]);
        if (retval < 0){
            snprintf(errmsg, sizeof(errmsg), "*ERROR* Pixie16ReadModuleInfo failed, retval = %d\n", retval);
            termWrite->WriteError(errmsg);
            Pixie_Print_MSG(errmsg);
            return false;
        }
    }

    for (int i = 0 ; i < num_modules ; ++i){
        if (!GetFirmwareFile(rev[i], bit[i], MHz[i],
                             ComFPGA, SPFPGA,
                             DSPCode, DSPVar)) {
            snprintf(errmsg, sizeof(errmsg), "Module %d: Unknown module\n", i);
            termWrite->Write(errmsg);
            return false;
        }

        switch (MHz[i]) {
        case 100:
            timestamp_factor[i] = 10;
            break;
        case 250:
            timestamp_factor[i] = 8;
            break;
        case 500:
            timestamp_factor[i] = 10;
            break;
        default:
            timestamp_factor[i] = 10;
            break;
        }


        snprintf(errmsg, sizeof(errmsg), "Booting Pixie-16 module #%d, Rev=%d, S/N=%d, Bits=%d, MSPS=%d\n", i, rev[i], sn[i], bit[i], MHz[i]);
        termWrite->Write(errmsg);
        termWrite->Write("ComFPGAConfigFile:\t");
        termWrite->Write(ComFPGA);
        termWrite->Write("\nSPFPGAConfigFile:\t");
        termWrite->Write(SPFPGA);
        termWrite->Write("\nDSPCodeFile:\t");
        termWrite->Write(DSPCode);
        termWrite->Write("\nDSPVarFile:\t\t");
        termWrite->Write(DSPVar);
        termWrite->Write("\n");
        retval = Pixie16BootModule(ComFPGA, SPFPGA, TrigFPGA, DSPCode, DSPSet, DSPVar, i, 0x7F);
        termWrite->Write("\n----------------------------------------\n\n");
        
        if (retval < 0){
            snprintf(errmsg, sizeof(errmsg), "*ERROR* Pixie16BootModule failed, retval = %d\n", retval);
            termWrite->WriteError(errmsg);
            Pixie_Print_MSG(errmsg);
            return false;
        }
    }

    termWrite->Write("All modules booted.\n");
    termWrite->Write("DSPParFile:\t");
    termWrite->Write(DSPSet);
    termWrite->Write("\n");
    return true;
}

bool XIAControl::AdjustBaseline()
{
    termWrite->Write("Adjusting baseline of all modules and channels...");
    int retval = AdjustBaselineOffset(num_modules);
    if (retval < 0){
        snprintf(errmsg, sizeof(errmsg), "*ERROR* Pixie16AdjustOffsets failed, retval = %d\n", retval);
        termWrite->Write("\n");
        termWrite->WriteError(errmsg);
        Pixie_Print_MSG(errmsg);
        return false;
    }

    termWrite->Write("\n... Done.\n");
    return true;
}

bool XIAControl::AdjustBlCut()
{
    termWrite->Write("Acquiring the baseline cut...");
    unsigned int BLcut[PRESET_MAX_MODULES][16];
    int retval;
    for (int i = 0 ; i < num_modules ; ++i){
        for (int j = 0 ; j < 16 ; ++j){
            retval = Pixie16BLcutFinder(i, j, &BLcut[i][j]);
            if (retval < 0){
                snprintf(errmsg, sizeof(errmsg), "*ERROR* Pixie16BLcutFinder for mod = %d, ch = %d failed, retval = %d\n", i, j, retval);
                termWrite->Write("\n");
                termWrite->WriteError(errmsg);
                Pixie_Print_MSG(errmsg);
                return false;
            }
        }
    }

    termWrite->Write("\n... Done.\n");
    termWrite->Write("Module:");
    for (int i = 0 ; i < 16 ; ++i){
        snprintf(errmsg, sizeof(errmsg), "\tCh. %d:",i);
        termWrite->Write(errmsg);
    }
    termWrite->Write("\n");

    for (int i = 0 ; i < num_modules ; ++i){
        snprintf(errmsg, sizeof(errmsg), "%d:", i);
        termWrite->Write(errmsg);
        for (int j = 0 ; j < 16 ; ++j){
            snprintf(errmsg, sizeof(errmsg), "\t%d", BLcut[i][j]);
            termWrite->Write(errmsg);
        }
        termWrite->Write("\n");
    }

    return true;
}


bool XIAControl::StartLMR()
{
    int retval;

    // First we check if the modules have already been synchronized.
    // if so, we don't need to reset the clocks (could be annoying if there are random offsets each new run!)
    #ifdef CHECK_SYNCH
    unsigned int synch_val[PRESET_MAX_MODULES];
    bool is_synch = true;
    for (unsigned short i = 0 ; i < num_modules ; ++i){
        retval = Pixie16ReadSglModPar(const_cast<char *>("IN_SYNCH"), &synch_val[i], i);
        if (retval < 0){
            sprintf(errmsg, "*ERROR* Pixie16ReadSglModPar reading IN_SYNCH failed, retval = %d\n", retval);
            termWrite->WriteError(errmsg);
            Pixie_Print_MSG(errmsg);
            return false;
        }

        if (synch_val[i] != 0)
            is_synch = false;

    }
    #endif // CHECK_SYNCH


    // If we are not synchronized, then we reset the clock.
    #ifdef CHECK_SYNCH
    if (!is_synch){
    #endif // CHECK_SYNCH
        termWrite->Write("Trying to write IN_SYNCH...\n");
        retval = Pixie16WriteSglModPar(const_cast<char *>("IN_SYNCH"), 0, 0);
        if (retval < 0){
            snprintf(errmsg, sizeof(errmsg), "*ERROR* Pixie16WriteSglModPar writing IN_SYNCH failed, retval = %d\n", retval);
            termWrite->WriteError(errmsg);
            Pixie_Print_MSG(errmsg);
            return false;
        }
        termWrite->Write("... Done.\n");
    
    #ifdef CHECK_SYNCH
    }
    #endif // CHECK_SYNCH


    termWrite->Write("Trying to write SYNCH_WAIT...\n");
    retval = Pixie16WriteSglModPar(const_cast<char *>("SYNCH_WAIT"), 1, 0);
    if (retval < 0){
        snprintf(errmsg, sizeof(errmsg), "*ERROR* Pixie16WriteSglModPar writing SYNCH_WAIT failed, retval = %d\n", retval);
        termWrite->WriteError(errmsg);
        Pixie_Print_MSG(errmsg);
        return false;
    }
    termWrite->Write("... Done.\n");

    termWrite->Write("About to start list mode run...\n");
    retval = Pixie16StartListModeRun(num_modules, 0x100, NEW_RUN);
    if (retval < 0){
        snprintf(errmsg, sizeof(errmsg), "*ERROR* Pixie16StartListModeRun failed, retval = %d\n", retval);
        termWrite->WriteError(errmsg);
        Pixie_Print_MSG(errmsg);
        return false;
    }
    termWrite->Write("List mode started OK\n");
    return true;
}

bool XIAControl::CheckIsRunning()
{
    bool am_I_running = true;
    int retval;
    for (int i = 0 ; i < num_modules ; ++i){
        retval = Pixie16CheckRunStatus(i);
        if (retval < 0){
            snprintf(errmsg, sizeof(errmsg), "*ERROR* Pixie16CheckRunStatus failed, retval = %d\n", retval);
            termWrite->WriteError(errmsg);
            Pixie_Print_MSG(errmsg);
        }
        am_I_running = (am_I_running && retval);
    }

    return am_I_running;
}


bool XIAControl::StopRun()
{
    int retval;

    // In principle, we should only need to do this for one of
    // the modules. However we will try to stop all of them.
    // Most will probably get retval=0 which means it has stopped.
    for (int i = 0 ; i < num_modules ; ++i){
        // First we check that the module are in fact running.
        retval = Pixie16CheckRunStatus(i);
        if (retval < 0){
            snprintf(errmsg, sizeof(errmsg), "*ERROR* Pixie16CheckRunStatus failed, retval = %d\n", retval);
            termWrite->WriteError(errmsg);
            Pixie_Print_MSG(errmsg);
        } else if (retval > 0){
            retval = Pixie16EndRun(i);
            if (retval < 0){
                snprintf(errmsg, sizeof(errmsg), "*ERROR* Pixie16EndRun failed, retval = %d\n", retval);
                termWrite->WriteError(errmsg);
                Pixie_Print_MSG(errmsg);
                return false;
            }
        }
    }
    return true;
}


bool XIAControl::SynchModules()
{
    termWrite->Write("Trying to write IN_SYNCH...\n");
    int retval = Pixie16WriteSglModPar(const_cast<char *>("IN_SYNCH"), 0, 0);
    if (retval < 0){
        snprintf(errmsg, sizeof(errmsg), "*ERROR* Pixie16WriteSglModPar writing IN_SYNCH failed, retval = %d\n", retval);
        termWrite->WriteError(errmsg);
        Pixie_Print_MSG(errmsg);
        return false;
    }
    termWrite->Write("... Done.\n");

    return true;
}

bool XIAControl::WriteScalers()
{
    double ICR[PRESET_MAX_MODULES][16], OCR[PRESET_MAX_MODULES][16];
    unsigned int stats[448];
    int retval;
    {
        for (int i = 0 ; i < num_modules ; ++i){
            retval = Pixie16ReadStatisticsFromModule(stats, i);
            if (retval < 0){
                snprintf(errmsg, sizeof(errmsg), "*ERROR* Pixie16ReadStatisticsFromModule failed, retval = %d\n", retval);
                termWrite->WriteError(errmsg);
                Pixie_Print_MSG(errmsg);
            }

            for (int j = 0 ; j < 16 ; ++j){
                
                uint64_t fastPeakN = stats[FASTPEAKSA_ADDRESS + j - DATA_MEMORY_ADDRESS - DSP_IO_BORDER];
                fastPeakN = fastPeakN << 32;
                fastPeakN += stats[FASTPEAKSB_ADDRESS + j - DATA_MEMORY_ADDRESS - DSP_IO_BORDER];

                uint64_t fastPeakP = last_stats[i][FASTPEAKSA_ADDRESS + j - DATA_MEMORY_ADDRESS - DSP_IO_BORDER];
                fastPeakP = fastPeakP << 32;
                fastPeakP += last_stats[i][FASTPEAKSB_ADDRESS + j - DATA_MEMORY_ADDRESS - DSP_IO_BORDER];

                double fastPeak = fastPeakN - fastPeakP;

                uint64_t LiveTimeN = stats[LIVETIMEA_ADDRESS + j - DATA_MEMORY_ADDRESS - DSP_IO_BORDER];
                LiveTimeN = LiveTimeN << 32;
                LiveTimeN |= stats[LIVETIMEB_ADDRESS + j - DATA_MEMORY_ADDRESS - DSP_IO_BORDER];

                uint64_t LiveTimeP = last_stats[i][LIVETIMEA_ADDRESS + j - DATA_MEMORY_ADDRESS - DSP_IO_BORDER];
                LiveTimeP = LiveTimeP << 32;
                LiveTimeP |= last_stats[i][LIVETIMEB_ADDRESS + j - DATA_MEMORY_ADDRESS - DSP_IO_BORDER];

                double liveTime = LiveTimeN - LiveTimeP;
                if (timestamp_factor[i] == 8)
                    liveTime *= 2e-6/250.;
                else
                    liveTime *= 1e-6/100.;

                uint64_t ChanEventsN = stats[CHANEVENTSA_ADDRESS + j - DATA_MEMORY_ADDRESS - DSP_IO_BORDER];
                ChanEventsN = ChanEventsN << 32;
                ChanEventsN |= stats[CHANEVENTSB_ADDRESS + j - DATA_MEMORY_ADDRESS - DSP_IO_BORDER];

                uint64_t ChanEventsP = last_stats[i][CHANEVENTSA_ADDRESS + j - DATA_MEMORY_ADDRESS - DSP_IO_BORDER];
                ChanEventsP = ChanEventsP << 32;
                ChanEventsP |= last_stats[i][CHANEVENTSB_ADDRESS + j - DATA_MEMORY_ADDRESS - DSP_IO_BORDER];
                
                double ChanEvents = ChanEventsN - ChanEventsP;

                uint64_t runTimeN = stats[RUNTIMEA_ADDRESS - DATA_MEMORY_ADDRESS - DSP_IO_BORDER];
                runTimeN = runTimeN << 32;
                runTimeN |= stats[RUNTIMEB_ADDRESS - DATA_MEMORY_ADDRESS - DSP_IO_BORDER];

                uint64_t runTimeP = last_stats[i][RUNTIMEA_ADDRESS - DATA_MEMORY_ADDRESS - DSP_IO_BORDER];
                runTimeP = runTimeP << 32;
                runTimeP |= last_stats[i][RUNTIMEB_ADDRESS - DATA_MEMORY_ADDRESS - DSP_IO_BORDER];

                double runTime = runTimeN - runTimeP;

                runTime *= 1.0e-6 / 100.;

                ICR[i][j] = (liveTime !=0) ? fastPeak/liveTime : 0;
                OCR[i][j] = (runTime != 0) ? ChanEvents/runTime : 0;
            }

            for (int j = 0 ; j < 448 ; ++j){
                last_stats[i][j] = stats[j];
            }
        }
    }

    FILE *scaler_file_in = fopen(SCALER_FILE_NAME_IN, "w");
    FILE *scaler_file_out = fopen(SCALER_FILE_NAME_OUT, "w");

    fprintf(scaler_file_in, "Input count rate:\n\n\n");
    fprintf(scaler_file_out, "Output count rate:\n\n\n");

    std::stringstream csv_stream;
    csv_stream << "module,channel,input,output\n";

    for (int i = 0 ; i < 16 ; ++i){
        fprintf(scaler_file_in, "\t%d", i);
        fprintf(scaler_file_out, "\t%d", i);
    }
    fprintf(scaler_file_in, "\n");
    fprintf(scaler_file_out, "\n");

    for (int i = 0 ; i < num_modules ; ++i){
        fprintf(scaler_file_in, "%d", i);
        fprintf(scaler_file_out, "%d", i);
        for (int j = 0 ; j < 16 ; ++j){
            fprintf(scaler_file_in, "\t%.2f", ICR[i][j]);
            fprintf(scaler_file_out, "\t%.2f", OCR[i][j]);
            csv_stream << i << "," << j << "," << ICR[i][j] << "," << OCR[i][j] << "\n";
        }
        fprintf(scaler_file_in, "\n");
        fprintf(scaler_file_out, "\n");
    }

    fclose(scaler_file_in);
    fclose(scaler_file_out);

    {
        std::ofstream csv_file(SCALER_FILE_CSV);
        csv_file << csv_stream.str();
    }

    return true;
}


bool XIAControl::CheckFIFO(unsigned int minReadout)
{
    unsigned int numFIFOwords;
    int retval;

    for (int i = 0 ; i < num_modules ; ++i){
        retval = Pixie16CheckExternalFIFOStatus(&numFIFOwords, i);
        if (retval < 0){
            snprintf(errmsg, sizeof(errmsg), "*ERROR* Pixie16CheckExternalFIFOStatus failed, retval = %d\n", i);
            termWrite->WriteError(errmsg);
            Pixie_Print_MSG(errmsg);
            return false;
        }
        if (numFIFOwords < minReadout)
            continue;
        else
            return true;
    }

    return false;
}


bool XIAControl::ReadFIFO()
{
    uint32_t *FIFOdata = lmdata;
    unsigned int fifoSize;
    int retval;
    for (int i = 0 ; i < num_modules ; ++i){
        retval = Pixie16CheckExternalFIFOStatus(&fifoSize, i);
        if (retval < 0){
            snprintf(errmsg, sizeof(errmsg), "*ERROR* Pixie16CheckExternalFIFOStatus failed, retval = %d\n", i);
            termWrite->WriteError(errmsg);
            Pixie_Print_MSG(errmsg);
            return false;
        }
        if (fifoSize < 4 /* EXTFIFO_READ_THRESH */ ) // Make sure we don't read from an empty FIFO.
            continue;
        retval = Pixie16ReadDataFromExternalFIFO(FIFOdata, fifoSize, i);
        if (retval < 0){
            snprintf(errmsg, sizeof(errmsg), "*ERROR* Pixie16ReadDataFromExternalFIFO failed, retval = %d\n", retval);
            termWrite->WriteError(errmsg);
            Pixie_Print_MSG(errmsg);
            return false;
        }
        ParseQueue(FIFOdata, fifoSize, i);
    }

    return true;
}

bool XIAControl::ExitXIA()
{
    // Check that there are no runs currently going on.
    is_running = CheckIsRunning();

    if (is_running){
        // End the current run.
        is_running = !StopRun();
    }
    int retval = Pixie16ExitSystem(num_modules);
    if (retval < 0){
        snprintf(errmsg, sizeof(errmsg), "*ERROR* Pixie16ExitSystem failed, retval = %d\n", retval);
        termWrite->Write(errmsg);
        return false;
    }
    return true;
}


void XIAControl::ParseQueue(uint32_t *raw_data, size_t size, int module)
{
    int event_length=0, header_length=0;
    int current_position=0;
    int64_t tlow, thigh;
    Event_t evt;

    if (overflow_fifo[module].size() > 0){
        event_length = (overflow_fifo[module][0] & 0x7FFE0000) >> 17;
        header_length = (overflow_fifo[module][0] & 0x1F000) >> 12;
        int evtsize = event_length - overflow_fifo[module].size();
        if (evtsize > size) { // Event spans several FIFOs :O
            for (int i = 0 ; i < size ; ++i){
                overflow_fifo[module].push_back(raw_data[i]);
            }
            return;
        }

        uint32_t *tmp = new uint32_t[event_length];
        for (size_t i = 0 ; i < overflow_fifo[module].size() ; ++i){
            tmp[i] = overflow_fifo[module][i];
        }

        for (size_t i = 0 ; i < event_length - overflow_fifo[module].size() ; ++i){
            tmp[i+overflow_fifo[module].size()] = raw_data[i];
        }

        tlow = tmp[1];
        thigh = (tmp[2] & 0x0000FFFF);
        evt.timestamp = thigh << 32;
        evt.timestamp |= tlow;
        evt.timestamp *= timestamp_factor[module];

        for (int i = 0 ; i < header_length ; ++i){
            evt.raw_data[i] = tmp[i];
        }
        evt.size_raw = event_length;
        delete[] tmp;

        sorted_events.push(evt);
        most_recent_t[module] = MAX(most_recent_t[module], evt.timestamp);
        data_avalible += evt.size_raw;

        current_position = event_length - overflow_fifo[module].size();
        overflow_fifo[module].clear();
    }

    while (current_position < size){
        event_length = (raw_data[current_position] & 0x7FFE0000) >> 17;
        header_length = (raw_data[current_position] & 0x1F000 ) >> 12;

        if (current_position + event_length > size){
            for (int i = current_position ; i < size ; ++i){
                overflow_fifo[module].push_back(raw_data[i]);
            }
            break;
        }

        for (int i = 0 ; i < header_length ; ++i){
            evt.raw_data[i] = raw_data[current_position+i];
        }
        evt.size_raw = event_length;

        tlow = evt.raw_data[1];
        thigh = (evt.raw_data[2] & 0x0000FFFF);
        evt.timestamp = thigh << 32;
        evt.timestamp |= tlow;
        evt.timestamp *= timestamp_factor[module];

        sorted_events.push(evt);
        most_recent_t[module] = MAX(most_recent_t[module], evt.timestamp);
        data_avalible += evt.size_raw;

        current_position += event_length;
    }
}