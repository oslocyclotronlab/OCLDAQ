
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

#include "Configuration.h"

#include <sstream>
#include <vector>
#include <string>

using namespace std;
using namespace ::DAQ::DDAS;


/*!
 * \brief Test some basic logic of the Configuration class
 */
class ConfigurationTest : public CppUnit::TestFixture
{

  public:
    CPPUNIT_TEST_SUITE( ConfigurationTest );
    CPPUNIT_TEST( print_0 );
    CPPUNIT_TEST( setModEvtLength_0 );
    CPPUNIT_TEST( setModEvtLength_1 );
    CPPUNIT_TEST( setSlotMap_0 );
    CPPUNIT_TEST( setSlotMap_1 );
    CPPUNIT_TEST( setHardwareMap_0 );
    CPPUNIT_TEST( setHardwareMap_1 );
    CPPUNIT_TEST( setHardwareMap_2 );
    CPPUNIT_TEST_SUITE_END();

  public:
    void setUp() {
    }

    void tearDown() {
    }


    void print_0() {
        Configuration config;
        config.setNumberOfModules(2);
        config.setCrateId(123);
        config.setModuleEventLengths({123,345});
        config.setSlotMap({2,3});
        config.setSettingsFilePath("/path/to/settings.file");

        std::stringstream stream;
        config.print(stream);

        EQMSG("Print output",
              std::string("Crate number 123: 2 modules, in slots:2 3 DSPParFile: /path/to/settings.file"),
              stream.str());
    }

    void setModEvtLength_0() {
        Configuration config;
        CPPUNIT_ASSERT_THROW_MESSAGE("settings modevtlen b/4 setNumberOfModules is an error",
                                     config.setModuleEventLengths({0}),
                                        std::runtime_error);
    }

    void setModEvtLength_1() {
        Configuration config;
        config.setNumberOfModules(2);
        CPPUNIT_ASSERT_NO_THROW_MESSAGE("settings modevtlen correctly succeeds",
                                     config.setModuleEventLengths({0, 2}));
    }

    void setSlotMap_0() {
        Configuration config;
        CPPUNIT_ASSERT_THROW_MESSAGE("settings slot map b/4 setNumberOfModules is an error",
                                     config.setSlotMap({0}),
                                        std::runtime_error);
    }

    void setSlotMap_1() {
        Configuration config;
        config.setNumberOfModules(2);
        CPPUNIT_ASSERT_NO_THROW_MESSAGE("settings slot map correctly succeeds",
                                     config.setSlotMap({0, 2}));
    }


    void setHardwareMap_0() {

        using namespace DAQ::DDAS::HardwareRegistry;

        Configuration config;
        config.setNumberOfModules(2);
        CPPUNIT_ASSERT_NO_THROW_MESSAGE("setting hdwr map correctly succeeds",
                                     config.setHardwareMap({RevB_100MHz_12Bit, RevF_250MHz_14Bit}));
    }

    void setHardwareMap_1() {

        using namespace DAQ::DDAS::HardwareRegistry;

        Configuration config;
        CPPUNIT_ASSERT_THROW_MESSAGE("setting hdwr map b/4 setNumberOfModules is an error",
                                     config.setHardwareMap({RevD_100MHz_12Bit, RevF_250MHz_14Bit}),
                                     std::runtime_error);
    }

    void setHardwareMap_2() {

        using namespace DAQ::DDAS::HardwareRegistry;

        Configuration config;
        config.setNumberOfModules(1);

        std::vector<int> mapping = {RevC_100MHz_12Bit};
        config.setHardwareMap(mapping);

        ASSERTMSG("setting hdwr map actually creates change in map",
                  mapping == config.getHardwareMap());

    }

};


// Register it with the test factory
CPPUNIT_TEST_SUITE_REGISTRATION( ConfigurationTest );


