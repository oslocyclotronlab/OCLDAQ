#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>

int main( int argc, char* argv[])
{
    // Generate the TestRunner
    CppUnit::TextUi::TestRunner runner;
    CppUnit::TestFactoryRegistry &registry = CppUnit::TestFactoryRegistry::getRegistry();
    
    // Add the test suites
    runner.addTest( registry.makeTest() );

    // Run the tests
    runner.run();

    return 0;
}
