#include "xiacontrol.h"
#include <QApplication>

#include <fstream>

#include <cereal/archives/json.hpp>

#include "XIASetup.h"

/*int main()
{
    XIASetup setup;

    setup.numMod = 5;
    for (int i = 0 ; i < 5 ; ++i){
        setup.PXISlotMap[i] = 2+i;
    }

    setup.offlineMode = 0;
    setup.SetFileName = "TestFile.set";
    setup.StartSync = true;
    setup.FirmwareFile = "Firmware.set";
    {
        std::ofstream out("out.json");
        cereal::JSONOutputArchive archive(out);
        int some_int;
        double v;
        archive( CEREAL_NVP(setup),
                 some_int,
                 cereal::make_nvp("test", v));
    }

    return 0;

}*/
#include <iostream>
#include <thread>
#include <signal.h>
char leaveprog='n';

void keyb_int(int sig_num)
{
    if (sig_num == SIGINT) {
        printf("\n\nLeaving...\n");
        leaveprog = 'y';
    }
}

int runApp(int argc, char *argv[])
{
    QApplication a(argc, argv);
    XIAControl w;
    w.show();

    return a.exec();
}

int main(int argc, char *argv[])
{
    std::thread graphicThread(runApp, argc, argv);
    signal(SIGINT, keyb_int);
    /*while (leaveprog=='n'){
        std::cout << "Something..." << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }*/

    if (graphicThread.joinable())
        graphicThread.join();
    return 0;
}
