//
// Created by Vetle Wegner Ingeberg on 25/06/2025.
//

#include <iostream>
#include <fstream>
#include <map>
#include <cstring>
#include <csignal>

#include <unistd.h>


#include <pixie16app_export.h>

#include "functions.h"

void keyb_int(int sig_num)
{
    if (sig_num == SIGINT) {
        std::cout << "Trying to end run..." << std::endl;
        auto retval = Pixie16EndRun(0);
        if ( retval < 0 )
            std::cerr << "Error ending run" << std::endl;
        else
            std::cout << "Run end signal has been sent" << std::endl;
    }
}

unsigned short PXIMapping[PRESET_MAX_MODULES];
//int timestamp_factor[PRESET_MAX_MODULES];
using fw_map_t = std::map<std::string, std::string>;

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

std::map<std::string, std::string> ReadConfig(const char *config){
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
    std::cout << "Reading firmware file..." << std::endl;

    if ( !input.is_open() ){
        std::cerr << "Error: Couldn't read firmware config. file" << std::endl;
        return {};
    }

    while ( next_line(input, line) ){
        if ( line.empty() || line[0] == '#' )
            continue; // Ignore empty lines or comments.

        // Search for "=" sign on the line.
        std::string::size_type pos_eq = line.find('=');

        // If not found, write a warning and continue to next line.
        if ( pos_eq == std::string::npos ){
            std::cerr << "Error: Couldn't parse config line '" << line << "', skipping..." << std::endl;
            continue;
        }

        std::string key = strip(line.substr(0, pos_eq));
        std::string val = strip(line.substr(pos_eq+1));

        // If the key have already been entered.
        if ( fw.find(key) != fw.end() ){
            std::cerr << "Multiple definitions of '" << key.c_str() << "'" << std::endl;
        }

        fw[key] = val;
    }
    std::cout << "Configuration successfully loaded" << std::endl;
    return fw;
}

bool GetFirmwareFile(const fw_map_t& firmwares, const unsigned short &revision, const unsigned short &ADCbits,
                     const unsigned short &ADCMSPS, char *ComFPGA, char *SPFPGA, char *DSPcode, char *DSPVar)
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
        std::cerr << "Unknown Pixie-16 revision, rev=" << revision << std::endl;
        return false;
    }

    // Search our map for the firmware files.
    if ( firmwares.find(key_Com) == firmwares.end() ){
        std::cerr << "Missing firmware file '" << key_Com << "'" << std::endl;
        return false;
    }

    key_Com = firmwares.find(key_Com)->second;

    if ( firmwares.find(key_SPFPGA) == firmwares.end() ){
        std::cerr << "Missing firmware file '" << key_SPFPGA << "'" << std::endl;
        return false;
    }

    key_SPFPGA = firmwares.find(key_SPFPGA)->second;

    if ( firmwares.find(key_DSPcode) == firmwares.end() ){
        std::cerr << "Missing firmware file '" << key_DSPcode << "'" << std::endl;
        return false;
    }

    key_DSPcode = firmwares.find(key_DSPcode)->second;

    if ( firmwares.find(key_DSPVar) == firmwares.end() ){
        std::cerr << "Missing firmware file '" << key_DSPVar << "'" << std::endl;
        return false;
    }

    key_DSPVar = firmwares.find(key_DSPVar)->second;

    // If we reach this point, we know that we have all the firmwares!
    strcpy(ComFPGA, key_Com.c_str());
    strcpy(SPFPGA, key_SPFPGA.c_str());
    strcpy(DSPcode, key_DSPcode.c_str());
    strcpy(DSPVar, key_DSPVar.c_str());

    return true;
}

