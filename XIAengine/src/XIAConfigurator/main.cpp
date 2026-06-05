#include "XIAConfigProtocol.h"
#include "xiaconfigurator.h"
#include "xiainterface_remote.h"

#include <QApplication>

#include <cstdlib>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    const std::string host = argc > 1 ? argv[1] : "localhost";
    const int port = argc > 2 ? std::atoi(argv[2]) : xia_config_protocol::DefaultConfigPort;

    try {
        const auto num_modules = XIAInterfaceRemote::ProbeNumModules(host, port);
        auto interface = std::make_unique<XIAInterfaceRemote>(host, port, num_modules);

        XIAConfigurator configurator(interface.get());
        configurator.show();
        return app.exec();
    } catch (const std::exception &ex) {
        std::cerr << "Failed to start XIA configurator: " << ex.what() << std::endl;
        return EXIT_FAILURE;
    }
}
