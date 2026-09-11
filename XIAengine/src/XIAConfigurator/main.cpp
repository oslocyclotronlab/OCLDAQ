#include "XIAConfigProtocol.h"
#include "xiaconfigurator.h"
#include "xiainterface_remote.h"

#include <QApplication>
#include <QInputDialog>

#include <cstdlib>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    std::string host;
    if ( argc > 1 ) {
        host = argv[1];
    } else {
        bool ok = false;
        QString address = QInputDialog::getText(nullptr, QObject::tr("Connect to XIA server"),
            QObject::tr("Server IP address:"), QLineEdit::Normal, QString(), &ok);
        if ( !ok || address.isEmpty() ) {
            std::cerr << "No server IP address given." << std::endl;
            return EXIT_FAILURE;
        }
        host = address.toStdString();
    }
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
