/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2016.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Jeromy Tompkins
         NSCL
         Michigan State University
         East Lansing, MI 48824-1321
*/


#include <cppunit/extensions/HelperMacros.h>

#include "Asserts.h"

#include "ConfigurationParser.h"
#include "Configuration.h"

#include <sstream>
#include <vector>
#include <string>

using namespace std;
using namespace ::DAQ::DDAS;

namespace HR = ::DAQ::DDAS::HardwareRegistry;

template<class T>
std::ostream& operator<<(std::ostream& stream, const std::vector<T>& vec)
{
    stream << "(";
    for (auto& number : vec) {
        stream << number << " ";
    }
    stream << ")";

    return stream;
}

// A test suite
class ConfigurationParserTest : public CppUnit::TestFixture
{

  public:
    CPPUNIT_TEST_SUITE( ConfigurationParserTest );
    CPPUNIT_TEST( parseCfgPixieFile_0 );
    CPPUNIT_TEST( parseCfgPixieFile_1 );
    CPPUNIT_TEST( parseCfgPixieFile_2 );
    CPPUNIT_TEST( parseCfgPixieFile_3 );
    CPPUNIT_TEST( parseCfgPixieFile_4 );
    CPPUNIT_TEST( parseCfgPixieFile_5a );
    CPPUNIT_TEST( parseCfgPixieFile_5b );
    CPPUNIT_TEST( parseCfgPixieFile_5c );
    CPPUNIT_TEST( parseCfgPixieFile_6 );
    CPPUNIT_TEST( parseCfgPixieFile_7 );
    CPPUNIT_TEST( parseCfgPixieFile_8 );
    CPPUNIT_TEST( generate_0 );
    CPPUNIT_TEST( parseGeneralTag_0 );
    CPPUNIT_TEST( parseGeneralTag_1 );
    CPPUNIT_TEST( parseGeneralTag_2 );
    CPPUNIT_TEST_SUITE_END();

    vector<string> m_cfgFileContent;

  public:
    void setUp() {
        m_cfgFileContent = createSampleFileContent();
    }

    void tearDown() {
    }

    vector<string> createSampleFileContent() {
        vector<string> linesOfFile;

        linesOfFile.push_back("0   # crate id");
        linesOfFile.push_back("3   # number of modules");
        linesOfFile.push_back("2");
        linesOfFile.push_back("3");
        linesOfFile.push_back("4");
        linesOfFile.push_back("/path/to/my/settings/file.set");

        linesOfFile.push_back("[100MSPS]");
        linesOfFile.push_back("a");
        linesOfFile.push_back("b");
        linesOfFile.push_back("c");
        linesOfFile.push_back("d");
        linesOfFile.push_back("2.1");
        linesOfFile.push_back("[250MSPS]");
        linesOfFile.push_back("e");
        linesOfFile.push_back("f");
        linesOfFile.push_back("g");
        linesOfFile.push_back("h");
        linesOfFile.push_back("23.1");
        linesOfFile.push_back("[500MSPS]");
        linesOfFile.push_back("i");
        linesOfFile.push_back("j");
        linesOfFile.push_back("k");
        linesOfFile.push_back("l");
        linesOfFile.push_back("232.1");
        linesOfFile.push_back(""); // empty lines should be skipped
        linesOfFile.push_back("[Rev24-145Bit-23234MSPS]");
        linesOfFile.push_back(" m"); // leading space should be skipped
        linesOfFile.push_back("     n"); // leading tab sould be skipped
        linesOfFile.push_back("o "); // trailing space should be trimmed
        linesOfFile.push_back("p    "); // trailing tab should be trimmed
        linesOfFile.push_back("2323.1"); // clock calibration

        return linesOfFile;
    }

    string mergeLines(const vector<string>& content) {

        string mergedContent;

        for (auto& line : content) {
            mergedContent += line + '\n';
        }

        return mergedContent;
    }

    string createSampleStream() {
        return mergeLines(createSampleFileContent());
    }

    void parseCfgPixieFile_0() {

        ConfigurationParser parser;
        std::stringstream stream(createSampleStream());

        Configuration config;
        parser.parse(stream, config);

        EQMSG("Crate id is parsed correctly", 0, config.getCrateId());
    }