int BootXIA(const char* settings_file, const fw_map_t& firmwares, const unsigned short& num_modules)
{
    int retval;
    char ComFPGA[2048], SPFPGA[2048], DSPCode[2048], DSPVar[2048];
    char TrigFPGA[] = "trig";
    char DSPSet[2048];
    strcpy(DSPSet, settings_file);

    unsigned short rev[PRESET_MAX_MODULES], bit[PRESET_MAX_MODULES], MHz[PRESET_MAX_MODULES];
    unsigned int sn[PRESET_MAX_MODULES];

    std::cout << "Reading hardware information" << std::endl;
    for (int i = 0 ; i < num_modules ; ++i){
        retval = Pixie16ReadModuleInfo(i, &rev[i], &sn[i], &bit[i], &MHz[i]);
        if (retval < 0){
            std::cerr << "*ERROR* Pixie16ReadModuleInfo failed, retval = " << retval << std::endl;
            return retval;
        }
    }

    for (int i = 0 ; i < num_modules ; ++i){
        if (!GetFirmwareFile(firmwares,rev[i], bit[i], MHz[i],
                             ComFPGA, SPFPGA, DSPCode, DSPVar)) {
            std::cerr << "Module " << i << ": Unknown module" << std::endl;
            return -1000;
        }

        /*switch (MHz[i]) {
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
        }*/

        std::cout << "Booting Pixie-16 module #" << i << ", ";
        std::cout << "Rev=" << rev[i] << ", ";
        std::cout << "S/N=" << sn[i] << ", ";
        std::cout << "Bits=" << bit[i] << ", ";
        std::cout << "MSPS=" << MHz[i] << std::endl;
        std::cout << "ComFPGAConfigFile:\t" << ComFPGA << std::endl;
        std::cout << "SPFPGAConfigFile:\t" << SPFPGA << std::endl;
        std::cout << "DSPCodeFile:\t" << DSPCode << std::endl;
        std::cout << "DSPVarFile:\t" << DSPVar << std::endl;
        retval = Pixie16BootModule(ComFPGA, SPFPGA, TrigFPGA, DSPCode, DSPSet, DSPVar, i, 0x7F);
        std::cout << "----------------------------------------" << std::endl;

        if (retval < 0){
            std::cerr << "*ERROR* Pixie16BootModule failed, retval = " << retval << std::endl;
            return retval;
        }
    }

    std::cout << "All modules booted." << std::endl;
    std::cout << "DSPParFile:\t" << DSPSet << std::endl;
    return 0;
}

