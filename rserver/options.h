//
// Created by Vetle Wegner Ingeberg on 29/04/2026.
//

#ifndef OPTIONS_H
#define OPTIONS_H

#include <structopt/app.hpp>

#include <string>
#include <logfault/logfault.h>

struct Options_t {

    // HTTP engine
    std::optional<std::string> engine = "http";

    // Optional bind address for the ROOT HTTP server.
    // Default is 0.0.0.0. Maybe not the most secure...
    std::optional<std::string> bind_address = "0.0.0.0";

    // Optional bind port for the ROOT HTTP server.
    // Default is 8686.
    std::optional<size_t> bind_port = 8686;

    // Define log level
    std::optional<logfault::LogLevel> log_level = logfault::LogLevel::INFO;

    // Name of shared histograms
    std::optional<std::string> shared_histograms_name = "XIAonline";
};

STRUCTOPT(Options_t, engine, bind_address, bind_port, log_level, shared_histograms_name);

#endif //OPTIONS_H
