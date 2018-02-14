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

#include "FirmwareVersionFileParser.h"
#include "Configuration.h"

#include <sstream>
#include <vector>
#include <string>

using namespace std;
using namespace ::DAQ::DDAS;

/*!
 * \brief Tests for the FirmwareVersionFileParser class
 */
class FirmwareVersionFileParserTest : public CppUnit::TestFixture
{

  public:
    CPPUNIT_TEST_SUITE( FirmwareVersionFileParserTest );
    CPPUNIT_TEST( parse_0a );
    CPPUNIT_TEST( parse_0b );
    CPPUNIT_TEST( parse_0c );
    CPPUNIT_TEST( parse_1a );
    CPPUNIT_TEST( parse_1b );
    CPPUNIT_TEST( parse_1c );
    CPPUNIT_TEST( parse_2a );
    CPPUNIT_TEST( parse_2b );
    CPPUNIT_TEST( parse_2c );
    CPPUNIT_TEST( parse_3a );
    CPPUNIT_TEST( parse_3b );
    CPPUNIT_TEST( parse_3c );
    CPPUNIT_TEST( parse_4 );
    CPPUNIT_TEST( parse_5 );
    CPPUNIT_TEST( parse_6 );
    CPPUNIT_TEST( parse_7 );
    CPPUNIT_TEST( parse_8 );
    CPPUNIT_TEST( parse_9 );
    CPPUNIT_TEST( parse_10 );
    CPPUNIT_TEST( parse_11 );
    CPPUNIT_TEST( parse_12 );
    CPPUNIT_TEST( parse_13 );
    CPPUNIT_TEST( parse_14 );
    CPPUNIT_TEST( parse_15 );
    CPPUNIT_TEST( parse_16 );
    CPPUNIT_TEST( parse_17 );
    CPPUNIT_TEST( parse_18 );
    CPPUNIT_TEST( parse_19 );
    CPPUNIT_TEST( parse_20 );
    CPPUNIT_TEST( parse_21 );
    CPPUNIT_TEST( parse_22 );
    CPPUNIT_TEST( parse_23 );
    CPPUNIT_TEST( parse_24 );
    CPPUNIT_TEST( parse_25 );
    CPPUNIT_TEST( parse_26 );
    CPPUNIT_TEST( parse_27 );
    CPPUNIT_TEST( parse_28 );
    CPPUNIT_TEST( parse_29 );
    CPPUNIT_TEST( parse_30 );
    CPPUNIT_TEST( parse_31 );
    CPPUNIT_TEST_SUITE_END();

    Configuration m_config;
  public:
    void setUp() {
        FirmwareVersionFileParser parser;
        m_config = Configuration();

        stringstream stream(mergeLines(createSampleFileContent()));
        parser.parse(stream, m_config);
    }

    void tearDown() {
    }