    void parseCfgPixieFile_1() {

        ConfigurationParser parser;
        std::stringstream stream(createSampleStream());
        Configuration config;
        parser.parse(stream, config);

        EQMSG("Number of modules is parsed correctly", size_t(3),
              config.getNumberOfModules());
    }

    void parseCfgPixieFile_2() {

        ConfigurationParser parser;
        std::stringstream stream(createSampleStream());
        Configuration config;
        parser.parse(stream, config);

        EQMSG("Slot mapping is parsed correctly", vector<unsigned short>({2,3,4}),
              config.getSlotMap());
    }

    void parseCfgPixieFile_3() {

        ConfigurationParser parser;
        std::stringstream stream(createSampleStream());

        Configuration config;
        parser.parse(stream, config);
        EQMSG("Path to set file is parsed correctly",
              string("/path/to/my/settings/file.set"),
              config.getSettingsFilePath());
    }


    void parseCfgPixieFile_4() {
        ConfigurationParser parser;
        auto lines = createSampleFileContent();
        lines.at(1) = "4"; // oops we changed the number of modules without adding to slot map

        Configuration config;

        std::string message;
        bool threwException = false;

        stringstream stream(mergeLines(lines));
        try {
            parser.parse(stream, config);
        } catch (std::exception& exc) {
            threwException = true;
            message = exc.what();
        }

        ASSERTMSG("Failure should occur if insufficient slot mapping data exists",
                  threwException);
        EQMSG("Error message should be informative",
              message, string("Failure occurred while reading in slot map data."));
    }

    void parseCfgPixieFile_5a() {
        ConfigurationParser parser;
        std::stringstream stream(createSampleStream());

        Configuration config;
        parser.parse(stream, config);

        FirmwareConfiguration fwConfig = config.getFirmwareConfiguration(HR::RevB_100MHz_12Bit);
        auto& spec = HR::getSpecification(HR::RevB_100MHz_12Bit);
        EQ(string("a"), fwConfig.s_ComFPGAConfigFile);
        EQ(string("b"), fwConfig.s_SPFPGAConfigFile);
        EQ(string("c"), fwConfig.s_DSPCodeFile);
        EQ(string("d"), fwConfig.s_DSPVarFile);
        EQMSG("calibration", 2.1, spec.s_clockCalibration);
    }

    void parseCfgPixieFile_5b() {
        ConfigurationParser parser;
        std::stringstream stream(createSampleStream());

        Configuration config;
        parser.parse(stream, config);

        FirmwareConfiguration fwConfig = config.getFirmwareConfiguration(HR::RevC_100MHz_12Bit);
        auto& spec = HR::getSpecification(HR::RevC_100MHz_12Bit);
        EQ(string("a"), fwConfig.s_ComFPGAConfigFile);
        EQ(string("b"), fwConfig.s_SPFPGAConfigFile);
        EQ(string("c"), fwConfig.s_DSPCodeFile);
        EQ(string("d"), fwConfig.s_DSPVarFile);
        EQMSG("calibration", 2.1, spec.s_clockCalibration);

    }

    void parseCfgPixieFile_5c() {
        ConfigurationParser parser;
        std::stringstream stream(createSampleStream());

        Configuration config;
        parser.parse(stream, config);

        FirmwareConfiguration fwConfig = config.getFirmwareConfiguration(HR::RevD_100MHz_12Bit);
        auto& spec = HR::getSpecification(HR::RevD_100MHz_12Bit);
        EQ(string("a"), fwConfig.s_ComFPGAConfigFile);
        EQ(string("b"), fwConfig.s_SPFPGAConfigFile);
        EQ(string("c"), fwConfig.s_DSPCodeFile);
        EQ(string("d"), fwConfig.s_DSPVarFile);
        EQMSG("calibration", 2.1, spec.s_clockCalibration);
    }

    void parseCfgPixieFile_6() {
        ConfigurationParser parser;
        std::stringstream stream(createSampleStream());

        Configuration config;
        parser.parse(stream, config);

        FirmwareConfiguration fwConfig = config.getFirmwareConfiguration(HR::RevF_250MHz_14Bit);
        auto& spec = HR::getSpecification(HR::RevF_250MHz_14Bit);
        EQ(string("e"), fwConfig.s_ComFPGAConfigFile);
        EQ(string("f"), fwConfig.s_SPFPGAConfigFile);
        EQ(string("g"), fwConfig.s_DSPCodeFile);
        EQ(string("h"), fwConfig.s_DSPVarFile);
        EQMSG("calibration", 23.1, spec.s_clockCalibration);
    }

