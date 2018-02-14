#include "SystemBooter.h"
#include <Configuration.h>

#include <pixie16app_export.h>
//#include <pixie16sys_export.h>

#include <unistd.h>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <cstring>


using namespace std;

namespace DAQ {
namespace DDAS {


/*!
 * \brief Constructor
 *
 * Enables verbose output by default.
 */
SystemBooter::SystemBooter() : m_verbose(true)
{}

/*!
 * \brief Boot the entire system
 *
 * \param config    a configuration describing the system
 * \param type      style of boot
 *
 * \throws std::runtime_error if Pixie16InitSystem() call returns an error
 * \throws std::runtime_error if populateHardwareMap() throws
 * \throws std::runtime_error if bootModuleByIndex() throws
 *
 * Provided a configuration, all modules will be booted. The configuration contains
 * the firmware files for each hardware type, the slot map, and the number
 * of modules. During the course of booting, the hardware will be queried as
 * well to determine the the serial number, revision, adc frequency, and resolution.
 * The revision, adc frequency, and resolution will all be parsed and the information
 * will be stored in the configuration as a HardwareRegistry::HardwareType.
 *
 */
void SystemBooter::boot(Configuration &config, BootType type)
{
    /****************************************************/
    cout << "------------------------\n";
    cout << "Initializing PXI access... \n";
    cout.flush();

    int NumModules = config.getNumberOfModules();

    int retval = Pixie16InitSystem(NumModules, config.getSlotMap().data(), 0);
    if(retval < 0) {
        std::stringstream errmsg;
        errmsg << "SystemBooter::boot() Failure. Pixie16InitSystem returned = "<< retval << ".";
        throw std::runtime_error(errmsg.str());
    } else {
        cout << "System initialized successfully. " << endl;
    }

    // give the system some time to settle after initialization
    usleep(1000);

    populateHardwareMap(config);

    for (int index=0; index<NumModules; ++index) {
        bootModuleByIndex(index, config, type);
    }

    if (m_verbose) {
        cout << "All modules ok " << endl;
    }

}


/*!
 * \brief Read and store hardware info from each of the modules in the system
 *
 * \param config    the system configuration
 *
 * \throws std::runtime_error if Pixie16ReadModuleInfo returns error code
 *
 * To retrieve information about all of the modules in the system, Pixie16ReadModuleInfo
 * is called for each module index. The resulting revision number, adc bits, and adc
 * frequency is printed (if verbose output enabled) and the hardware mapping is stored
 * in the configuration that was passed in.
 */
void SystemBooter::populateHardwareMap(Configuration &config)
{
    //////////////////////////////////////////////////////////
    //  Read hardware variant information of each Pixie-16 module
    //////////////////////////////////////////////////////////

    unsigned short ModRev;
    unsigned int   ModSerNum;
    unsigned short ModADCBits;
    unsigned short ModADCMSPS;

    int NumModules = config.getNumberOfModules();
    std::vector<int> hdwrMapping(NumModules);

    for(unsigned short k=0; k<NumModules; k++) {
        int retval = Pixie16ReadModuleInfo(k, &ModRev, &ModSerNum, &ModADCBits, &ModADCMSPS);
        if (retval < 0)
        {
            std::stringstream errmsg;
            errmsg << "SystemBooter::boot() Reading hardware variant information (i.e. Pixie16ReadModuleInfo()) failed ";
            errmsg << " for module "<<k<<" with retval=" << retval << ".";
            throw std::runtime_error(errmsg.str());
        } else {

            if (m_verbose) {
                logModuleInfo(k, ModRev, ModSerNum, ModADCBits, ModADCMSPS);
            }

            auto type = HardwareRegistry::computeHardwareType(ModRev, ModADCMSPS, ModADCBits);
            hdwrMapping[k] = type;

        }
    }

    // store the hardware map in the configuration so other components of the program
    // can understand more about the hardware being used.
    config.setHardwareMap(hdwrMapping);
}


/*!
 * \brief Print out some basic information regarding the module
 *
 * \param modIndex      index of the module in the system
 * \param ModRev        hardware revision
 * \param ModSerNum     serial number
 * \param ModADCBits    adc resolution (number of bits)
 * \param ModADCMSPS    adc frequency (MHz)
 */
void SystemBooter::logModuleInfo(int modIndex, unsigned short ModRev,
                                 unsigned short ModSerNum, unsigned short ModADCBits,
                                 unsigned short ModADCMSPS)
{
    std::cout <<"Found Pixie-16 module #"<<modIndex;
    std::cout <<", Rev="<<ModRev;
    std::cout <<", S/N="<<ModSerNum<<", Bits="<<ModADCBits;
    std::cout <<", MSPS="<<ModADCMSPS;
    std::cout << endl;
}


/*!
 * \brief boot a single module
 *
 * \param modIndex  index of the module in the system
 * \param m_config  the system configuration
 * \param type      boot style (load firmware or settings only)
 *
 * \throws std::runtime_error if hardware type is unknown
 * \throws std::runtime_error if Pixie16BootModule returns an error code
 *
 * The system is booted into a usable state. The mechanics of booting involve
 * either loading firmware and settings or just settings, depending on the type
 * parameter that was passed as a second argument to the method. If the user
 * chooses to boot with a firmware load, the firmware files stored in the
 * configuration associated with the hardware will be used. The settings
 * file that will be used in any boot type, will be the path stored in the
 * configuration.
 *
 * \todo Check that the firmware file paths are less than 256 characters in length.
 */
void
SystemBooter::bootModuleByIndex(int modIndex, Configuration& m_config, BootType type)
{

    const size_t FILENAME_STR_MAXLEN = 256;
    char Pixie16_Com_FPGA_File[FILENAME_STR_MAXLEN];
    char Pixie16_SP_FPGA_File[FILENAME_STR_MAXLEN];
    char Pixie16_DSP_Code_File[FILENAME_STR_MAXLEN];
    char Pixie16_DSP_Var_File[FILENAME_STR_MAXLEN];
    char Pixie16_Trig_FPGA_File[FILENAME_STR_MAXLEN];
    char DSPParFile[FILENAME_STR_MAXLEN];

    // Select firmware and dsp files based on hardware variant

    std::vector<int> hdwrMap = m_config.getHardwareMap();

    if (hdwrMap[modIndex] == HardwareRegistry::Unknown) {
        std::stringstream errmsg;
        errmsg <<"Cannot boot module "<< modIndex
              << ". Hardware type not recognized" << std::endl;
        throw std::runtime_error(errmsg.str());
    }

    // Because the Pixie16BootModule takes char* strings, we have to copy our beautiful
    // std::strings into the character arrays. Note that there is no check at the moment
    // to ensure that the firmware file paths are no more than 256 characters.
    FirmwareConfiguration fwConfig = m_config.getFirmwareConfiguration(hdwrMap[modIndex]);
    strcpy(Pixie16_Com_FPGA_File, fwConfig.s_ComFPGAConfigFile.c_str());
    strcpy(Pixie16_SP_FPGA_File,  fwConfig.s_SPFPGAConfigFile.c_str());
    strcpy(Pixie16_DSP_Code_File, fwConfig.s_DSPCodeFile.c_str());
    strcpy(Pixie16_DSP_Var_File,  fwConfig.s_DSPVarFile.c_str());

    strcpy(DSPParFile, m_config.getSettingsFilePath().c_str());

    if (m_verbose) {
        if (type == FullBoot) {
            cout << "\nBooting Pixie-16 module #" << modIndex << endl;
            cout << "\tComFPGAConfigFile:  " << Pixie16_Com_FPGA_File << endl;
            cout << "\tSPFPGAConfigFile:   " << Pixie16_SP_FPGA_File << endl;
            cout << "\tDSPCodeFile:        " << Pixie16_DSP_Code_File << endl;
            cout << "\tDSPVarFile:         " << Pixie16_DSP_Var_File << endl;
            cout << "\tDSPParFile:         " << DSPParFile << endl;
            cout << "--------------------------------------------------------\n\n";
        } else {
            cout << "\nEstablishing communication parameters with module #" << modIndex << std::endl;
            std::cout << "\tSkipping firmware load." << std::endl;
        }
    }

    int retval = Pixie16BootModule (Pixie16_Com_FPGA_File,
                                // Name of communications FPGA config. file
                                Pixie16_SP_FPGA_File,
                                // Name of signal processing FPGA config. file
                                Pixie16_Trig_FPGA_File,
                                // placeholder name of trigger FPGA configuration file
                                Pixie16_DSP_Code_File,
                                // Name of executable code file for digital
                                // signal processor (DSP)
                                DSPParFile,           // Name of DSP parameter file
                                Pixie16_DSP_Var_File, // Name of DSP variable names file
                                modIndex,                    // Pixie module number
                                computeBootMask(type));                // Fast boot pattern bitmask

    if(retval != 0) {
        std::stringstream errmsg;
        errmsg << "Failed for module " << modIndex << " error code " << retval << " !";
        throw std::runtime_error(errmsg.str());
    }

}

/*!
 * \brief Enable or disable verbose output
 *
 * \param enable    enables output messages
 *
 * By default, the output verbosity setting is enabled. If it is disabled,
 * there will be no output printed to the terminal.
 */
void SystemBooter::setVerbose(bool enable)
{
    m_verbose = enable;
}

/*!
 * \return the state of verbosity
 */
bool SystemBooter::isVerbose() const
{
    return m_verbose;
}

/*!
 * \brief Convert BootType enumeration to usable boot mask
 *
 * \param type  either BootType::FullBoot or BootType::SettingsOnly
 *
 * \return 0x7f for FullBoot, 0x70 for SettingsOnly
 *
 * The bootmask is ultimately used in the Pixie16BootModule function.
 */
unsigned int SystemBooter::computeBootMask(BootType type)
{
    if (type == FullBoot) return 0x7f;
    else                  return 0x70;
}


} // end DDAS namespace
} // end DAQ namespace