    vector<string> createSampleFileContent() {
        vector<string> linesOfFile;
        string line;

        // TOP_SRC_DIR is set in the Makefile using -DTOP_SRC_DIR="@top_srcdir@"
        std::ifstream file(TOP_SRC_DIR "/readout/DDASFirmwareVersions.txt.in", std::ios::in);
        while (1) {
            getline(file, line);
            if (!file.good()) break;
            linesOfFile.push_back(line);
        }
//        // the following is copied in from a real DDASFirmwareVersions.txt file
//        linesOfFile.push_back("####################################################");
//        linesOfFile.push_back("## Only modify the sections below when there is a ##");
//        linesOfFile.push_back("## need to update firmware file names. But please ##");
//        linesOfFile.push_back("## keep all line spacing intact, i.e. do not add  ##");
//        linesOfFile.push_back("## or delete any lines below or add any spaces.   ##");
//        linesOfFile.push_back("##  Just modify certain lines if needed.          ##");
//        linesOfFile.push_back("####################################################");
//        linesOfFile.push_back("#***Rev-B/C/D***");
//        linesOfFile.push_back("#/usr/opt/ddas/firmware/1.0-002/firmware/test100/syspixie16.bin");
//        linesOfFile.push_back("#/usr/opt/ddas/firmware/1.0-002/firmware/test100/fippixie16.bin");
//        linesOfFile.push_back("        ");
//        linesOfFile.push_back("[FPGAFirmwarefiles]");
//        linesOfFile.push_back("");
//        linesOfFile.push_back("***Rev-B/C/D***");
//        linesOfFile.push_back("/usr/opt/ddas/firmware/1.0-002/firmware/syspixie16_current_14b100m.bin");
//        linesOfFile.push_back("/usr/opt/ddas/firmware/1.0-002/firmware/fippixie16_current_14b100m.bin");
//        linesOfFile.push_back("");
//        linesOfFile.push_back("***Rev-F-14Bit-100MSPS***");
//        linesOfFile.push_back("/usr/opt/ddas/firmware/1.0-002/firmware/syspixie16_current_14b100m.bin");
//        linesOfFile.push_back("/usr/opt/ddas/firmware/1.0-002/firmware/fippixie16_current_14b100m.bin");
//        linesOfFile.push_back("");
//        linesOfFile.push_back("***Rev-F-16Bit-100MSPS***");
//        linesOfFile.push_back("/usr/opt/ddas/firmware/1.0-002/firmware/syspixie16_current_16b100m.bin");
//        linesOfFile.push_back("/usr/opt/ddas/firmware/1.0-002/firmware/fippixie16_current_16b100m.bin");
//        linesOfFile.push_back("");
//        linesOfFile.push_back("");
//        linesOfFile.push_back("***Rev-F-12Bit-250MSPS***");
//        linesOfFile.push_back("/usr/opt/ddas/firmware/1.0-002/firmware/syspixie16_current_12b250m.bin");
//        linesOfFile.push_back("/usr/opt/ddas/firmware/1.0-002/firmware/fippixie16_current_12b250m.bin");
//        linesOfFile.push_back("");
//        linesOfFile.push_back("***Rev-F-14Bit-250MSPS***");
//        linesOfFile.push_back("/usr/opt/ddas/firmware/1.0-002/firmware/syspixie16_current_14b250m.bin");
//        linesOfFile.push_back("/usr/opt/ddas/firmware/1.0-002/firmware/fippixie16_current_14b250m.bin");
//        linesOfFile.push_back("");
//        linesOfFile.push_back("***Rev-F-16Bit-250MSPS***");
//        linesOfFile.push_back("/usr/opt/ddas/firmware/1.0-002/firmware/syspixie16_current_16b250m.bin");
//        linesOfFile.push_back("/usr/opt/ddas/firmware/1.0-002/firmware/fippixie16_current_16b250m.bin");
//        linesOfFile.push_back("");
//        linesOfFile.push_back("***Rev-F-12Bit-500MSPS***");
//        linesOfFile.push_back("/usr/opt/ddas/firmware/1.0-002/firmware/syspixie16_current_12b500m.bin");
//        linesOfFile.push_back("/usr/opt/ddas/firmware/1.0-002/firmware/fippixie16_current_12b500m.bin");
//        linesOfFile.push_back("");
//        linesOfFile.push_back("***Rev-F-14Bit-500MSPS***");
//        linesOfFile.push_back("/usr/opt/ddas/firmware/1.0-002/firmware/syspixie16_current_14b500m.bin");
//        linesOfFile.push_back("/usr/opt/ddas/firmware/1.0-002/firmware/fippixie16_current_14b500m.bin");
//        linesOfFile.push_back("");
//        linesOfFile.push_back("");
//        linesOfFile.push_back("");
//        linesOfFile.push_back("");
//        linesOfFile.push_back("[DSPCodefiles]");
//        linesOfFile.push_back("#***Rev-B/C/D***");
//        linesOfFile.push_back("#/usr/opt/ddas/firmware/1.0-002/dsp/test100/Pixie16.ldr");
//        linesOfFile.push_back("#/usr/opt/ddas/firmware/1.0-002/dsp/test100/Pixie16.var");
//        linesOfFile.push_back("");
//        linesOfFile.push_back("***Rev-B/C/D***");
//        linesOfFile.push_back("/usr/opt/ddas/firmware/1.0-002/dsp/Pixie16_current_14b100m.ldr");
//        linesOfFile.push_back("/usr/opt/ddas/firmware/1.0-002/dsp/Pixie16_current_14b100m.var");
//        linesOfFile.push_back("");
//        linesOfFile.push_back("");
//        linesOfFile.push_back("***Rev-F-14Bit-100MSPS***");
//        linesOfFile.push_back("");
//        linesOfFile.push_back("/usr/opt/ddas/firmware/1.0-002/dsp/Pixie16_current_14b100m.ldr");
//        linesOfFile.push_back("/usr/opt/ddas/firmware/1.0-002/dsp/Pixie16_current_14b100m.var");
//        linesOfFile.push_back("");
//        linesOfFile.push_back("***Rev-F-16Bit-100MSPS***");
//        linesOfFile.push_back("/usr/opt/ddas/firmware/1.0-002/dsp/Pixie16_current_16b100m.ldr");
//        linesOfFile.push_back("/usr/opt/ddas/firmware/1.0-002/dsp/Pixie16_current_16b100m.var");
//        linesOfFile.push_back("");
//        linesOfFile.push_back("***Rev-F-12Bit-250MSPS***");
//        linesOfFile.push_back("/usr/opt/ddas/firmware/1.0-002/dsp/Pixie16_current_12b250m.ldr");
//        linesOfFile.push_back("/usr/opt/ddas/firmware/1.0-002/dsp/Pixie16_current_12b250m.var");
//        linesOfFile.push_back("");
//        linesOfFile.push_back("***Rev-F-14Bit-250MSPS***");
//        linesOfFile.push_back("/usr/opt/ddas/firmware/1.0-002/dsp/Pixie16_current_14b250m.ldr");
//        linesOfFile.push_back("/usr/opt/ddas/firmware/1.0-002/dsp/Pixie16_current_14b250m.var");
//        linesOfFile.push_back("");
//        linesOfFile.push_back("***Rev-F-16Bit-250MSPS***");
//        linesOfFile.push_back("/usr/opt/ddas/firmware/1.0-002/dsp/Pixie16_current_16b250m.ldr");
//        linesOfFile.push_back("/usr/opt/ddas/firmware/1.0-002/dsp/Pixie16_current_16b250m.var");
//        linesOfFile.push_back("");
//        linesOfFile.push_back("***Rev-F-12Bit-500MSPS***");
//        linesOfFile.push_back("/usr/opt/ddas/firmware/1.0-002/dsp/Pixie16_current_12b500m.ldr");
//        linesOfFile.push_back("/usr/opt/ddas/firmware/1.0-002/dsp/Pixie16_current_12b500m.var");
//        linesOfFile.push_back("");
//        linesOfFile.push_back("***Rev-F-14Bit-500MSPS***");
//        linesOfFile.push_back("/usr/opt/ddas/firmware/1.0-002/dsp/Pixie16_current_14b500m.ldr");
//        linesOfFile.push_back("/usr/opt/ddas/firmware/1.0-002/dsp/Pixie16_current_14b500m.var");
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

    void parse_0a() {
        FirmwareConfiguration fwConfig = m_config.getFirmwareConfiguration(HardwareRegistry::RevB_100MHz_12Bit);
        EQMSG("RevB common firmware is set up appropriately",
              string("@firmwaredir@/syspixie16_current_14b100m.bin"),
              fwConfig.s_ComFPGAConfigFile);
    }

    void parse_0b() {
        FirmwareConfiguration fwConfig = m_config.getFirmwareConfiguration(HardwareRegistry::RevC_100MHz_12Bit);
        EQMSG("RevC common firmware is set up appropriately",
              string("@firmwaredir@/syspixie16_current_14b100m.bin"),
              fwConfig.s_ComFPGAConfigFile);
    }

    void parse_0c() {
        FirmwareConfiguration fwConfig = m_config.getFirmwareConfiguration(HardwareRegistry::RevD_100MHz_12Bit);
        EQMSG("RevD common firmware is set up appropriately",
              string("@firmwaredir@/syspixie16_current_14b100m.bin"),
              fwConfig.s_ComFPGAConfigFile);
    }

    void parse_1a() {
        FirmwareConfiguration fwConfig = m_config.getFirmwareConfiguration(HardwareRegistry::RevB_100MHz_12Bit);
        EQMSG("RevBCD fippi firmware file is set up appropriately",
              string("@firmwaredir@/fippixie16_current_14b100m.bin"),
              fwConfig.s_SPFPGAConfigFile);
    }

    void parse_1b() {
        FirmwareConfiguration fwConfig = m_config.getFirmwareConfiguration(HardwareRegistry::RevC_100MHz_12Bit);
        EQMSG("RevBCD fippi firmware file is set up appropriately",
              string("@firmwaredir@/fippixie16_current_14b100m.bin"),
              fwConfig.s_SPFPGAConfigFile);
    }

    void parse_1c() {
        FirmwareConfiguration fwConfig = m_config.getFirmwareConfiguration(HardwareRegistry::RevD_100MHz_12Bit);
        EQMSG("RevBCD fippi firmware file is set up appropriately",
              string("@firmwaredir@/fippixie16_current_14b100m.bin"),
              fwConfig.s_SPFPGAConfigFile);
    }

    void parse_2a() {
        FirmwareConfiguration fwConfig = m_config.getFirmwareConfiguration(HardwareRegistry::RevB_100MHz_12Bit);
        EQMSG("RevBCD dsp code file is set up appropriately",
              string("@dspdir@/Pixie16_current_14b100m.ldr"),
              fwConfig.s_DSPCodeFile);
    }

    void parse_2b() {
        FirmwareConfiguration fwConfig = m_config.getFirmwareConfiguration(HardwareRegistry::RevC_100MHz_12Bit);
        EQMSG("RevBCD dsp code file is set up appropriately",
              string("@dspdir@/Pixie16_current_14b100m.ldr"),
              fwConfig.s_DSPCodeFile);
    }

    void parse_2c() {
        FirmwareConfiguration fwConfig = m_config.getFirmwareConfiguration(HardwareRegistry::RevD_100MHz_12Bit);
        EQMSG("RevBCD dsp code file is set up appropriately",
              string("@dspdir@/Pixie16_current_14b100m.ldr"),
              fwConfig.s_DSPCodeFile);
    }



    void parse_3a() {
        FirmwareConfiguration fwConfig = m_config.getFirmwareConfiguration(HardwareRegistry::RevB_100MHz_12Bit);
        EQMSG("RevBCD dsp code file is set up appropriately",
              string("@dspdir@/Pixie16_current_14b100m.ldr"),
              fwConfig.s_DSPCodeFile);
    }

    void parse_3b() {
        FirmwareConfiguration fwConfig = m_config.getFirmwareConfiguration(HardwareRegistry::RevC_100MHz_12Bit);
        EQMSG("RevBCD dsp code file is set up appropriately",
              string("@dspdir@/Pixie16_current_14b100m.ldr"),
              fwConfig.s_DSPCodeFile);
    }

    void parse_3c() {
        FirmwareConfiguration fwConfig = m_config.getFirmwareConfiguration(HardwareRegistry::RevD_100MHz_12Bit);
        EQMSG("RevBCD dsp code file is set up appropriately",
              string("@dspdir@/Pixie16_current_14b100m.ldr"),
              fwConfig.s_DSPCodeFile);
    }

    void parse_4() {
        FirmwareConfiguration fwConfig = m_config.getFirmwareConfiguration(HardwareRegistry::RevF_100MHz_14Bit);
        EQMSG("RevF_100MHz_14Bit common firmware is set up appropriately",
              string("@firmwaredir@/syspixie16_current_14b100m.bin"),
              fwConfig.s_ComFPGAConfigFile);
    }

    void parse_5() {
        FirmwareConfiguration fwConfig = m_config.getFirmwareConfiguration(HardwareRegistry::RevF_100MHz_14Bit);
        EQMSG("RevF_100MHz_14Bit fippi firmware file is set up appropriately",
              string("@firmwaredir@/fippixie16_current_14b100m.bin"),
              fwConfig.s_SPFPGAConfigFile);
    }
    void parse_6() {
        FirmwareConfiguration fwConfig = m_config.getFirmwareConfiguration(HardwareRegistry::RevF_100MHz_14Bit);
        EQMSG("RevF_100MHz_14Bit dsp code file is set up appropriately",
              string("@dspdir@/Pixie16_current_14b100m.ldr"),
              fwConfig.s_DSPCodeFile);
    }

    void parse_7() {
        FirmwareConfiguration fwConfig = m_config.getFirmwareConfiguration(HardwareRegistry::RevF_100MHz_14Bit);
        EQMSG("RevF_100MHz_14Bit dsp var file is set up appropriately",
              string("@dspdir@/Pixie16_current_14b100m.ldr"),
              fwConfig.s_DSPCodeFile);
    }

    void parse_8() {
        FirmwareConfiguration fwConfig = m_config.getFirmwareConfiguration(HardwareRegistry::RevF_100MHz_16Bit);
        EQMSG("RevF_100MHz_16Bit common firmware is set up appropriately",
              string("@firmwaredir@/syspixie16_current_16b100m.bin"),
              fwConfig.s_ComFPGAConfigFile);
    }

    void parse_9() {
        FirmwareConfiguration fwConfig = m_config.getFirmwareConfiguration(HardwareRegistry::RevF_100MHz_16Bit);
        EQMSG("RevF_100MHz_16Bit fippi firmware file is set up appropriately",
              string("@firmwaredir@/fippixie16_current_16b100m.bin"),
              fwConfig.s_SPFPGAConfigFile);
    }

    void parse_10() {
        FirmwareConfiguration fwConfig = m_config.getFirmwareConfiguration(HardwareRegistry::RevF_100MHz_16Bit);
        EQMSG("RevF_100MHz_16Bit dsp code file is set up appropriately",
              string("@dspdir@/Pixie16_current_16b100m.ldr"),
              fwConfig.s_DSPCodeFile);
    }

    void parse_11() {
        FirmwareConfiguration fwConfig = m_config.getFirmwareConfiguration(HardwareRegistry::RevF_100MHz_16Bit);
        EQMSG("RevF_100MHz_14Bit dsp var file is set up appropriately",
              string("@dspdir@/Pixie16_current_16b100m.ldr"),
              fwConfig.s_DSPCodeFile);
    }


    void parse_12() {
        FirmwareConfiguration fwConfig = m_config.getFirmwareConfiguration(HardwareRegistry::RevF_250MHz_12Bit);
        EQMSG("RevF_250MHz_12Bit common firmware is set up appropriately",
              string("@firmwaredir@/syspixie16_current_12b250m.bin"),
              fwConfig.s_ComFPGAConfigFile);
    }

    void parse_13() {
        FirmwareConfiguration fwConfig = m_config.getFirmwareConfiguration(HardwareRegistry::RevF_250MHz_12Bit);
        EQMSG("RevF_250MHz_12Bit fippi firmware file is set up appropriately",
              string("@firmwaredir@/fippixie16_current_12b250m.bin"),
              fwConfig.s_SPFPGAConfigFile);
    }

    void parse_14() {
        FirmwareConfiguration fwConfig = m_config.getFirmwareConfiguration(HardwareRegistry::RevF_250MHz_12Bit);
        EQMSG("RevF_250MHz_12Bit dsp code file is set up appropriately",
              string("@dspdir@/Pixie16_current_12b250m.ldr"),
              fwConfig.s_DSPCodeFile);
    }

    void parse_15() {
        FirmwareConfiguration fwConfig = m_config.getFirmwareConfiguration(HardwareRegistry::RevF_250MHz_12Bit);
        EQMSG("RevF_250MHz_12Bit dsp var file is set up appropriately",
              string("@dspdir@/Pixie16_current_12b250m.ldr"),
              fwConfig.s_DSPCodeFile);
    }

    void parse_16() {
        FirmwareConfiguration fwConfig = m_config.getFirmwareConfiguration(HardwareRegistry::RevF_250MHz_14Bit);
        EQMSG("RevF_250MHz_14Bit common firmware is set up appropriately",
              string("@firmwaredir@/syspixie16_current_14b250m.bin"),
              fwConfig.s_ComFPGAConfigFile);
    }

    void parse_17() {
        FirmwareConfiguration fwConfig = m_config.getFirmwareConfiguration(HardwareRegistry::RevF_250MHz_14Bit);
        EQMSG("RevF_250MHz_14Bit fippi firmware file is set up appropriately",
              string("@firmwaredir@/fippixie16_current_14b250m.bin"),
              fwConfig.s_SPFPGAConfigFile);
    }

    void parse_18() {
        FirmwareConfiguration fwConfig = m_config.getFirmwareConfiguration(HardwareRegistry::RevF_250MHz_14Bit);
        EQMSG("RevF_250MHz_14Bit dsp code file is set up appropriately",
              string("@dspdir@/Pixie16_current_14b250m.ldr"),
              fwConfig.s_DSPCodeFile);
    }

    void parse_19() {
        FirmwareConfiguration fwConfig = m_config.getFirmwareConfiguration(HardwareRegistry::RevF_250MHz_14Bit);
        EQMSG("RevF_250MHz_14Bit dsp var file is set up appropriately",
              string("@dspdir@/Pixie16_current_14b250m.ldr"),
              fwConfig.s_DSPCodeFile);
    }


    void parse_20() {
        FirmwareConfiguration fwConfig = m_config.getFirmwareConfiguration(HardwareRegistry::RevF_250MHz_16Bit);
        EQMSG("RevF_250MHz_16Bit common firmware is set up appropriately",
              string("@firmwaredir@/syspixie16_current_16b250m.bin"),
              fwConfig.s_ComFPGAConfigFile);
    }

    void parse_21() {
        FirmwareConfiguration fwConfig = m_config.getFirmwareConfiguration(HardwareRegistry::RevF_250MHz_16Bit);
        EQMSG("RevF_250MHz_16Bit fippi firmware file is set up appropriately",
              string("@firmwaredir@/fippixie16_current_16b250m.bin"),
              fwConfig.s_SPFPGAConfigFile);
    }

    void parse_22() {
        FirmwareConfiguration fwConfig = m_config.getFirmwareConfiguration(HardwareRegistry::RevF_250MHz_16Bit);
        EQMSG("RevF_250MHz_16Bit dsp code file is set up appropriately",
              string("@dspdir@/Pixie16_current_16b250m.ldr"),
              fwConfig.s_DSPCodeFile);
    }

    void parse_23() {
        FirmwareConfiguration fwConfig = m_config.getFirmwareConfiguration(HardwareRegistry::RevF_250MHz_16Bit);
        EQMSG("RevF_250MHz_16Bit dsp var file is set up appropriately",
              string("@dspdir@/Pixie16_current_16b250m.ldr"),
              fwConfig.s_DSPCodeFile);
    }

    void parse_24() {
        FirmwareConfiguration fwConfig = m_config.getFirmwareConfiguration(HardwareRegistry::RevF_500MHz_12Bit);
        EQMSG("RevF_500MHz_12Bit common firmware is set up appropriately",
              string("@firmwaredir@/syspixie16_current_12b500m.bin"),
              fwConfig.s_ComFPGAConfigFile);
    }

    void parse_25() {
        FirmwareConfiguration fwConfig = m_config.getFirmwareConfiguration(HardwareRegistry::RevF_500MHz_12Bit);
        EQMSG("RevF_500MHz_12Bit fippi firmware file is set up appropriately",
              string("@firmwaredir@/fippixie16_current_12b500m.bin"),
              fwConfig.s_SPFPGAConfigFile);
    }

    void parse_26() {
        FirmwareConfiguration fwConfig = m_config.getFirmwareConfiguration(HardwareRegistry::RevF_500MHz_12Bit);
        EQMSG("RevF_500MHz_12Bit dsp code file is set up appropriately",
              string("@dspdir@/Pixie16_current_12b500m.ldr"),
              fwConfig.s_DSPCodeFile);
    }

    void parse_27() {
        FirmwareConfiguration fwConfig = m_config.getFirmwareConfiguration(HardwareRegistry::RevF_500MHz_12Bit);
        EQMSG("RevF_500MHz_16Bit dsp var file is set up appropriately",
              string("@dspdir@/Pixie16_current_12b500m.ldr"),
              fwConfig.s_DSPCodeFile);
    }

    void parse_28() {
        FirmwareConfiguration fwConfig = m_config.getFirmwareConfiguration(HardwareRegistry::RevF_500MHz_14Bit);
        EQMSG("RevF_500MHz_14Bit common firmware is set up appropriately",
              string("@firmwaredir@/syspixie16_current_14b500m.bin"),
              fwConfig.s_ComFPGAConfigFile);
    }

    void parse_29() {
        FirmwareConfiguration fwConfig = m_config.getFirmwareConfiguration(HardwareRegistry::RevF_500MHz_14Bit);
        EQMSG("RevF_500MHz_14Bit fippi firmware file is set up appropriately",
              string("@firmwaredir@/fippixie16_current_14b500m.bin"),
              fwConfig.s_SPFPGAConfigFile);
    }

    void parse_30() {
        FirmwareConfiguration fwConfig = m_config.getFirmwareConfiguration(HardwareRegistry::RevF_500MHz_14Bit);
        EQMSG("RevF_500MHz_14Bit dsp code file is set up appropriately",
              string("@dspdir@/Pixie16_current_14b500m.ldr"),
              fwConfig.s_DSPCodeFile);
    }

    void parse_31() {
        FirmwareConfiguration fwConfig = m_config.getFirmwareConfiguration(HardwareRegistry::RevF_500MHz_14Bit);
        EQMSG("RevF_500MHz_16Bit dsp var file is set up appropriately",
              string("@dspdir@/Pixie16_current_14b500m.ldr"),
              fwConfig.s_DSPCodeFile);
    }

    };

// Register it with the test factory
CPPUNIT_TEST_SUITE_REGISTRATION( FirmwareVersionFileParserTest );

