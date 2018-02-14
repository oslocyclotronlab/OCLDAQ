#ifndef FIRMWAREVERSIONFILEPARSER_H
#define FIRMWAREVERSIONFILEPARSER_H

#include <iosfwd>
#include <regex>

namespace DAQ {
namespace DDAS {

class Configuration;

/*!
 * \brief The FirmwareVersionFileParser class
 *
 * The FirmwareVersionFileParser is designed to parse the DDASFirmwareVersions.txt
 * file that is installed by the project. The DDASFirmwareVersions.txt file has
 * two major sections. The top section contains fpga firmware file paths and the
 * bottom section provides the paths to dsp configuration code. The format of this
 * file can be observed in \ref DDASFirmwareVersions.txt.in , which is the template
 * file used by automake to generate the DDASFirmwareVersion.txt file.
 *
 * Ultimately, the contents of the DDASFirmwareVersions.txt file will be stored in
 * the Configuration object passed in as an argument to the parse() method. That
 * object will keep a database of the firmware files organized by their associated
 * hardware type.
 *
 */
class FirmwareVersionFileParser
{
    std::regex m_matchExpr;

    // docs are provided in the source file. Doxygen output is provided as
    // user facing documentation.

public:
    FirmwareVersionFileParser();

    void parse(std::istream& input, Configuration& config);

};



} // end DDAS namespace
} // end DAQ namespace

#endif // FIRMWAREVERSIONFILEPARSER_H
