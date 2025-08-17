//
// Created by Vetle Wegner Ingeberg on 17/08/2025.
//


#include <pixie16app_export.h>
#include <pixie16app_defs.h>

#include <string>
#include <array>
#include <map>
#include <sstream>
#include <memory>
#include <iostream>
#include <fstream>

#include <csignal>
#include <cstring>

#include "functions.h"
#include "XIAControl.h"
char leaveprog = 'n';

void keyb_int(int sig_num)
{
    if (sig_num == SIGINT) {
        std::cout << "\n\nLeaving...\n" << std::endl;
        leaveprog = 'y';
    }
}

struct HW_Info_t {
    unsigned short revision;
    unsigned short bitdepth;
    unsigned short adcmhz;
    unsigned int serial_number;
};

using FW_Map_t = std::map<std::string, std::string>;

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

FW_Map_t ParseFWconfigFile(const char *config)
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

    FW_Map_t fw;
    std::cout << "Reading firmware mapping from file '" << config << "'" << std::endl;

    if ( !input.is_open() ){
        std::cerr << "Error: Could not open cofiguration file '" << config << "'." << std::endl;
        return fw;
    }

    while ( next_line(input, line) ){
        if ( line.empty() || line[0] == '#' )
            continue; // Ignore empty lines or comments.

        // Search for "=" sign on the line.
        std::string::size_type pos_eq = line.find('=');

        // If not found, write a warning and continue to next line.
        if ( pos_eq == std::string::npos ){
            std::cerr << "Could not understand line '" << line << "', continuing..." << std::endl;
            continue;
        }

        std::string key = strip(line.substr(0, pos_eq));
        std::string val = strip(line.substr(pos_eq+1));

        // If the key have already been entered.
        if ( fw.find(key) != fw.end() ){
            std::cerr << "Found multiple definitions of '" << key << "'. Using the latest." << std::endl;
        }

        fw[key] = val;
    }
    return fw;
}


bool GetFirmwareFile(FW_Map_t firmwares,
                const unsigned short &revision, const unsigned short &ADCbits, const unsigned short &ADCMSPS,
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
        std::cerr << "Unknown Pixie-16 revision, rev = " << revision << std::endl;
        return false;
    }

    // Search our map for the firmware files.
    if ( firmwares.find(key_Com) == firmwares.end() ){
        std::cerr << "Missing firmware file '" << key_Com << "'." << std::endl;
        return false;
    }

    if ( firmwares.find(key_SPFPGA) == firmwares.end() ){
        std::cerr << "Missing firmware file '" << key_SPFPGA << "'." << std::endl;
        return false;
    }

    if ( firmwares.find(key_DSPcode) == firmwares.end() ){
        std::cerr << "Missing firmware file '" << key_DSPcode << "'." << std::endl;
        return false;
    }

    if ( firmwares.find(key_DSPVar) == firmwares.end() ){
        std::cerr << "Missing firmware file '" << key_DSPVar << "'." << std::endl;
        return false;
    }

    // If we reach this point, we know that we have all the firmwares!
    strcpy(ComFPGA, firmwares[key_Com].c_str());
    strcpy(SPFPGA, firmwares[key_SPFPGA].c_str());
    strcpy(DSPcode, firmwares[key_DSPcode].c_str());
    strcpy(DSPVar, firmwares[key_DSPVar].c_str());

    return true;
}

bool boot(int num_modules) {
    auto fw = ParseFWconfigFile("XIA_Firmware.txt");

    // Boot the modules
    int retval = 0;
    char comFPGA[2048], SPFPGA[2048], DSPCode[2048], DSPVar[2048];
    char trigFPGA[] = "trig";
    char DSPSet[2048];
    strcpy(DSPSet, "settings.set");
    HW_Info_t hardware[PRESET_MAX_MODULES];

    for ( size_t i = 0 ; i < num_modules ; ++i ){
        retval = Pixie16ReadModuleInfo(i, &hardware[i].revision, &hardware[i].serial_number,
                                       &hardware[i].bitdepth, &hardware[i].adcmhz);
        if ( retval < 0 ){
            std::cerr << "*ERROR* Pixie16ReadModuleInfo failed, retval = " << retval << std::endl;
            return retval;
        }
    }

    for ( size_t i = 0 ; i < num_modules ; ++i ){
        if ( !GetFirmwareFile(fw, hardware[i].revision, hardware[i].bitdepth, hardware[i].adcmhz,
                              comFPGA, SPFPGA, DSPCode, DSPVar) ){
            std::cerr << "Module " << i << ": Unknown module" << std::endl;
            return -1;
        }

        std::cout << "----------Booting Pixie-16 module #" << i << "----------" << std::endl;
        std::cout << "Revision:\t" << hardware[i].revision << std::endl;
        std::cout << "Serial number:\t" << hardware[i].serial_number << std::endl;
        std::cout << "ADC Bits:\t" << hardware[i].bitdepth << std::endl;
        std::cout << "ADC MSPS:\t" << hardware[i].adcmhz << std::endl;
        std::cout << "ComFPGAConfigFile:\t" << comFPGA << std::endl;
        std::cout << "SPFPGAConfigFile:\t" << SPFPGA << std::endl;
        std::cout << "DSPCodeFile:\t" << DSPCode << std::endl;
        std::cout << "DSPVarFile:\t" << DSPVar << std::endl;
        std::cout << "DSPSetFile:\t" << DSPSet << std::endl;
        retval = Pixie16BootModule(comFPGA, SPFPGA, trigFPGA,
                                   DSPCode, DSPSet, DSPVar, i, 0x7F);
        if ( retval < 0 ){
            std::cerr << "*ERROR* Pixie16BootModule failed, retval = " << retval << std::endl;
            return retval;
        }
        std::cout << "Adjusting baseline..." << std::flush;
        retval = Pixie16AdjustOffsets(i);
        if ( retval < 0 ){
            std::cerr << "*ERROR* Pixie16AdjustOffsets failed, retval = " << retval << std::endl;
            return retval;
        }
        std::cout << " Done." << std::endl;

    }

    std::cout << "All modules booted." << std::endl;
    std::cout << "----------------------------------------------" << std::endl;
    return true;
}

