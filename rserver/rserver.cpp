//
// Created by Vetle Wegner Ingeberg on 28/04/2026.
//

#include <csignal>

#include <histogram/SharedHistograms.h>

#include "options.h"
#include "ROOTServer.h"

#include <THttpServer.h>
#include <TH1.h>
#include <TH2.h>
#include <TH3.h>

char leaveprog = 'n';

void keyb_int(int sig_num)
{
    if (sig_num == SIGINT || sig_num == SIGQUIT || sig_num == SIGTERM) {
        printf("\n\nLeaving...\n");
        leaveprog = 'y';
    }
}

void start_server(const Options_t& options) {
    ROOTServer server(options, leaveprog);
}

int main(int argc, char *argv[]) {
    signal(SIGINT, keyb_int); // set up interrupt handler (Ctrl-C)
    signal(SIGQUIT, keyb_int);
    signal(SIGTERM, keyb_int);
    signal(SIGPIPE, SIG_IGN);

    try {
        auto options = structopt::app("rserver").parse<Options_t>(argc, argv);
        // Set up a log-handler to stdout
        logfault::LogManager::Instance().AddHandler(std::make_unique<logfault::StreamHandler>(std::clog, options.log_level.value()));
        try {
            start_server(options);
        } catch (std::exception& e) {
            LFLOG_ERROR << e.what();
            exit(-1);
        }
    } catch (structopt::exception& e) {
        std::cout << e.what() << "\n";
        std::cout << e.help() << std::endl;
        exit(-1);
    }
    return 0;
}