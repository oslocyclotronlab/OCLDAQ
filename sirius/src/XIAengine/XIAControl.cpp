#include "XIAControl.h"

#include "WriteTerminal.h"
#include "utilities.h"

#include <thread>
#include <chrono>
#include <fstream>

#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <sys/time.h>

#include "pixie16app_export.h"
#include "pixie16sys_export.h"


const bool next_line(std::istream &in, std::string &line)
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
    , settings_file( SETname )
{
    ReadConfigFile(FWname.c_str());
    num_modules = 0;
    for (int i = 0 ; i < PRESET_MAX_MODULES ; ++i){
        if (PXImap[i] > 0)
            PXISlotMap[num_modules++] = PXImap[i];
    }
}


void XIAControl::XIAthread()
{
    // We set the internal flag signalizing that the thread is running.
    thread_is_running = true;
    timeval tim;
    double t1=0, t2=0;

    // We should run all the time, but only while there is no active
    // runs we will be idle for 1 second at a time, then check if
    // a run has been started or not.
    while ( thread_is_running ){
        if (is_running){

            // Check if we have enough for reading out XIA (we require at least 16384 words)
            if (CheckFIFO(XIA_FIFO_MIN_READOUT)){

                if (!ReadFIFO())
                    StopRun();
            }


            gettimeofday(&tim, NULL);
            t2 = tim.tv_sec + 1e-6*tim.tv_usec;

            if (t2 - t1 > 5){
                if ( !WriteScalers() )
                    StopRun();
                t1 = t2;
            }

        } else {
            std::this_thread::sleep_for(std::chrono::seconds(1)); // We sleep for 1 second at a time... Maybe a bit much?
        }
    }


    return;
}


bool XIAControl::SetPXIMapping(const unsigned short PXImap[PRESET_MAX_MODULES])
{
    if (is_running)
        return false;

    if (is_booted || is_initialized ){
        is_initialized = ExitXIA();
        is_booted = is_initialized ? true : false;
    }

    // Update the PXI mapping.
    for (int i = 0 ; i < PRESET_MAX_MODULES ; ++i){
        if (PXImap[i] > 0)
            PXISlotMap[num_modules++] = PXImap[i];
    }

    // Finished!
    return true;
}

bool XIAControl::SetFirmwareFile(const std::string &FWname)
{
    if (is_running)
        return false;

    is_booted = false; // We set this to zero so that we force the reboot later.
    return ReadConfigFile(FWname.c_str());
}

bool XIAControl::SetSettingsFile(const std::string &SETname)
{
    if (is_running)
        return false;

    settings_file = SETname;

    if (is_booted){ // First we will try writing the data without rebooting.
        std::lock_guard<std::mutex> xia_guard(xia_mutex);
        char tmp[2048];
        sprintf(tmp, "%s", settings_file.c_str());
        int retval = Pixie16LoadDSPParametersFromFile(tmp);
        if (retval < 0){
            is_booted = false; // Try again next time we boot.
            sprintf(errmsg, "*ERROR* Pixie16LoadDSPParametersFromFile failed, retval = %d\n", retval);
            termWrite->WriteError(errmsg);
            Pixie_Print_MSG(errmsg);
        }
    }

    return true; // We were at lease able to change the internal value of where to read the .set file.
}


bool XIAControl::XIA_check_buffer(int bufsize)
{
    // Check that we are actually running.
    if (!is_running)
        return false;

    // Lock the queue mutex such that we can check if we have enough data.
    std::lock_guard<std::mutex> queue_guard(queue_mutex);
    int have_data = data_avalible + overflow_queue.size();
    have_data -= XIA_MIN_READOUT;
    if ( have_data < bufsize)
        return false;

    return  true;
}

bool XIAControl::XIA_check_buffer_ST(int bufsize)
{
    // Check that we are actually running.
    if (!is_running)
        return false;
    double t1=last_time.tv_sec + 1e-6*last_time.tv_usec;
    double t2;
    if (CheckFIFO(XIA_FIFO_MIN_READOUT)){

        if (!ReadFIFO())
            StopRun();
    }
    timeval tmp;
    gettimeofday(&tmp, NULL);
    t2 = tmp.tv_sec + 1e-6*tmp.tv_usec;
    if (t2 - t1 > 5 ){
        WriteScalers();
        last_time = tmp;
    }


    // Lock the queue mutex such that we can check if we have enough data.
    std::lock_guard<std::mutex> queue_guard(queue_mutex);
    int have_data = data_avalible + overflow_queue.size();
    have_data -= XIA_MIN_READOUT;
    if ( have_data < bufsize)
        return false;

    return  true;
}



