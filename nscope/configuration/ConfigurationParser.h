#ifndef CONFIGURATIONPARSER_H
#define CONFIGURATIONPARSER_H

#include <iosfwd>
#include <regex>

namespace DAQ {
namespace DDAS {

class Configuration;
class FirmwareConfiguration;


/*!
 * \brief The ConfigurationParser class
 *
 * The ConfigurationParser class is designed to parse the contents of
 * the cfgPixie16.txt file. This file is pretty basic. There are two
 * main sections to it. The top section is mandatory and contains information
 * about the slot map, crate id, and settings file path. The bottom half
 * specifies possible overloads to the firmware configuration of the RevBCD,
 * RevF_250MHz_14Bit, and RevF_500MHz_12Bit hardware types. It has the
 * following form:
 *
 * \verbatim
 * CRATE_ID
 * NUM_MODULES
 * SLOT_MODULE_0
 * SLOT_MODULE_1
 * ...
 * SLOT_MODULE_N-1
 * PATH_TO_SETTINGS_FILE
 *
 * [RevX-YBit-ZMSPS]
 * ComFPGACONFIG_FILE
 * SPFPGACONFIG_FILE
 * DSPCODE_FILE
 * DSPVAR_FILE
 * Clock_Calibration
 *
 * [100MSPS]
 * ComFPGACONFIG_FILE
 * SPFPGACONFIG_FILE
 * DSPCODE_FILE
 * DSPVAR_FILE
 * Clock_Calibration
 * [250MSPS]
 * ComFPGACONFIG_FILE
 * SPFPGACONFIG_FILE
 * DSPCODE_FILE
 * DSPVAR_FILE
 * Clock_Calibration
 * [500MSPS]
 * ComFPGACONFIG_FILE
 * SPFPGACONFIG_FILE
 * DSPCODE_FILE
 * DSPVAR_FILE
 * Clock_Calibration
 * \endverbatim
 *
 * where CRATE_ID is a non-negative number, NUM_MODULES is a positive number,
 * SLOT_MODULE_# is a number greater than or equal to 2, and PATH_TO_SETTINGS_FILE
 * is an legitimate path. In the top section, the parser will ignore up to 256 characters
 * following the leftmost integer or string found on each line. Because of this,
 * it is customary to add notes on each of these lines. There is no convention
 * for adding notes, though man people like to use a #. An example would be (note the varying
 * conventions for demonstration):
 *
 * \verbatim
 * 1    # crate id
 * 2    number of modules
 * 2    | slot of first module
 * 3    - slot of second module
 * /path/to/setfile.set ! another comment
 * \endverbatim
 *
 * For the second half, the user can specify firmware information for specific hardware
 * types using a tag. The general format is [RevX-YBit-ZMSPS], where X is a revision number,
 * Y is the adc resolution, and Z is the adc frequency. Besides that general format, there
 * are three more tags ([100MSPS], [250MSPS], or [500MSPS]) that can be used. These latter
 * tags are being phased out and should no longer be used. Every tag that is provided _must_
 * be followed by five lines, each containing the ComFPGAConfig,
 * SPFPGAConfig, DSPCode, and DSPVar files and the clock calibration in order. The clock
 * calibration should be a floating point value that is used to convert a raw timestamp in
 * ticks to a time in nanoseconds.
 *
 * The ConfigurationParser can be used in the following fashion:
 *
 * \code
 * using namespace DAQ::DDAS;
 *
 * Configuration config;
 * CnfigurationParser parser;
 *
 * std::ifstream configFile("cfgPixie16.txt", std::ios::in);
 *
 * parser.parse(configFile, config);
 *
 * \endcode
 *
 */
class ConfigurationParser
{
private:
    std::regex m_matchExpr;

public:

    ConfigurationParser();

    void parse(std::istream& input, Configuration& config);
    bool parseHardwareTypeTag(const std::string& line, int& revision, int& freq, int& resolution);
    FirmwareConfiguration extractFirmwareConfiguration(std::istream &input);
    double extractClockCalibration(std::istream &input);
    void updateClockCalibration(int type, double calibration);
};


} // end DDAS namespace
} // end DAQ namespace


#endif // CONFIGURATIONPARSER_H
