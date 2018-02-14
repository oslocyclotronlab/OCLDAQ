#ifndef MEDIALOCATORTESTS_H
#define MEDIALOCATORTESTS_H

#include <cppunit/extensions/HelperMacros.h>

#include <MediaLocator.h>

#include <fstream>
#include <iostream>
#include <cstdio>
#include <string>


class MediaLocatorTests : public CppUnit::TestFixture
{
    private:

    public:
    // Define the test suite
    CPPUNIT_TEST_SUITE( MediaLocatorTests );
    CPPUNIT_TEST ( locatePath_0 );
    CPPUNIT_TEST ( locatePath_1 );
    CPPUNIT_TEST ( locatePath_2 );
    CPPUNIT_TEST ( locatePath_3 );
    CPPUNIT_TEST_SUITE_END();

    public:
    // Begin the standard methods
    void setUp() {
      std::ofstream file("___test_file___");
      file << " ";
      file.close();
    }
    void tearDown() {
      std::remove("___test_file___"); 
    }

    // Begin tests
    void locatePath_0() {
      std::string locatedPath = MediaLocator().locateFile("does not exist");
      CPPUNIT_ASSERT_EQUAL_MESSAGE(
          "Return value is empty string when file does not exist",
          std::string(), locatedPath);
    }

    void locatePath_1() {
      MediaLocator locator;
      locator.addPath(".");

      std::string locatedPath = locator.locateFile("___test_file___");
      CPPUNIT_ASSERT_EQUAL_MESSAGE(
          "Path is returned if found in first directory",
          std::string("./___test_file___"), locatedPath);
    }

    void locatePath_2() {
      MediaLocator locator;
      locator.addPath("asdfasd");
      locator.addPath(".");

      std::string locatedPath = locator.locateFile("___test_file___");
      CPPUNIT_ASSERT_EQUAL_MESSAGE(
          "Path is returned if found in later directory",
          std::string("./___test_file___"), locatedPath);
    }

    void locatePath_3() {
      MediaLocator locator;
      locator.addPath("asdfasd");

      std::string locatedPath = locator.locateFile("___test_file___");
      CPPUNIT_ASSERT_EQUAL_MESSAGE(
          "Empty path is returned if file not found",
          std::string(), locatedPath);
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(MediaLocatorTests);
#endif
