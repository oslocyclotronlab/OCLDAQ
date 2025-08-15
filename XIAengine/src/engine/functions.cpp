#include "functions.h"

#include <pixie16app_export.h>
#include <pixie16app_defs.h>

#include <string>
#include <array>
#include <map>
#include <sstream>
#include <memory>
#include <iostream>
#include <fstream>

#include <INIReader.h>

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

// ##############################################
// Some helper functions for the mapping function
// ##############################################

typedef struct {
    int PCIBusNum;
    int PCIDevNum;
} pci_addr_t;

bool operator<(const pci_addr_t &lhs, const pci_addr_t &rhs);
std::string exec(const char* cmd);
std::map<pci_addr_t, unsigned short> GetMapping(const char *ini_file = "pxisys.ini");

std::vector<unsigned short> ReadSlotMap(const char *ini_file)
{
    auto map = GetMapping(ini_file);
    auto devices = exec("lspci | grep \"PLX Technology, Inc. PCI9054 32-bit 33MHz PCI <-> IOBus Bridge (rev 0b)\"");
    std::vector<unsigned short> slotMap;

    // Next we need to decode the output. We read the string, line by line.
    std::istringstream stream(devices);
    std::string line;
    std::vector<pci_addr_t> addresses;
    int min_pci = 255;
    while ( std::getline(stream, line) ){
        // First we need to decode the first two characters
        std::string PCIBusNumber(line.begin(), line.begin()+2);
        std::string PCIDevNumber(line.begin()+3, line.begin()+5);

        int busNum = std::stoi(PCIBusNumber, nullptr, 16);
        int devNum = std::stoi(PCIDevNumber, nullptr, 16);
        addresses.push_back({busNum, devNum});
        min_pci = ( busNum < min_pci ) ? busNum : min_pci;
    }

    // If the min PCI bus found is larger than the minimum PCI bus in the config file,
    // we will need figure out the offset
    int min_plx = 255;
    for ( auto &m : map ){
        min_plx =  ( m.first.PCIBusNum < min_plx ) ? m.first.PCIBusNum : min_plx;
    }

    int offset = min_pci - min_plx;
    for ( auto &addr : addresses ){
        // sometimes there may be a busOffset, if so, we will
        if ( map.find({addr.PCIBusNum-offset, addr.PCIDevNum}) == map.end() ){
            std::string errmsg = "Could not find PCI bus " + std::to_string(addr.PCIBusNum-offset);
            errmsg += ", dev " + std::to_string(addr.PCIDevNum);
            throw std::runtime_error(errmsg);
        }
        slotMap.push_back(map[{addr.PCIBusNum-offset, addr.PCIDevNum}]);
    }

    std::sort(std::begin(slotMap), std::end(slotMap));
    return slotMap;
}

bool operator<(const pci_addr_t &lhs, const pci_addr_t &rhs)
{
    if ( lhs.PCIBusNum == rhs.PCIBusNum )
        return ( lhs.PCIDevNum < rhs.PCIDevNum );
    return (lhs.PCIBusNum < rhs.PCIBusNum);
}

std::string exec(const char* cmd) {
    std::array<char, 128> buffer{};
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return result;
}

std::map<pci_addr_t, unsigned short> GetMapping(const char *ini_file)
{
    INIReader reader(ini_file);
    std::string slot_name;

    if ( reader.ParseError() != 0 ){
        std::string errmsg = "Could not load '" + std::string(ini_file) + "'";
        throw std::runtime_error(errmsg);
    }
    std::map<pci_addr_t, unsigned short> map;
    for ( unsigned short slot = 2 ; slot <= 14 ; ++slot ){
        slot_name = "Slot" + std::to_string(slot);
        int busNum = reader.GetInteger(slot_name, "PCIBusNumber", -1);
        int devNum = reader.GetInteger(slot_name, "PCIDeviceNumber", -1);
        if ( busNum < 0 || devNum < 0 ){
            std::string errmsg = "Could read Bus/Dev number for section '" + std::string(slot_name) + "'";
            throw std::runtime_error(errmsg);
        }
        map[{busNum, devNum}] = slot;
    }
    return map;
}


std::string p_strip(const std::string& s)
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

        std::string key = p_strip(line.substr(0, pos_eq));
        std::string val = p_strip(line.substr(pos_eq+1));

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