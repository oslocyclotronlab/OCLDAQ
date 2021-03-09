//
// Created by Vetle Wegner Ingeberg on 11/02/2021.
//

#include "FirmwareMap.h"

#include <INIReader.h>

#include <string>
#include <iostream>

std::string to_string(const FirmwareMap_t::key_t &key)
{
    std::string str = "rev " + std::to_string(key.rev);
    str += ", " + std::to_string(key.bit) + "-bit, ";
    str += ", " + std::to_string(key.mhz) + " MHz";
    return str;
}

bool operator<(const FirmwareMap_t::key_t &lhs, const FirmwareMap_t::key_t &rhs)
{
    if ( lhs.rev == rhs.rev ){
        if ( lhs.bit == rhs.bit ){
            return lhs.mhz < rhs.mhz;
        } else {
            return lhs.bit < rhs.bit;
        }
    } else {
        return lhs.rev < rhs.rev;
    }
}

FirmwareMap_t::FirmwareMap_t(const char *file)
{
    INIReader reader(file);
    if ( reader.ParseError() != 0 ){
        std::string errmsg = "Error parsing file '" + std::string(file) + "'";
        throw std::runtime_error(errmsg);
    }

    const auto& sections = reader.Sections();
    for ( auto &section : sections ){
        key_t key{};
        value_t value{};
        key.rev = reader.GetInteger(section, "rev", 0);
        key.bit = reader.GetInteger(section, "bit", 0);
        key.mhz = reader.GetInteger(section, "mhz", 0);
        value.comFPGA = reader.Get(section, "comFPGA", "");
        value.SPFPGA = reader.Get(section, "SPFPGA", "");
        value.DSPcode = reader.Get(section, "DSPCode", "");
        value.DSPvar = reader.Get(section, "DSPCode", "");

        if ( !key.is_valid() ){
            std::cerr << "Section " << section << " in file " << file << " is invalid." << std::endl;
            continue;
        }
        if ( fwmap.find(key) != fwmap.end() ){
            std::cerr << "Duplicated entries for module " + to_string(key) << std::endl;
        }
        fwmap[key] = value;
    }
}

FirmwareMap_t::value_t FirmwareMap_t::FindFirmware(const key_t &key) const
{
    auto found = fwmap.find(key);
    if ( found == fwmap.end() ){
        std::string errstr = "Could not find firmware for module with key '" + to_string(key);
        throw std::runtime_error(errstr);
    }
    return found->second;
}