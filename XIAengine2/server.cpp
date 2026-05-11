//
// Created by Vetle Wegner Ingeberg on 11/05/2026.
//


#include <net_control.h>

void connect(line_channel *lc, void*) {
    line_sender ls(lc);
    ls << "Hi and welcome!" << '\n';
}

void disconnect(line_channel *lc, void*) {
    lc.
}

int main() {

    io_select ioc;

    line_server server(ioc, 8282, "server", );

    return 0;

}