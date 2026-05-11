#include "xiaconfigurator.h"
#include "xiainterface_remote.h"

#include <QApplication>
#include <QMessageBox>

#include <iostream>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    bool offline = false;
    QString host;
    quint16 port = 32010;
    for (int i = 1; i < argc; ++i) {
        const QString a = QString::fromUtf8(argv[i]);
        if (a == QLatin1String("--offline")) {
            offline = true;
            continue;
        }
        if (host.isEmpty()) {
            host = a;
            continue;
        }
        bool ok = false;
        const unsigned long p = a.toULong(&ok);
        if (!ok || p == 0 || p > 65535) {
            std::cerr << "Invalid port: " << argv[i] << std::endl;
            return 2;
        }
        port = static_cast<quint16>(p);
    }

    if (host.isEmpty()) {
        std::cerr << "Usage: xiaconfigurator <host> [port] [--offline]\n"
                     "  Connects to XIAengine JSON-RPC (default port 32010).\n"
                     "  If the daemon is unconfigured, performs init_boot (use --offline for offline mode).\n";
        return 2;
    }

    QString err;
    auto iface = XIAInterfaceRemote::create(host, port, offline, &err);
    if (!iface) {
        QMessageBox::critical(nullptr, QStringLiteral("Connection failed"),
                              QStringLiteral("Could not connect: %1").arg(err));
        return 1;
    }

    try {
        XIAConfigurator cfg(iface.get());
        cfg.setWindowTitle(QStringLiteral("XIAConfigurator — %1:%2").arg(host).arg(port));
        cfg.show();
        return app.exec();
    } catch (const std::exception &ex) {
        QMessageBox::critical(nullptr, QStringLiteral("Configurator error"),
                              QString::fromUtf8(ex.what()));
        return 1;
    }
}
