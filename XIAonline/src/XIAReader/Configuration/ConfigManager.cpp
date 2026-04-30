//
// Created by Vetle Wegner Ingeberg on 27/04/2026.
//

#include "ConfigManager.h"

#include <iostream>
#include <magic_enum.hpp>
#include <logfault/logfault.h>

#include <yaml-cpp/yaml.h>
#include "../Tools/yaml_helpers.h"

#include "enumerate.h"

#include "XIA_CFD.h"
#include "xiaformat.h"
#include "entry.h"

#define NUMBER_OF_MODULES_PER_CRATE 16      //! There are 4 bits for denoting module number (slot number)
#define NUMBER_OF_CHANNELS_PER_MODULE 16    //! There are 4 bits for denoting the channel number
#define NUMBER_OF_CRATES 2                  //! For now, we assume two crates. More could be supported...

ConfigManager ConfigManager::FromFile(const char *file)
{
    return ConfigManager(YAML::LoadFile(file));
}

ConfigManager ConfigManager::FromFile(std::istream &s)
{
    return ConfigManager(YAML::Load(s));
}

ConfigManager::ConfigManager(const YAML::Node &setup)
    : dinfo( NUMBER_OF_CRATES * NUMBER_OF_MODULES_PER_CRATE * NUMBER_OF_CHANNELS_PER_MODULE )
{
    for ( auto [num, det] : enumerate(dinfo) ){
        det = {num, XIA::f000MHz, unused, 0};
    }

    try {
        for ( auto &crate : setup["setup"]["crates"] ){
            for ( auto &slot : crate["slots"] ){
                auto freq = slot["speed"].as<XIA::ADCSamplingFreq>();
                for ( auto &channel : slot["detectors"] ){
                    size_t address = crate["crate"].as<size_t>() * NUMBER_OF_MODULES_PER_CRATE * NUMBER_OF_CHANNELS_PER_MODULE;
                    address += slot["slot"].as<size_t>() * NUMBER_OF_CHANNELS_PER_MODULE;
                    address += channel["channel"].as<size_t>();
                    auto type = channel["type"].as<DetectorType>();
                    auto num = channel["detectorID"].as<size_t>();

                    auto quad = setup["calibration"][std::string(magic_enum::enum_name(type))]["quad"][num].as<double>();
                    auto gain = setup["calibration"][std::string(magic_enum::enum_name(type))]["gain"][num].as<double>();
                    auto shift = setup["calibration"][std::string(magic_enum::enum_name(type))]["shift"][num].as<double>();
                    auto time_shift = setup["calibration"][std::string(magic_enum::enum_name(type))]["time_shift"][num].as<double>();

                    dinfo[address] = {address,
                                      freq,
                                      type,
                                      num,
                                      quad,
                                      gain,
                                      shift,
                                      time_shift - static_cast<int>(time_shift),
                                      static_cast<int>(time_shift)};
                }
            }
        }
    } catch (std::exception &e) {
        LFLOG_ERROR << "Error reading setup. Got error: " << e.what() << ".";
        exit(-1);
    }
}


bool ConfigManager::keep(const XIA_base_t* xia) const {
    switch (dinfo.at(xia->index()).type) {
        case labr:
        case deDet:
        case eDet:
        case eGuard:
        case ppac:
        case rfchan:
        case qint:
        case oscarF:
        case oscarB:
            return true;
        default:
            return false;
    }
}

Entry_t ConfigManager::operator()(const XIA_base_t* raw) const {
    auto *detector = &dinfo.at(raw->index());

    int64_t timestamp = raw->timestamp() * (( detector->sfreq == XIA::f250MHz ) ? 8 : 10);

    double energy = detector->CalibrateEnergy(raw->eventEnergy);
    auto [cfdcorr, cfdres] = detector->CalibrateCFD(raw->cfd_result);

    timestamp += detector->timestamp_shift;
    cfdcorr += detector->cfd_shift;

    return {
        detector->type,
        static_cast<unsigned short>(detector->detectorID),
        static_cast<unsigned short>(raw->eventEnergy),
        static_cast<unsigned short>(raw->cfd_result),
        timestamp,
        raw->finishCode,
        energy,
        cfdcorr,
        cfdres,
        getQDC(raw),
        getTrace(raw)
    };
}