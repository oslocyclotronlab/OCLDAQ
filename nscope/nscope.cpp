#include "Main.h"
#include <iostream>
#include <stdexcept>

using namespace std;

int
main (int argc, char **argv)
{
    try {
        TApplication theApp ("App", &argc, argv);

        Main mainWindow (gClient->GetRoot ());

        theApp.Run ();
    } catch (std::exception& exc) {
        std::cout << "Caught an exception : " << exc.what() << std::endl;
        return 1;
    }
    return 0;
}
