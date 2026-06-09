#include "XIAControl.h"

#include "WriteTerminal.h"
#include "functions.h"
#include "xiaformat.h"

#include <algorithm>
#include <thread>
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <sstream>
#include <utility>
#include <vector>

#include <cstdio>
#include <cstring>
#include <sys/time.h>

#include <pixie16app_export.h>
#include <pixie16sys_export.h>

namespace {
constexpr unsigned int kLiveTimeAAddress = 0x0004a37f;
constexpr unsigned int kLiveTimeBAddress = 0x0004a38f;
constexpr unsigned int kFastPeaksAAddress = 0x0004a39f;
constexpr unsigned int kFastPeaksBAddress = 0x0004a3af;
constexpr unsigned int kRunTimeAAddress = 0x0004a342;
constexpr unsigned int kRunTimeBAddress = 0x0004a343;
constexpr unsigned int kChanEventsAAddress = 0x0004a41f;
constexpr unsigned int kChanEventsBAddress = 0x0004a42f;

constexpr int kChannelsPerModule = 16;
constexpr int kStatisticsWords = 448;

std::string strip(const std::string& s)
{
    std::string::size_type start = s.find_first_not_of(" \t\r\n");
    if( start==std::string::npos )
        start = 0;

    std::string::size_type stop = s.find_last_not_of(" \t\r\n");
    if( stop==std::string::npos )
        stop = s.size()-1;

    return s.substr(start, stop+1-start);
}

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

uint64_t read_counter(const unsigned int *stats, unsigned int high_address, unsigned int low_address)
{
    const auto high_index = high_address - DATA_MEMORY_ADDRESS - DSP_IO_BORDER;
    const auto low_index = low_address - DATA_MEMORY_ADDRESS - DSP_IO_BORDER;
    return (static_cast<uint64_t>(stats[high_index]) << 32) | stats[low_index];
}

std::string firmware_key(const std::string &prefix,
                         unsigned short revision,
                         unsigned short adc_bits,
                         unsigned short adc_msps)
{
    if (revision == 11 || revision == 12 || revision == 13)
        return prefix + "_RevBCD";

    return prefix + "_RevF_" + std::to_string(adc_msps) + "MHz_" + std::to_string(adc_bits) + "Bit";
}

std::string output_path_near_data_file(const std::string &data_filename, const std::string &filename)
{
    const std::filesystem::path data_path(data_filename);
    const auto parent = data_path.parent_path();
    return (parent.empty() ? std::filesystem::path(filename) : parent / filename).string();
}
}



XIAControl::XIAControl(WriteTerminal *writeTerm,
                       const unsigned short PXImap[PRESET_MAX_MODULES],
                       const std::string &FWname,
                       const std::string &SETname)
    : termWrite( writeTerm )
    , data_available( 0 )
    , is_initialized( false )
    , is_booted( false )
    , is_running( false )
    , settings_file( SETname )
    , num_modules( 0 )
    , lmdata( EXTERNAL_FIFO_LENGTH )
{
    ReadConfigFile(FWname.c_str());
    for (int i = 0 ; i < PRESET_MAX_MODULES ; ++i){
        if (PXImap[i] > 0)
            PXISlotMap[num_modules++] = PXImap[i];
    }
    std::fill(std::begin(most_recent_t), std::end(most_recent_t), 0);
    std::fill(std::begin(timestamp_factor), std::end(timestamp_factor), 10);
    std::fill(&last_stats[0][0], &last_stats[0][0] + PRESET_MAX_MODULES * kStatisticsWords, 0);
}

XIAControl::~XIAControl()
{
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
    int have_data = data_available + overflow_queue.size();
    have_data -= XIA_MIN_READOUT;
    if ( have_data < bufsize) // First test to determine if we have enough data to make an actual buffer.
        return false;
    return true;
}



