#include "FirmwareVersionFileParser.h"
#include "Configuration.h"

#include <string>
#include <iostream>
#include <algorithm>
#include <iterator>


namespace {
    // The total number of hardware types we expect at the NSCL
    const int TOTAL_PIXIE16_VARIANTS = 8;
}

namespace DAQ {
namespace DDAS {


FirmwareVersionFileParser::FirmwareVersionFileParser()
 : m_matchExpr(R"(^\[Rev([xXa-fA-F0-9]+)-(\d+)Bit-(\d+)MSPS\]$)")
{}


/*!
 * \brief Main entry point for parsing DDASFirmwareVersions.txt file
 *
 * \param input     the stream to read file content from
 * \param config    the configuration to store data in
 *
 * Any firmware configurations that were stored in the configuration object
 * before this will be overwritten with new content.
 */
void FirmwareVersionFileParser::parse(std::istream &input, Configuration &config)
{

    // these will overwrite any existing firmware configurations with an empty configuration.
    config.setFirmwareConfiguration(HardwareRegistry::RevB_100MHz_12Bit, FirmwareConfiguration());
    config.setFirmwareConfiguration(HardwareRegistry::RevC_100MHz_12Bit, FirmwareConfiguration());
    config.setFirmwareConfiguration(HardwareRegistry::RevD_100MHz_12Bit, FirmwareConfiguration());
    config.setFirmwareConfiguration(HardwareRegistry::RevF_100MHz_14Bit, FirmwareConfiguration());
    config.setFirmwareConfiguration(HardwareRegistry::RevF_100MHz_16Bit, FirmwareConfiguration());
    config.setFirmwareConfiguration(HardwareRegistry::RevF_250MHz_12Bit, FirmwareConfiguration());
    config.setFirmwareConfiguration(HardwareRegistry::RevF_250MHz_14Bit, FirmwareConfiguration());
    config.setFirmwareConfiguration(HardwareRegistry::RevF_250MHz_16Bit, FirmwareConfiguration());
    config.setFirmwareConfiguration(HardwareRegistry::RevF_500MHz_12Bit, FirmwareConfiguration());
    config.setFirmwareConfiguration(HardwareRegistry::RevF_500MHz_14Bit, FirmwareConfiguration());

    // read input file with code provided by XIA using XIA defined formatted file

    for(std::string line; std::getline(input, line,'\n');) {

        if (std::regex_match(line , m_matchExpr) ) {
            std::smatch color_match;
            std::regex_search(line, color_match, m_matchExpr);
            int revision = std::stoi(std::string(color_match[1].first, color_match[1].second),0,0);
            int adcRes = std::stoi(std::string(color_match[2].first, color_match[2].second));
            int adcFreq = std::stoi(std::string(color_match[3].first, color_match[3].second));
            int calibration;

            FirmwareConfiguration fwConfig;
            input >> fwConfig.s_ComFPGAConfigFile;
            if (!input.good()) throw std::runtime_error("DDASFirmwareVersionFile.txt is incomplete!");

            input >> fwConfig.s_SPFPGAConfigFile;
            if (!input.good()) throw std::runtime_error("DDASFirmwareVersionFile.txt is incomplete!");

            input >> fwConfig.s_DSPCodeFile;
            if (!input.good()) throw std::runtime_error("DDASFirmwareVersionFile.txt is incomplete!");

            input >> fwConfig.s_DSPVarFile;
            if (!input.good()) throw std::runtime_error("DDASFirmwareVersionFile.txt is incomplete!");

            input >> calibration;
            if (!input.good()) throw std::runtime_error("DDASFirmwareVersionFile.txt is incomplete!");

            int type = HardwareRegistry::createHardwareType(revision, adcFreq, adcRes, calibration);
            config.setFirmwareConfiguration(type, fwConfig);
        }

    }
}

} // end DAQ
} // end DDAS
