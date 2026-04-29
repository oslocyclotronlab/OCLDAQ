//
// Created by Vetle Wegner Ingeberg on 27/04/2026.
//

#include <iostream>

#include "UserConfiguration.h"

#include <yaml-cpp/yaml.h>
#include "yaml_helpers.h"

#include <logfault/logfault.h>

UserConfiguration UserConfiguration::FromFile(const char *file)
{
    return UserConfiguration(YAML::LoadFile(file));
}

UserConfiguration UserConfiguration::FromFile(std::istream &s)
{
    return UserConfiguration(YAML::Load(s));
}

UserConfiguration::UserConfiguration(const YAML::Node &setup)
    : config_manager( setup )
    , split_time( 1500. )
    , coincidence_time( 1500. )
    , trigger_type( any )
    , sort_type( gap )
{
    try {
        split_time = setup["analysis"]["split_time"].as<double>();
    } catch (std::exception &e) {
        LFLOG_WARN << "Split time is missing or wrongly formatted. Using 1500 ns. Error message: " << e.what() << ".";
        LFLOG_DEBUG << "Error reading 'split_time'. Error: " << e.what() << ".";
        split_time = 1500.;
    }

    try {
        trigger_type = setup["analysis"]["trigger_type"].as<DetectorType>();
    } catch (std::exception &e) {
        LFLOG_WARN << "Trigger type is missing or wrongly formatted. Using 'any'.";
        LFLOG_DEBUG << "Error reading 'trigger_type'. Error: " << e.what() << ".";
        trigger_type = any;
    }

    try {
        sort_type = setup["analysis"]["sort_type"].as<SortType>();
    } catch (std::exception &e) {
        LFLOG_WARN << "Sort type is not configured. Will use 'gap'";
        LFLOG_DEBUG << "Error reading 'sort_type'. Error: " << e.what() << ".";
    }

    try {
        coincidence_time = setup["analysis"]["split_time"].as<double>();
    } catch (std::exception &e) {
        LFLOG_WARN << "Split time is missing or wrongly formatted. Using 1500 ns. Error message: " << e.what() << ".";
        LFLOG_DEBUG << "Error reading 'split_time'. Error: " << e.what() << ".";
        split_time = 1500.;
    }

}

const size_t UserConfiguration::GetNumDetectors(const DetectorType &type) const {
    try {
        return userConfig["setup"]["detectors"][std::string(magic_enum::enum_name(type))].as<size_t>();
    } catch (const std::exception &e) {
        LFLOG_WARN << e.what();
        return 0;
    }
}