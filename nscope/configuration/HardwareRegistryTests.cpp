
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

#include "HardwareRegistry.h"

#include <sstream>
#include <vector>
#include <string>

using namespace std;
namespace HR = ::DAQ::DDAS::HardwareRegistry;

/*!
 * \brief Tests for the ModEvtFileParser class
 */
class HardwareRegistryTest : public CppUnit::TestFixture
{

  public:
    CPPUNIT_TEST_SUITE( HardwareRegistryTest );
    CPPUNIT_TEST( resetToDefaults_0 );
    CPPUNIT_TEST( getSpecification_0a );
    CPPUNIT_TEST( getSpecification_0b );
    CPPUNIT_TEST( getSpecification_0c );
    CPPUNIT_TEST( getSpecification_1 );
    CPPUNIT_TEST( getSpecification_2 );
    CPPUNIT_TEST( getSpecification_3 );
    CPPUNIT_TEST( getSpecification_4 );
    CPPUNIT_TEST( getSpecification_5 );
    CPPUNIT_TEST( getSpecification_6 );
    CPPUNIT_TEST( getSpecification_7 );
    CPPUNIT_TEST( configureHardwareType_0 );
    CPPUNIT_TEST( computeHardwareType_0 );
    CPPUNIT_TEST( computeHardwareType_1 );
    CPPUNIT_TEST( computeHardwareType_2 );
    CPPUNIT_TEST( computeHardwareType_3 );
    CPPUNIT_TEST( computeHardwareType_4 );
    CPPUNIT_TEST( computeHardwareType_5 );
    CPPUNIT_TEST( computeHardwareType_6 );
    CPPUNIT_TEST( computeHardwareType_7 );
    CPPUNIT_TEST( computeHardwareType_8 );
    CPPUNIT_TEST( computeHardwareType_9 );
    CPPUNIT_TEST( computeHardwareType_10 );
    CPPUNIT_TEST( createHardwareType_0 );
    CPPUNIT_TEST( computeHardwareType_11 );
    CPPUNIT_TEST( createHardwareType_1 );
    CPPUNIT_TEST_SUITE_END();

  public:
    void setUp() {
    }

    void tearDown() {
        HR::resetToDefaults();
    }

    // this test should be kept as the very first test of them all
    // because if it does not pass, then all remaining tests are
    // subject to being dependent on the previous test
    void resetToDefaults_0() {
        // change the definition of revb
        HR::configureHardwareType(HR::RevB_100MHz_12Bit, { 430, 23 });

        // rest the registry
        HR::resetToDefaults();

        // check revb tot be what we expect
        getSpecification_0a();

    }

    void getSpecification_0a() {
        auto spec = HR::getSpecification(HR::RevB_100MHz_12Bit);
        EQMSG("revb default rev", 11, spec.s_hdwrRevision);
        EQMSG("revb default adc freq", 100, spec.s_adcFrequency);
        EQMSG("revb default adc resolution", 12, spec.s_adcResolution);
    }

    void getSpecification_0b() {
        auto spec = HR::getSpecification(HR::RevC_100MHz_12Bit);
        EQMSG("revc default rev", 12, spec.s_hdwrRevision);
        EQMSG("revc default adc freq", 100, spec.s_adcFrequency);
        EQMSG("revc default adc resolution", 12, spec.s_adcResolution);
    }

    void getSpecification_0c() {
        auto spec = HR::getSpecification(HR::RevD_100MHz_12Bit);
        EQMSG("revd default rev", 13, spec.s_hdwrRevision);
        EQMSG("revd default adc freq", 100, spec.s_adcFrequency);
        EQMSG("revd default adc resolution", 12, spec.s_adcResolution);
    }

    void getSpecification_1() {
        auto spec = HR::getSpecification(HR::RevF_100MHz_14Bit);
        EQMSG("revf_100_14bit default adc freq", 100, spec.s_adcFrequency);
        EQMSG("revf_100_14bit default adc resolution", 14, spec.s_adcResolution);
    }

    void getSpecification_2() {
        auto spec = HR::getSpecification(HR::RevF_100MHz_16Bit);
        EQMSG("revf_100_16bit default adc freq", 100, spec.s_adcFrequency);
        EQMSG("revf_100_16bit default adc resolution", 16, spec.s_adcResolution);
    }

    void getSpecification_3() {
        auto spec = HR::getSpecification(HR::RevF_250MHz_12Bit);
        EQMSG("revf_250_12bit default adc freq", 250, spec.s_adcFrequency);
        EQMSG("revf_250_12bit default adc resolution", 12, spec.s_adcResolution);
    }

