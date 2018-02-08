#include "ConfigurationParser.h"
#include "Configuration.h"

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <vector>

using namespace std;
#define  FILENAME_STR_MAXLEN     256


namespace DAQ {
namespace DDAS {

ConfigurationParser::ConfigurationParser()
 : m_matchExpr(R"(^\[Rev([xXa-fA-F0-9]+)-(\d+)Bit-(\d+)MSPS\]$)")
{}

/*!
 * \brief Parse the contents of the cfgPixie16.txt file
 *
 * \param input     the input stream associated with the cfgPixie16 content (likely an std::ifstream)
 * \param config    a configuration to store the parsed data
 *
 * \throws std::runtime_error if failed to read in sufficient slot map data for number of modules
 * \throws std::runtime_error if settings file does not end in .set
 *
 */
void ConfigurationParser::parse(istream &input, Configuration &config)
{
    int CrateNum;
    std::vector<unsigned short> PXISlotMap;
    int NumModules;

    char temp[FILENAME_STR_MAXLEN];
    std::string line;
    std::string DSPParFile;

    std::string ComFPGAConfigFile_RevBCD;
    std::string SPFPGAConfigFile_RevBCD;
    std::string DSPCodeFile_RevBCD;
    std::string DSPVarFile_RevBCD;

    std::string ComFPGAConfigFile_RevF_250MHz_14Bit;
    std::string SPFPGAConfigFile_RevF_250MHz_14Bit;
    std::string DSPCodeFile_RevF_250MHz_14Bit;
    std::string DSPVarFile_RevF_250MHz_14Bit;

    std::string ComFPGAConfigFile_RevF_500MHz_12Bit;
    std::string SPFPGAConfigFile_RevF_500MHz_12Bit;
    std::string DSPCodeFile_RevF_500MHz_12Bit;
    std::string DSPVarFile_RevF_500MHz_12Bit;


    input >> CrateNum;
    input.getline(temp,FILENAME_STR_MAXLEN);
    input >> NumModules;
    input.getline(temp,FILENAME_STR_MAXLEN);
    PXISlotMap.resize(NumModules);
    for(int i = 0; i < NumModules; i++){
        input >> PXISlotMap[i];
        if (!input.good()) {
            throw std::runtime_error("Failure occurred while reading in slot map data.");
        }
        input.getline(temp,FILENAME_STR_MAXLEN);
    }
    input >> DSPParFile;
    input.getline(temp,FILENAME_STR_MAXLEN);

    //check to make sure that this line contains a set file (.set extension)
    //since the format has changed from previous versions of the code.
    if( DSPParFile.find_last_of(".set") == std::string::npos) {
        std::string errmsg("The file ");
        errmsg += DSPParFile;
        errmsg += " read in from configuration file";
        errmsg += " does not appear to be a *.set file required by DDAS";
        throw std::runtime_error(errmsg);
    }


    // the [100MSPS], [250MSPS], and [500MSPS] tags are still supported but should not be used.
    while (getline(input, line)) {
        if (line == "[100MSPS]") {
            FirmwareConfiguration fwConfig = extractFirmwareConfiguration(input);
            double calibration = extractClockCalibration(input);

            config.setFirmwareConfiguration(HardwareRegistry::RevB_100MHz_12Bit, fwConfig);
            updateClockCalibration(HardwareRegistry::RevB_100MHz_12Bit, calibration);

            config.setFirmwareConfiguration(HardwareRegistry::RevC_100MHz_12Bit, fwConfig);
            updateClockCalibration(HardwareRegistry::RevC_100MHz_12Bit, calibration);

            config.setFirmwareConfiguration(HardwareRegistry::RevD_100MHz_12Bit, fwConfig);
            updateClockCalibration(HardwareRegistry::RevD_100MHz_12Bit, calibration);

        } else if (line == "[250MSPS]"){
            FirmwareConfiguration fwConfig = extractFirmwareConfiguration(input);
            double calibration = extractClockCalibration(input);

            config.setFirmwareConfiguration(HardwareRegistry::RevF_250MHz_14Bit, fwConfig);
            updateClockCalibration(HardwareRegistry::RevF_250MHz_14Bit, calibration);

        } else if (line == "[500MSPS]"){
            FirmwareConfiguration fwConfig = extractFirmwareConfiguration(input);
            double calibration = extractClockCalibration(input);

            config.setFirmwareConfiguration(HardwareRegistry::RevF_500MHz_12Bit, fwConfig);
            updateClockCalibration(HardwareRegistry::RevF_500MHz_12Bit, calibration);

        } else if (std::regex_match(line , m_matchExpr) ) {
            int revision, adcFreq, adcRes;
            if (parseHardwareTypeTag(line, revision, adcFreq, adcRes)) {

                FirmwareConfiguration fwConfig = extractFirmwareConfiguration(input);
                double calibration = extractClockCalibration(input);
                int type = HardwareRegistry::createHardwareType(revision, adcFreq, adcRes, calibration);
                config.setFirmwareConfiguration(type, fwConfig);

            } else {
                std::string msg("ConfigurationParser::parse() Failed to parse ");
                msg += " the hardware tag '" + line + "'";
                throw std::runtime_error(msg);
            }
        }

    }

    config.setCrateId(CrateNum);
    config.setNumberOfModules(NumModules);
    config.setSlotMap(PXISlotMap);
    config.setSettingsFilePath(DSPParFile);
}

void ConfigurationParser::updateClockCalibration(int type, double calibration)
{
    HardwareRegistry::HardwareSpecification& hdwrSpec
            = HardwareRegistry::getSpecification(type);
    hdwrSpec.s_clockCalibration = calibration;
}



/*!
 * \brief parseHardwareTypeTag
 *
 * \param line          the tag to parse
 * \param revision      integer variable to store X into
 * \param freq          integer variable to store Y into
 * \param resolution    integer variable to store Z into
 *
 * Parses the values of X, Y, and Z from a tag of the form [RevX-YBit-ZMSPS].
 *
 * \retval false if line is not in the format [RevX-YBit-ZMSPS]
 * \retval true otherwise
 */
bool ConfigurationParser::parseHardwareTypeTag(const std::string& line,
                                               int &revision, int &freq, int &resolution)
{
    bool result = false;
    std::smatch color_match;
    std::regex_search(line, color_match, m_matchExpr);

    if (color_match.size() == 4) {
      std::string revStr(color_match[1].first, color_match[1].second);
      revision = std::stoi(revStr, 0, 0); // auto detect base
      resolution = std::stoi(std::string(color_match[2].first, color_match[2].second));
      freq = std::stoi(std::string(color_match[3].first, color_match[3].second));
      result = true;
    }
    return result;
}

/*!
 * \brief ConfigurationParser::extractFirmwareConfiguration
 *
 * \param input     the stream to read from
 *
 * The current implementation does not support reading firmware paths with whitespace in them
 *
 * \return a firmware configuration encapsulating the data read from the file.
 *
 * \throw std::runtimer_error if an error occurs while processing next 4 lines
 */
FirmwareConfiguration ConfigurationParser::extractFirmwareConfiguration(std::istream &input)
{
    FirmwareConfiguration fwConfig;
    // load in files to overide defaults

    //load syspixie
    input >> fwConfig.s_ComFPGAConfigFile;
    if (!input.good())
        throw std::runtime_error("Configuration file contains incomplete hardware specification!");

    //load fippipixe
    input >> fwConfig.s_SPFPGAConfigFile;
    if (!input.good())
        throw std::runtime_error("Configuration file contains incomplete hardware specification!");

    //load ldr file
    input >> fwConfig.s_DSPCodeFile;
    if (!input.good())
        throw std::runtime_error("Configuration file contains incomplete hardware specification!");

    //load var file
    input >> fwConfig.s_DSPVarFile;
    if (!input.good())
        throw std::runtime_error("Configuration file contains incomplete hardware specification!");

    return fwConfig;
}

/*!
 * \brief ConfigurationParser::extractClockCalibration
 *
 * \param input the stream to read from
 *
 * \return the clock calibration integer that was read from the file
 *
 * \throw std::runtime_error if an error occurs while processing the next line
 */
double ConfigurationParser::extractClockCalibration(std::istream& input)
{
    double calibration;
    input >> calibration;
    if (!input.good()) {
        throw std::runtime_error("ConfigurationParser attempted to parse an incomplete hardware specification!");
    }
    return calibration;
}



} // end DDAS namespace
} // end DAQ namespace