bool XIAControl::XIA_fetch_buffer(uint32_t *buffer, int bufsize)
{
    std::lock_guard<std::mutex> queue_guard(queue_mutex);
    int current_pos = 0;
    int have_data = data_avalible + overflow_queue.size();
    have_data -= XIA_MIN_READOUT;

    // This function should NEVER be called unless we have
    // enough data. However, in the case it happends.
    if ( have_data < bufsize ){
        return false;
    }

    for (size_t i = 0 ; i < overflow_queue.size() ; ++i){
        buffer[current_pos++] = overflow_queue[i];
    }
    overflow_queue.clear();
    Event_t current_word;
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

bool XIAControl::XIA_start_run()
{
    // We should NOT try to lock the XIA resources at this
    // time since we will be using a comibination of function
    // calls to other methods that will do the spesific things
    // we want it to do.
    //std::lock_guard<std::mutex> xia_guard(xia_mutex);

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

    // Adjust baseline.
    AdjustBaseline(); // At the moment, this isn't a critical error. We can keep on running.
    // We will wait a second before moving on.
    termWrite->Write("Sleeping for 1 second");
    std::this_thread::sleep_for(std::chrono::seconds(1));
    termWrite->Write("... Awake again\n");

    // Write synch to the modules.
    if (!SynchModules()) // We won't start running unless all modules are in synch and ready for action...
        return false;

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

bool XIAControl::XIA_end_run(FILE *output_file)
{

    // Check if run is ongoing. If not, return true.
    if (!is_running)
        return true;

    // Now we will end the list mode run.
    is_running = StopRun();

    // We will wait for 0.5 seconds to make sure that all data
    // has been processed by the XIA DSP/FPGA.
    termWrite->Write("Sleeping for 0.5 seconds...");
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    termWrite->Write("... Awake again.\n");

    // Then we will run a readout of the FIFO.
    if (!ReadFIFO()) // We had an error. I'm not sure how this will be fixed, if fixed...
        return true;


    // Dump the rest of the data we have onto the file.
    std::lock_guard<std::mutex> guard_queue(queue_mutex);

    // Allocate memory where we will dump the FIFO contents.
    unsigned int current_pos = 0;
    unsigned int size = overflow_queue.size() + data_avalible;
    uint32_t *buf = new uint32_t[size];
    for (unsigned int i = 0 ; i < overflow_queue.size() ; ++i)
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
    is_booted = is_initialized ? true : false;
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
        std::string::size_type pos_eq = line.find("=");

        // If not found, write a warning and continue to next line.
        if ( pos_eq == std::string::npos ){
            sprintf(errmsg, "Could not understand line '%s', continuing...\n", line.c_str());
            termWrite->WriteError(errmsg);
            continue;
        }

        std::string key = strip(line.substr(0, pos_eq));
        std::string val = strip(line.substr(pos_eq+1));

        // If the key have already been entered.
        if ( fw.find(key) != fw.end() ){
            sprintf(errmsg, "Mutiple definitions of '%s'\n", key.c_str());
            termWrite->WriteError(errmsg);
            return false;
        }

        fw[key] = val;
    }
    termWrite->Write("Done reading firmware files\n");
    firmwares.swap(fw);
    return true;
}

bool XIAControl::InitializeXIA()
{
    // Lock the XIA mutex to prevent any other
    // thread from communicating with the modules.
    std::lock_guard<std::mutex> xia_guard(xia_mutex);

    int retval = Pixie16InitSystem(num_modules, PXISlotMap, 0);
    if (retval < 0){
        sprintf(errmsg, "*ERROR* Pixie16InitSystem failed, retval = %d\n", retval);
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
        sprintf(errmsg, "Unknown Pixie-16 revision, rev=%d\n", revision);
        termWrite->WriteError(errmsg);
        return false;
    }

    // Search our map for the firmware files.
    if ( firmwares.find(key_Com) == firmwares.end() ){
        sprintf(errmsg, "Missing firmware file '%s'\n", key_Com.c_str());
        termWrite->WriteError(errmsg);
        return false;
    }

    if ( firmwares.find(key_SPFPGA) == firmwares.end() ){
        sprintf(errmsg, "Missing firmware file '%s'\n", key_SPFPGA.c_str());
        termWrite->WriteError(errmsg);
        return false;
    }

    if ( firmwares.find(key_DSPcode) == firmwares.end() ){
        sprintf(errmsg, "Missing firmware file '%s'\n", key_DSPcode.c_str());
        termWrite->WriteError(errmsg);
        return false;
    }

    if ( firmwares.find(key_DSPVar) == firmwares.end() ){
        sprintf(errmsg, "Missing firmware file '%s'\n", key_DSPVar.c_str());
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

    // Lock the XIA mutex to prevent any other
    // thread from communicating with the modules.
    std::lock_guard<std::mutex> xia_guard(xia_mutex);

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
            sprintf(errmsg, "*ERROR* Pixie16ReadModuleInfo failed, retval = %d\n", retval);
            termWrite->WriteError(errmsg);
            Pixie_Print_MSG(errmsg);
            return false;
        }
    }

    for (int i = 0 ; i < num_modules ; ++i){
        if (!GetFirmwareFile(rev[i], bit[i], MHz[i],
                             ComFPGA, SPFPGA,
                             DSPCode, DSPVar)) {
            sprintf(errmsg, "Module %d: Unknown module\n", i);
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


        sprintf(errmsg, "Booting Pixie-16 module #%d, Rev=%d, S/N=%d, Bits=%d, MSPS=%d\n", i, rev[i], sn[i], bit[i], MHz[i]);
        termWrite->Write(errmsg);
        termWrite->Write("ComFPGAConfigFile:\t");
        termWrite->Write(ComFPGA);
        termWrite->Write("\nSPFPGAConfigFile:\t");
        termWrite->Write(SPFPGA);
        termWrite->Write("\nDSPCodeFile:\t");
        termWrite->Write(DSPCode);
        termWrite->Write("\nDSPVarFile:\t");
        termWrite->Write(DSPVar);
        termWrite->Write("\n----------------------------------------\n\n");

        retval = Pixie16BootModule(ComFPGA, SPFPGA, TrigFPGA, DSPCode, DSPSet, DSPVar, i, 0x7F);
        if (retval < 0){
            sprintf(errmsg, "*ERROR* Pixie16BootModule failed, retval = %d\n", retval);
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

    // Lock the XIA mutex to prevent any other
    // thread from communicating with the modules.
    std::lock_guard<std::mutex> xia_guard(xia_mutex);

    termWrite->Write("Adjusting baseline of all modules and channels...");
    int retval = Pixie16AdjustOffsets(num_modules);
    if (retval < 0){
        sprintf(errmsg, "*ERROR* Pixie16AdjustOffsets failed, retval = %d\n", retval);
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
    // Lock the XIA mutex to prevent any other
    // thread from communicating with the modules.
    std::lock_guard<std::mutex> xia_guard(xia_mutex);

    termWrite->Write("Acquiring the baseline cut...");
    unsigned int BLcut[PRESET_MAX_MODULES][16];
    int retval;
    for (int i = 0 ; i < num_modules ; ++i){
        for (int j = 0 ; j < 16 ; ++j){
            retval = Pixie16BLcutFinder(i, j, &BLcut[i][j]);
            if (retval < 0){
                sprintf(errmsg, "*ERROR* Pixie16BLcutFinder for mod = %d, ch = %d failed, retval = %d\n", i, j, retval);
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
        sprintf(errmsg, "\tCh. %d:",i);
        termWrite->Write(errmsg);
    }
    termWrite->Write("\n");

    for (int i = 0 ; i < num_modules ; ++i){
        sprintf(errmsg, "%d:", i);
        termWrite->Write(errmsg);
        for (int j = 0 ; j < 16 ; ++j){
            sprintf(errmsg, "\t%d", i);
            termWrite->Write(errmsg);
        }
        termWrite->Write("\n");
    }

    return true;
}


bool XIAControl::StartLMR()
{
    // Lock the XIA mutex to prevent any other
    // thread from communicating with the modules.
    std::lock_guard<std::mutex> xia_guard(xia_mutex);

    termWrite->Write("Trying to write SYNCH_WAIT...\n");
    int retval = Pixie16WriteSglModPar("SYNCH_WAIT", 1, 0);
    if (retval < 0){
        sprintf(errmsg, "*ERROR* Pixie16WriteSglModPar writing SYNCH_WAIT failed, retval = %d\n", retval);
        termWrite->WriteError(errmsg);
        Pixie_Print_MSG(errmsg);
        return false;
    }
    termWrite->Write("... Done.\n");

    termWrite->Write("About to start list mode run...\n");
    retval = Pixie16StartListModeRun(num_modules, 0x100, NEW_RUN);
    if (retval < 0){
        sprintf(errmsg, "*ERROR* Pixie16StartListModeRun failed, retval = %d\n", retval);
        termWrite->WriteError(errmsg);
        Pixie_Print_MSG(errmsg);
        return false;
    }
    termWrite->Write("List mode started OK\n");
    return true;
}

bool XIAControl::CheckIsRunning()
{
    // Lock the XIA mutex to prevent any other
    // thread from communicating with the modules.
    std::lock_guard<std::mutex> xia_guard(xia_mutex);

    bool am_I_running = true;
    int retval;
    for (int i = 0 ; i < num_modules ; ++i){
        retval = Pixie16CheckRunStatus(i);
        if (retval < 0){
            sprintf(errmsg, "*ERROR* Pixie16CheckRunStatus failed, retval = %d\n", retval);
            termWrite->WriteError(errmsg);
            Pixie_Print_MSG(errmsg);
        }
        am_I_running = (am_I_running&&retval);
    }

    return am_I_running;
}


bool XIAControl::StopRun()
{
    // Lock the XIA mutex to prevent any other
    // thread from communicating with the modules.
    std::lock_guard<std::mutex> xia_guard(xia_mutex);

    int retval;

    // In principle, we should only need to do this for one of
    // the modules. However we will try to stop all of them.
    // Most will probably get retval=0 which means it has stopped.
    for (int i = 0 ; i < num_modules ; ++i){
        // First we check that the module are in fact running.
        retval = Pixie16CheckRunStatus(i);
        if (retval < 0){
            sprintf(errmsg, "*ERROR* Pixie16CheckRunStatus failed, retval = %d\n", retval);
            termWrite->WriteError(errmsg);
            Pixie_Print_MSG(errmsg);
        } else if (retval > 0){
            retval = Pixie16EndRun(i);
            if (retval < 0){
                sprintf(errmsg, "*ERROR* Pixie16EndRun failed, retval = %d\n", retval);
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
    // Lock the XIA mutex to prevent any other
    // thread from communicating with the modules.
    std::lock_guard<std::mutex> xia_guard(xia_mutex);

    termWrite->Write("Trying to write IN_SYNCH...\n");
    int retval = Pixie16WriteSglModPar("IN_SYNCH", 0, 0);
    if (retval < 0){
        sprintf(errmsg, "*ERROR* Pixie16WriteSglModPar writing IN_SYNCH failed, retval = %d\n", retval);
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
        // Lock the XIA mutex to prevent any other
        // thread from communicating with the modules.
        std::lock_guard<std::mutex> xia_guard(xia_mutex);
        for (int i = 0 ; i < num_modules ; ++i){
            retval = Pixie16ReadStatisticsFromModule(stats, i);
            if (retval < 0){
                sprintf(errmsg, "*ERROR* Pixie16ReadStatisticsFromModule failed, retval = %d\n", retval);
                termWrite->WriteError(errmsg);
                Pixie_Print_MSG(errmsg);
            }

            for (int j = 0 ; j < 16 ; ++j){
                ICR[i][j] = Pixie16ComputeInputCountRate(stats, i, j);
                OCR[i][j] = Pixie16ComputeOutputCountRate(stats, i, j);
            }
        }
    }

    std::ofstream scaler_file_in(SCALER_FILE_NAME_IN);
    std::ofstream scaler_file_out(SCALER_FILE_NAME_OUT);

    scaler_file_in << "Module:/Ch.:";
    scaler_file_out << "Module:/Ch.:";
    for (int i = 0 ; i < 16 ; ++i){
        scaler_file_in << "\t" << i;
        scaler_file_out << "\t" << i;
    }
    scaler_file_in << "\n";
    scaler_file_out << "\n";

    for (int i = 0 ; i < num_modules ; ++i){
        scaler_file_in << i;
        scaler_file_out << i;
        for (int j = 0 ; j < 16 ; ++j){
            scaler_file_in << "\t" << ICR[i][j];
            scaler_file_out << "\t" << OCR[i][j];
        }
        scaler_file_in << "\n";
        scaler_file_out << "\n";
    }
    scaler_file_in << std::flush;
    scaler_file_out << std::flush;
    scaler_file_in.close();
    scaler_file_out.close();

    return true;
}


bool XIAControl::CheckFIFO(unsigned int minReadout)
{

    // Lock the XIA mutex to prevent any other
    // thread from communicating with the modules.
    std::lock_guard<std::mutex> xia_guard(xia_mutex);

    unsigned int numFIFOwords;
    int retval;

    for (int i = 0 ; i < num_modules ; ++i){
        retval = Pixie16CheckExternalFIFOStatus(&numFIFOwords, i);
        if (retval < 0){
            sprintf(errmsg, "*ERROR* Pixie16CheckExternalFIFOStatus failed, retval = %d\n", i);
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

    // Lock the XIA mutex to prevent any other
    // thread from communicating with the modules.
    std::lock_guard<std::mutex> xia_guard(xia_mutex);

    uint32_t *FIFOdata;
    unsigned int fifoSize;
    int retval;
    for (int i = 0 ; i < num_modules ; ++i){
        retval = Pixie16CheckExternalFIFOStatus(&fifoSize, i);
        if (retval == -1){
            sprintf(errmsg, "*ERROR* Pixie16CheckExternalFIFOStatus failed, retval = %d\n", i);
            termWrite->WriteError(errmsg);
            Pixie_Print_MSG(errmsg);
            return false;
        }
        if (fifoSize > 0) // Make sure we don't read from an empty FIFO.
            continue;
        FIFOdata = new uint32_t[fifoSize];
        retval = Pixie16ReadDataFromExternalFIFO(FIFOdata, fifoSize, i);
        if (retval == -1){
            sprintf(errmsg, "*ERROR* Pixie16ReadDataFromExternalFIFO failed, retval = %d\n", i);
            termWrite->WriteError(errmsg);
            Pixie_Print_MSG(errmsg);
            delete[] FIFOdata;
            return false;
        }
        ParseQueue(FIFOdata, fifoSize, i);
        delete[] FIFOdata;
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

    // Lock the XIA mutex to prevent any other
    // thread from communicating with the modules.
    std::lock_guard<std::mutex> xia_guard(xia_mutex);
    int retval = Pixie16ExitSystem(num_modules);
    if (retval < 0){
        sprintf(errmsg, "*ERROR* Pixie16ExitSystem failed, retval = %d\n", retval);
        termWrite->Write(errmsg);
        return false;
    }
    return true;
}


void XIAControl::ParseQueue(uint32_t *raw_data, int size, int module)
{
    int event_length, header_length;
    int current_position;
    int64_t tlow, thigh;
    Event_t evt;

    if (overflow_fifo[module].size() > 0){
        event_length = (overflow_fifo[module][0] & 0x3FFE0000) >> 17;
        header_length = (overflow_fifo[module][0] & 0x0001F000) >> 12;

        if (event_length - overflow_fifo[module].size() > size) { // Event spans several FIFOs :O
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
        evt.size_raw = header_length;
        delete[] tmp;

        // Striping away the list mode data.
        if (event_length != header_length){
            uint32_t new_f_head = header_length << 17;
            new_f_head |= (evt.raw_data[0] & 0x8001FFFF);
            evt.raw_data[0] = new_f_head;
            evt.raw_data[3] = (evt.raw_data[3] & 0x0000FFFF);
        }

        {   // Creating a scope for the guard to live.
            std::lock_guard<std::mutex> queue_guard(queue_mutex);
            sorted_events.push(evt);
            data_avalible += evt.size_raw;
        }

        current_position = event_length - overflow_fifo[module].size();
        overflow_fifo[module].clear();
    }

    while (current_position < size){
        event_length = (raw_data[current_position] & 0x3FFE0000) >> 17;
        header_length = (raw_data[current_position] & 0x0001F000) >> 12;

        if (current_position + event_length > size){
            for (int i = current_position ; i < size ; ++i){
                overflow_fifo[module].push_back(raw_data[i]);
            }
            break;
        }

        for (int i = 0 ; i < header_length ; ++i){
            evt.raw_data[i] = raw_data[current_position+i];
        }
        evt.size_raw = header_length;

        tlow = evt.raw_data[1];
        thigh = (evt.raw_data[2] & 0x0000FFFF);
        evt.timestamp = thigh << 32;
        evt.timestamp |= tlow;
        evt.timestamp *= timestamp_factor[module];

        // Remove trace
        if (event_length != header_length){
            uint32_t new_f_head = header_length << 17;
            new_f_head |= (evt.raw_data[0] & 0x8001FFFF);
            evt.raw_data[0] = new_f_head;
            evt.raw_data[3] = (evt.raw_data[3] & 0x0000FFFF);
        }

        {   // Creating a scope for the guard to live.
            std::lock_guard<std::mutex> queue_guard(queue_mutex);
            sorted_events.push(evt);
            data_avalible += evt.size_raw;
        }   // Should be released here...

        current_position += event_length;
    }
    return;
}