int main(int argc, char *argv[]) {
    signal(SIGINT, keyb_int); // set up interrupt handler (Ctrl-C)
    signal(SIGPIPE, SIG_IGN);

    int retval = 0;
    std::string output_file_name[PRESET_MAX_MODULES];
    FILE* files[PRESET_MAX_MODULES];
    unsigned short PXIMapping[PRESET_MAX_MODULES];
    auto mapping = ReadSlotMap();
    int set = 0;
    for ( auto &entry : mapping ){
        PXIMapping[set++] = entry;
    }
    int num_modules = set;
    retval = Pixie16InitSystem(num_modules, PXIMapping, 0);
    if ( retval != 0 ) {
        std::cerr << "Error initializing, got code " << retval << std::endl;
    }

    // Next we will boot
    if ( !boot(num_modules) ) {
        std::cerr << "Booting failed." << std::endl;
        retval = Pixie16ExitSystem(num_modules);
        return retval;
    }

    if ( argc == 2 ) {
        for ( int i = 0 ; i < num_modules ; ++i ) {
            output_file_name[i] = argv[1] + std::string("_") + std::to_string(i) + ".data";
        }
    } else {
        for ( int i = 0 ; i < num_modules ; ++i ) {
            output_file_name[i] = std::string("module_") + std::to_string(i) + ".data";
        }
    }

    // Synchronize modules
    retval = Pixie16WriteSglModPar(const_cast<char *>("IN_SYNCH"), 0, 0);
    if (  retval < 0 ) {
        std::cout << "Error synchronizing modules, retval = " << retval << std::endl;
        retval = Pixie16ExitSystem(num_modules);
        return retval;
    }

    // Start run
    retval = Pixie16StartListModeRun(num_modules, LIST_MODE_RUN, NEW_RUN);
    if ( retval != 0 ) {
        std::cerr << "Error starting run, got code " << retval << std::endl;
        retval = Pixie16ExitSystem(num_modules);
        return retval;
    }

    for ( int i = 0 ; i < num_modules ; ++i ) {
        files[i] = fopen(output_file_name[i].c_str(), "wb");
    }

    auto* lmdata = (unsigned int *)malloc(sizeof(unsigned int) * EXTERNAL_FIFO_LENGTH);
    unsigned int numFIFOwords;
    // Next we can start the run
    while ( leaveprog == 'n' ) {
        for ( int i = 0 ; i < num_modules ; ++i ) {
            retval = Pixie16CheckExternalFIFOStatus(&numFIFOwords, i);
            if ( retval != 0 ) {
                std::cerr << "Pixie16CheckExternalFIFOStatus gave " << retval << std::endl;
                retval = Pixie16ExitSystem(num_modules);
                free(lmdata);
                return retval;
            }

            if ( numFIFOwords > 2048 ) {
                retval = Pixie16ReadDataFromExternalFIFO(lmdata, numFIFOwords, i);
                if ( retval != 0 ) {
                    std::cerr << "Pixie16ReadDataFromExternalFIFO gave " << retval << std::endl;
                    retval = Pixie16ExitSystem(num_modules);
                    free(lmdata);
                    return retval;
                }
                fwrite(lmdata, sizeof(unsigned int), numFIFOwords, files[i]);
            }
        }
    }

    retval = Pixie16EndRun(num_modules);
    if ( retval != 0 ) {
        std::cerr << "Error ending run, got code " << retval << std::endl;
        retval = Pixie16ExitSystem(num_modules);
        return retval;
    }
    usleep(1000000); // Wait a second

    for ( int i = 0 ; i < num_modules ; ++i ) {
        retval = Pixie16CheckExternalFIFOStatus(&numFIFOwords, i);
        if ( retval != 0 ) {
            std::cerr << "Pixie16CheckExternalFIFOStatus gave " << retval << std::endl;
            retval = Pixie16ExitSystem(num_modules);
            free(lmdata);
            return retval;
        }
        retval = Pixie16ReadDataFromExternalFIFO(lmdata, numFIFOwords, i);
        if ( retval != 0 ) {
            std::cerr << "Pixie16ReadDataFromExternalFIFO gave " << retval << std::endl;
            retval = Pixie16ExitSystem(num_modules);
            free(lmdata);
            return retval;
        }
        fwrite(lmdata, sizeof(unsigned int), numFIFOwords, files[i]);
    }
    retval = Pixie16ExitSystem(num_modules);
    if (  retval != 0 ) {
        std::cerr << "Error exiting system, got code " << retval << std::endl;
    }
    free(lmdata);
    return retval;
}