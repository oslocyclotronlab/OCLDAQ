//
// Created by Vetle Wegner Ingeberg on 29/04/2026.
//

#ifndef OPTIONS_H
#define OPTIONS_H

//#include <structopt/app.hpp>

#include <optional>
#include <string>
#include <logfault/logfault.h>

//namespace OCL {
    struct Options_t {

        // HTTP engine
        std::optional<std::string> engine = "http";

        // Optional bind address for the ROOT HTTP server.
        // Default is 0.0.0.0. Maybe not the most secure...
        std::optional<std::string> bind_address = "0.0.0.0";

        // Optional bind port for the ROOT HTTP server.
        // Default is 8686.
        std::optional<size_t> bind_port = 8686;

        // External network data source
        std::optional<std::string> network_source = "0";

        // External network data source port
        std::optional<size_t> network_port = 32008;

        // Configuration file
        std::optional<std::string> configuration = "setup.yml";

        // Define log level
        std::optional<logfault::LogLevel> log_level = logfault::LogLevel::INFO;

        // Name of shared histograms
        std::optional<std::string> shared_histograms_name = "XIAonline";

        // Disable the "Elog" / "Elog All" buttons of the web interface.
        // The server binds to 0.0.0.0 by default, so anyone who can reach it
        // can otherwise post entries to the elog.
        std::optional<bool> disable_elog = false;
    };
//}

#endif //OPTIONS_H