    void parseCfgPixieFile_7() {
        ConfigurationParser parser;
        std::stringstream stream(createSampleStream());

        Configuration config;
        parser.parse(stream, config);

        FirmwareConfiguration fwConfig = config.getFirmwareConfiguration(HR::RevF_500MHz_12Bit);
        auto& spec = HR::getSpecification(HR::RevF_500MHz_12Bit);
        EQ(string("i"), fwConfig.s_ComFPGAConfigFile);
        EQ(string("j"), fwConfig.s_SPFPGAConfigFile);
        EQ(string("k"), fwConfig.s_DSPCodeFile);
        EQ(string("l"), fwConfig.s_DSPVarFile);
        EQMSG("calibration", 232.1, spec.s_clockCalibration);
    }

    void parseCfgPixieFile_8() {
        ConfigurationParser parser;
        std::stringstream stream(createSampleStream());

        Configuration config;
        parser.parse(stream, config);

        int type = HR::computeHardwareType(24,23234,145);
        FirmwareConfiguration fwConfig = config.getFirmwareConfiguration(type);
        auto& spec = HR::getSpecification(type);
        EQ(string("m"), fwConfig.s_ComFPGAConfigFile);
        EQ(string("n"), fwConfig.s_SPFPGAConfigFile);
        EQ(string("o"), fwConfig.s_DSPCodeFile);
        EQ(string("p"), fwConfig.s_DSPVarFile);
        EQMSG("calibration", 2323.1, spec.s_clockCalibration);
    }

    // these will be fairly sparse tests because as long as the call to
    // generate tests does not fail, we have the basic functionality for
    // each of the parsers used under test already
    void generate_0 () {
        std::string fwPath = TOP_SRC_DIR "/readout/DDASFirmwareVersions.txt.in";
        std::string cfgPath = TOP_SRC_DIR "/readout/crate_1/cfgPixie16.txt";
        std::string modEvtLenPath = TOP_SRC_DIR "/readout/crate_1/modevtlen.txt";

        CPPUNIT_ASSERT_NO_THROW_MESSAGE("generate method should succeed without a problem",
                                        Configuration::generate(fwPath, cfgPath, modEvtLenPath));

    }

    void parseGeneralTag_0() {

        // parse a tag that provides the revision as a hexadecimal value in lower case

        std::string tag = "[Rev0xfa2-23Bit-723MSPS]";
        ConfigurationParser parser;
        int revision, frequency, resolution;
        bool success = parser.parseHardwareTypeTag(tag, revision, frequency, resolution);
        EQMSG("tag successfully parses", true, success);
        EQMSG("revision", 4002, revision);
        EQMSG("frequency", 723, frequency);
        EQMSG("resolution", 23, resolution);

    }

    void parseGeneralTag_1() {

        // parse a tag that provides the revision as a hexadecimal value in mixed case

        std::string tag = "[Rev0xFa2-23Bit-723MSPS]";
        ConfigurationParser parser;
        int revision, frequency, resolution;
        bool success = parser.parseHardwareTypeTag(tag, revision, frequency, resolution);
        EQMSG("tag successfully parses", true, success);
        EQMSG("revision", 4002, revision);
        EQMSG("frequency", 723, frequency);
        EQMSG("resolution", 23, resolution);

    }


    void parseGeneralTag_2() {

        // parse a tag that provides the revision as a decimal value

        std::string tag = "[Rev234-23Bit-723MSPS]";
        ConfigurationParser parser;
        int revision, frequency, resolution;
        bool success = parser.parseHardwareTypeTag(tag, revision, frequency, resolution);
        EQMSG("tag successfully parses", true, success);
        EQMSG("revision", 234, revision);
        EQMSG("frequency", 723, frequency);
        EQMSG("resolution", 23, resolution);

    }

};

// Register it with the test factory
CPPUNIT_TEST_SUITE_REGISTRATION( ConfigurationParserTest );