    void getSpecification_4() {
        auto spec = HR::getSpecification(HR::RevF_250MHz_14Bit);
        EQMSG("revf_250_14bit default adc freq", 250, spec.s_adcFrequency);
        EQMSG("revf_250_14bit default adc resolution", 14, spec.s_adcResolution);
    }

    void getSpecification_5() {
        auto spec = HR::getSpecification(HR::RevF_250MHz_16Bit);
        EQMSG("revf_250_16bit default adc freq", 250, spec.s_adcFrequency);
        EQMSG("revf_250_16bit default adc resolution", 16, spec.s_adcResolution);
    }

    void getSpecification_6() {
        auto spec = HR::getSpecification(HR::RevF_500MHz_12Bit);
        EQMSG("revf_500_12bit default adc freq", 500, spec.s_adcFrequency);
        EQMSG("revf_500_12bit default adc resolution", 12, spec.s_adcResolution);
    }

    void getSpecification_7() {
        auto spec = HR::getSpecification(HR::RevF_500MHz_14Bit);
        EQMSG("revf_500_14bit default adc freq", 500, spec.s_adcFrequency);
        EQMSG("revf_500_14bit default adc resolution", 14, spec.s_adcResolution);
    }

    void configureHardwareType_0() {
        HR::configureHardwareType(HR::RevB_100MHz_12Bit, { 430, 23, 2 });

        auto spec = HR::getSpecification(HR::RevB_100MHz_12Bit);
        EQMSG("after configure, adc freq", 430, spec.s_adcFrequency);
        EQMSG("after configure, adc resolution", 23, spec.s_adcResolution);
        EQMSG("after configure, hdwr revision", 2, spec.s_hdwrRevision);
    }


    void computeHardwareType_0() {
        EQMSG("Compute RevD", int(HR::RevD_100MHz_12Bit), HR::computeHardwareType(13, 100, 12));
    }

    void computeHardwareType_1() {
        EQMSG("Compute RevF_100MHz_14Bit",
              int(HR::RevF_100MHz_14Bit), HR::computeHardwareType(15, 100, 14));
    }

    void computeHardwareType_2() {
        EQMSG("Compute RevF_100MHz_16Bit",
              int(HR::RevF_100MHz_16Bit), HR::computeHardwareType(15, 100, 16));
    }

    void computeHardwareType_3() {
        EQMSG("Compute RevF_250MHz_12Bit",
              int(HR::RevF_250MHz_12Bit), HR::computeHardwareType(15, 250, 12));
    }

    void computeHardwareType_4() {
        EQMSG("Compute RevF_250MHz_14Bit",
              int(HR::RevF_250MHz_14Bit), HR::computeHardwareType(15, 250, 14));
    }

    void computeHardwareType_5() {
        EQMSG("Compute RevF_250MHz_16Bit",
              int(HR::RevF_250MHz_16Bit), HR::computeHardwareType(15, 250, 16));
    }

    void computeHardwareType_6() {
        EQMSG("Compute RevF_500MHz_12Bit",
              int(HR::RevF_500MHz_12Bit), HR::computeHardwareType(15, 500, 12));
    }

    void computeHardwareType_7() {
        EQMSG("Compute RevF_500MHz_14Bit",
              int(HR::RevF_500MHz_14Bit), HR::computeHardwareType(15, 500, 14));
    }

    void computeHardwareType_8() {
        EQMSG("Compute Unknown", int(HR::Unknown), HR::computeHardwareType(15, 1000, 12));
    }

    void computeHardwareType_9() {
        EQMSG("Compute RevB", int(HR::RevB_100MHz_12Bit), HR::computeHardwareType(11, 100, 12));
    }

    void computeHardwareType_10() {
        EQMSG("Compute RevC", int(HR::RevC_100MHz_12Bit), HR::computeHardwareType(12, 100, 12));
    }

    void computeHardwareType_11() {
        int type = HR::createHardwareType(34, 343, 232, 42);

        int foundType = HR::computeHardwareType(34,343,232);
        EQMSG("new hardware type", type, foundType);
    }


    void createHardwareType_0() {
        int type = HR::createHardwareType(34, 343, 232, 42);
        EQMSG("new hardware type", 100, type); 
    }

    void createHardwareType_1() {
        int type1 = HR::createHardwareType(34, 343, 232, 42);
        int type2 = HR::createHardwareType(34, 343, 232, 42);

        EQMSG("duplicate types don't happen", type1, type2);
    }


};


// Register it with the test factory
CPPUNIT_TEST_SUITE_REGISTRATION( HardwareRegistryTest );