bool XIAControl::XIA_fetch_buffer(uint32_t *buffer, int bufsize, unsigned int *first_header)
{
    int current_pos = 0;
    int have_data = data_available + overflow_queue.size();
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
        current_word = std::move(const_cast<Event_t&>(sorted_events.top()));
        data_available -= current_word.raw_data.size();
        sorted_events.pop();

        for (int i = 0 ; i < current_word.raw_data.size() ; ++i){
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

bool XIAControl::SaveSettingsForDataFile(const char *fname)
{
    if (!fname || fname[0] == '\0')
        return false;

    std::filesystem::path settings_path(fname);
    settings_path.replace_extension(".set");

    char settings_filename[2048];
    snprintf(settings_filename, sizeof(settings_filename), "%s", settings_path.string().c_str());

    int retval = Pixie16SaveDSPParametersToFile(settings_filename);
    if (retval < 0) {
        snprintf(errmsg, sizeof(errmsg), "*ERROR* Pixie16SaveDSPParametersToFile failed, retval = %d\n", retval);
        termWrite->WriteError(errmsg);
        Pixie_Print_MSG(errmsg);
        return false;
    }

    snprintf(errmsg, sizeof(errmsg), "Saved DSP settings to '%s'\n", settings_filename);
    termWrite->Write(errmsg);
    return true;
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
    if (revision != 11 && revision != 12 && revision != 13 && revision != 15) {
        snprintf(errmsg, sizeof(errmsg), "Unknown Pixie-16 revision, rev=%d\n", revision);
        termWrite->WriteError(errmsg);
        return false;
    }

    const std::string key_Com = firmware_key("comFPGAConfigFile", revision, ADCbits, ADCMSPS);
    const std::string key_SPFPGA = firmware_key("SPFPGAConfigFile", revision, ADCbits, ADCMSPS);
    const std::string key_DSPcode = firmware_key("DSPCodeFile", revision, ADCbits, ADCMSPS);
    const std::string key_DSPVar = firmware_key("DSPVarFile", revision, ADCbits, ADCMSPS);

    const auto com_fpga = firmwares.find(key_Com);
    const auto sp_fpga = firmwares.find(key_SPFPGA);
    const auto dsp_code = firmwares.find(key_DSPcode);
    const auto dsp_var = firmwares.find(key_DSPVar);

    for (const auto &entry : {std::make_pair(key_Com, com_fpga),
                              std::make_pair(key_SPFPGA, sp_fpga),
                              std::make_pair(key_DSPcode, dsp_code),
                              std::make_pair(key_DSPVar, dsp_var)}) {
        if (entry.second == firmwares.end()) {
            snprintf(errmsg, sizeof(errmsg), "Missing firmware file '%s'\n", entry.first.c_str());
            termWrite->WriteError(errmsg);
            return false;
        }
    }

    snprintf(ComFPGA, 2048, "%s", com_fpga->second.c_str());
    snprintf(SPFPGA, 2048, "%s", sp_fpga->second.c_str());
    snprintf(DSPcode, 2048, "%s", dsp_code->second.c_str());
    snprintf(DSPVar, 2048, "%s", dsp_var->second.c_str());

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
    unsigned int BLcut[PRESET_MAX_MODULES][kChannelsPerModule];
    int retval;
    for (int i = 0 ; i < num_modules ; ++i){
        for (int j = 0 ; j < kChannelsPerModule ; ++j){
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
    for (int i = 0 ; i < kChannelsPerModule ; ++i){
        snprintf(errmsg, sizeof(errmsg), "\tCh. %d:",i);
        termWrite->Write(errmsg);
    }
    termWrite->Write("\n");

    for (int i = 0 ; i < num_modules ; ++i){
        snprintf(errmsg, sizeof(errmsg), "%d:", i);
        termWrite->Write(errmsg);
        for (int j = 0 ; j < kChannelsPerModule ; ++j){
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
    size_t current_pos = 0;
    const size_t size = overflow_queue.size() + data_available;
    std::vector<uint32_t> buf(size);
    for (auto &i : overflow_queue)
        buf[current_pos++] = i;
    overflow_queue.clear();
    Event_t evt;
    while (current_pos < size){
        evt = std::move(const_cast<Event_t&>(sorted_events.top()));
        sorted_events.pop();
        for (int i = 0 ; i < evt.raw_data.size() ; ++i){
            buf[current_pos++] = evt.raw_data[i];
        }
        data_available = data_available - evt.raw_data.size();
    }

    // Write to disk
    if ( output_file ){
        if ( fwrite(buf.data(), sizeof(uint32_t), size, output_file) != size ){
            termWrite->WriteError("Error while writing to file...\n");
        }

        // Lastly we will save run statistics from each module
        unsigned int run_statistics[kStatisticsWords];
        std::string outname = std::string(fname) + ".stats";
        auto stat_file = fopen(outname.c_str(), "w");
        if (!stat_file) {
            snprintf(errmsg, sizeof(errmsg), "Error: Could not open statistics file '%s'\n", outname.c_str());
            termWrite->WriteError(errmsg);
            return true;
        }
        for ( int mod = 0 ; mod < num_modules ; ++mod ){
            auto retval = Pixie16ReadStatisticsFromModule(run_statistics, mod);
            if ( retval < 0 ){
                std::cerr << "*Error* (Pixie16SaveHistogramToFile): Pixie16ReadHistogramFromFile failed, retval=";
                std::cerr << retval << std::endl;
            }
            fprintf(stat_file, "mod %d: %u", mod, run_statistics[0]);
            for ( int i = 1 ; i < kStatisticsWords ; ++i ){
                fprintf(stat_file, ", %u", run_statistics[i]);
            }
            fprintf(stat_file, "\n");
        }
        fclose(stat_file);
    }

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
    double ICR[PRESET_MAX_MODULES][kChannelsPerModule], OCR[PRESET_MAX_MODULES][kChannelsPerModule];
    unsigned int stats[kStatisticsWords];
    int retval;
    FILE* stat_file = nullptr;
    if ( filename.empty() )
        stat_file = fopen("/dev/null", "w");
    else {
        std::string outname = std::string(filename) + ".stats";
        stat_file = fopen(outname.c_str(), "w");
    }
    if (!stat_file) {
        termWrite->WriteError("Error: Could not open statistics output file\n");
        return false;
    }

    {
        for (int i = 0 ; i < num_modules ; ++i){
            retval = Pixie16ReadStatisticsFromModule(stats, i);
            if (retval < 0){
                snprintf(errmsg, sizeof(errmsg), "*ERROR* Pixie16ReadStatisticsFromModule failed, retval = %d\n", retval);
                termWrite->WriteError(errmsg);
                Pixie_Print_MSG(errmsg);
                fclose(stat_file);
                return false;
            }
            fprintf(stat_file, "mod %d: %u", i, stats[0]);
            for ( int k = 1 ; k < kStatisticsWords ; ++k ){
                fprintf(stat_file, ", %u", stats[k]);
            }
            fprintf(stat_file, "\n");

            for (int j = 0 ; j < kChannelsPerModule ; ++j){
                
                uint64_t fastPeakN = read_counter(stats, kFastPeaksAAddress + j, kFastPeaksBAddress + j);
                uint64_t fastPeakP = read_counter(last_stats[i], kFastPeaksAAddress + j, kFastPeaksBAddress + j);

                double fastPeak = fastPeakN - fastPeakP;

                uint64_t LiveTimeN = read_counter(stats, kLiveTimeAAddress + j, kLiveTimeBAddress + j);
                uint64_t LiveTimeP = read_counter(last_stats[i], kLiveTimeAAddress + j, kLiveTimeBAddress + j);

                double liveTime = LiveTimeN - LiveTimeP;
                if (timestamp_factor[i] == 8)
                    liveTime *= 2e-6/250.;
                else
                    liveTime *= 1e-6/100.;

                uint64_t ChanEventsN = read_counter(stats, kChanEventsAAddress + j, kChanEventsBAddress + j);
                uint64_t ChanEventsP = read_counter(last_stats[i], kChanEventsAAddress + j, kChanEventsBAddress + j);
                
                double ChanEvents = ChanEventsN - ChanEventsP;

                uint64_t runTimeN = read_counter(stats, kRunTimeAAddress, kRunTimeBAddress);
                uint64_t runTimeP = read_counter(last_stats[i], kRunTimeAAddress, kRunTimeBAddress);

                double runTime = runTimeN - runTimeP;

                runTime *= 1.0e-6 / 100.;

                ICR[i][j] = (liveTime !=0) ? fastPeak/liveTime : 0;
                OCR[i][j] = (runTime != 0) ? ChanEvents/runTime : 0;
            }

            for (int j = 0 ; j < kStatisticsWords ; ++j){
                last_stats[i][j] = stats[j];
            }
        }
    }

    fclose(stat_file);
    const std::string scaler_file_in_name = output_path_near_data_file(filename, SCALER_FILE_NAME_IN);
    const std::string scaler_file_out_name = output_path_near_data_file(filename, SCALER_FILE_NAME_OUT);
    const std::string scaler_file_csv_name = output_path_near_data_file(filename, SCALER_FILE_CSV);

    FILE *scaler_file_in = fopen(scaler_file_in_name.c_str(), "w");
    FILE *scaler_file_out = fopen(scaler_file_out_name.c_str(), "w");
    if (!scaler_file_in || !scaler_file_out) {
        termWrite->WriteError("Error: Could not open scaler output files\n");
        if (scaler_file_in)
            fclose(scaler_file_in);
        if (scaler_file_out)
            fclose(scaler_file_out);
        return false;
    }

    fprintf(scaler_file_in, "Input count rate:\n\n\n");
    fprintf(scaler_file_out, "Output count rate:\n\n\n");

    std::stringstream csv_stream;
    csv_stream << "module,channel,input,output\n";

    for (int i = 0 ; i < kChannelsPerModule ; ++i){
        fprintf(scaler_file_in, "\t%d", i);
        fprintf(scaler_file_out, "\t%d", i);
    }
    fprintf(scaler_file_in, "\n");
    fprintf(scaler_file_out, "\n");

    for (int i = 0 ; i < num_modules ; ++i){
        fprintf(scaler_file_in, "%d", i);
        fprintf(scaler_file_out, "%d", i);
        for (int j = 0 ; j < kChannelsPerModule ; ++j){
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
        std::ofstream csv_file(scaler_file_csv_name);
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
    uint32_t *FIFOdata = lmdata.data();
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
        if (fifoSize < 12 /* EXTFIFO_READ_THRESH */ ) // Make sure we don't read from an empty FIFO.
            continue;
        if (fifoSize > lmdata.size()) {
            snprintf(errmsg, sizeof(errmsg), "*ERROR* External FIFO size (%u) exceeds readout buffer size (%zu)\n", fifoSize, lmdata.size());
            termWrite->WriteError(errmsg);
            return false;
        }
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
    // We will work on the data from
    overflow_fifo[module].insert(overflow_fifo[module].end(), raw_data, raw_data + size);
    tmp.reserve(size + overflow_fifo[module].size());
    tmp.insert(tmp.end(), overflow_fifo[module].begin(), overflow_fifo[module].end());
    overflow_fifo[module].clear();

    auto begin = tmp.data();
    auto end = begin + tmp.size();
    auto pos = begin;
    while ( pos < end) {
        auto entry = reinterpret_cast<const XIA_base_t *>(pos);
        if (pos + entry->eventLen > end) {
            overflow_fifo[module].insert(overflow_fifo[module].end(), pos, end);
            tmp.clear();
            pos = end + 1; // make sure we are beyond the end
            break;
        } else {
            sorted_events.emplace(entry->timestamp() * timestamp_factor[module], pos, pos+entry->eventLen);
            pos += entry->eventLen;
            data_available += entry->eventLen;
        }
    }
}
