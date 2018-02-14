#include "ModEvtFileParser.h"
#include "Configuration.h"

#include <iostream>

namespace DAQ {
namespace DDAS {


/*!
 * \brief Parse and store the contents of the modevtlen.txt file in a configuration object
 *
 * \param input     the stream from which the contents of the file are read
 * \param config    the configuration to store the results in
 *
 * \throws std::runtime_error if fewer lines than there are modules are found
 * \throws std::runtime_error if a line is encountered with a value less than 4
 *
 * The parser will read in as many lines as the value returned by
 * config.getNumberOfModules(). For that reason, the caller must have already
 * set the number of modules in the configuration object.
 */
void ModEvtFileParser::parse(std::istream &input, Configuration &config)
{
    int NumModules = config.getNumberOfModules();
    std::vector<int> modEvtLenData(NumModules);

    for(int i=0; i<NumModules; i++) {
        input >> modEvtLenData[i];

        if (input.fail() || input.bad()) {
            std::string errmsg("Failure while reading module event length ");
            errmsg += "configuration file. ";
            errmsg += "Expected " + std::to_string(NumModules) + " entries but found ";
            errmsg += "only " + std::to_string(i) + ".";
            throw std::runtime_error(errmsg);
        }
        if (modEvtLenData[i] < 4) {
            std::string errmsg("Failure while reading module event length ");
            errmsg += "configuration file. Found event length less than 4.";
            throw std::runtime_error(errmsg);
        }
    }

    config.setModuleEventLengths(modEvtLenData);
}

} // end DDAS namespace
} // end DAQ namespace