int RunListMode(const unsigned short& num_modules, const int &run_no)
{
    // First we will need to synch the modules
    int modnum = 0;
    unsigned int *lmdata;
	char filnam[1024];
	unsigned int mod_numwordsread;
    int retval = Pixie16WriteSglModPar ("SYNCH_WAIT", 1, modnum);
    if( retval < 0 )
        std::cerr << "Synch Wait problem " << retval << std::endl;
    else
        std::cout << "Synch Wait OK " << retval << std::endl;

    retval = Pixie16WriteSglModPar ("IN_SYNCH", 0, modnum);
    if( retval < 0 )
        std::cerr << "In Sync problem " << retval << std::endl;
    else
        std::cout << "In Synch OK " << retval << std::endl;

    retval = Pixie16StartListModeRun (num_modules, 0x100, NEW_RUN);
    if (retval < 0)
    {
        std::cerr << "*ERROR* Pixie16StartListModeRun failed, retval = " << retval << std::endl;
        return -3;
    } else
        std::cout << "List Mode started OK " << retval << std::endl;

    usleep(100000); //delay for the DSP boot

    /*if( (lmdata = static_cast<unsigned int *>(malloc(sizeof(unsigned int) * 131072))) == nullptr)
    {
        printf("failed to allocate memory block lmdata\n");
        return -4;
    }*/

    //////////////////////////////////////////////
	// Acquiring data: here we terminate the loop
	// when the Director module accumulates more than
	// a given number of words in its external FIFO.
	while(1)
	{
		for(int k=0; k < num_modules; k++)
		{
		    snprintf(filnam, sizeof(filnam), "lmdata_r%d_mod%d.bin", run_no, k);
			retval = Pixie16SaveExternalFIFODataToFile(filnam, &mod_numwordsread, k, 0);
			if(retval<0)
			{
				std::cerr << "*ERROR* Pixie16SaveExternalFIFODataToFile failed in module " << k << ", retval = " << retval << std::endl;
				//free(lmdata);
				return -5;
			}
		}

		// Check the run status of the Director module (module #0) to see if the run has been stopped.
		// This is possible in a multi-chassis system where modules in one chassis can stop the run
		// in all chassis.
		retval = Pixie16CheckRunStatus(0);
		if (retval == 0)
		{
			std::cout << "End of run signal..." << std::endl;
			break;
		}
	}

	// Stop run in the Director module (module #0) - a SYNC interrupt should be generated
	// to stop run in all modules simultaneously
	retval = Pixie16EndRun(0);

	// Make sure all modules indeed finish their run successfully.
	for(int k=0; k < num_modules; k++)
	{
		int count = 0;
		do
		{
			retval = Pixie16CheckRunStatus(k);
			if (retval != 0)
			{
	            snprintf(filnam, sizeof(filnam), "lmdata_r%d_mod%d.bin", run_no, k);
				retval = Pixie16SaveExternalFIFODataToFile(filnam, &mod_numwordsread, k, 1);
				if(retval<0)
				{
				    std::cerr << "*ERROR* Pixie16SaveExternalFIFODataToFile failed in module " << k << ", retval = " << retval << std::endl;
					//free(lmdata);
					return -5;
				}
			} else {
				break;
			}

			count ++;
		}while(count < 10);
		if(count == 10)
		{
			std::cerr << "end run in module " << k << " failed" << std::endl;
		}
	}

	// All modules have their run stopped successfully. Now read out the possible last words from the external FIFO
	for(int k=0; k < num_modules; k++)
	{
	    snprintf(filnam, sizeof(filnam), "lmdata_r%d_mod%d.bin", run_no, k);
		retval = Pixie16SaveExternalFIFODataToFile(filnam, &mod_numwordsread, k, 1);
		if(retval<0)
		{
		    std::cerr << "*ERROR* Pixie16SaveExternalFIFODataToFile failed in module " << k << ", retval = " << retval << std::endl;
			return -5;
		}
	}


    return 0;
}

int main() {

    signal(SIGINT, keyb_int);
    signal(SIGPIPE, SIG_IGN);

    auto firmwares = ReadConfig("XIA_Firmware.txt");
    if ( firmwares.empty() ) {
        std::cerr << "Unable to find firmwares, quiting." << std::endl;
        return -1;
    }

    auto mapping = ReadSlotMap();
    if ( mapping.size() >= PRESET_MAX_MODULES ){
        std::string errmsg = "Too many PCI devices found, found " + std::to_string(mapping.size());
        throw std::runtime_error(errmsg);
    }
    unsigned short num_modules = 0;
    for ( auto &entry : mapping ){
        PXIMapping[num_modules++] = entry;
    }
    int retval = Pixie16InitSystem(num_modules, PXIMapping, 0);
    if ( retval < 0 ) {
        std::cerr << "*ERROR* Pixie16InitSystem failed, retval=" << retval << std::endl;
        return -1;
    }

    retval = BootXIA("settings.set", firmwares, num_modules);
    if ( retval < 0 ) {
        std::cerr << "*ERROR* BootXIA failed, retval=" << retval << std::endl;
        for (unsigned short i = 0 ; i < num_modules ; ++i)
            retval = Pixie16ExitSystem(i);
        return -1;
    }

    // Now we can begin the list mode run
    retval = RunListMode(num_modules, 0);
    if ( retval < 0 ) {
        std::cerr << "*ERROR* RunListMode failed, retval=" << retval << std::endl;
    }

    for (unsigned short i = 0 ; i < num_modules ; ++i)
        retval = Pixie16ExitSystem(i);

    return 0;
}