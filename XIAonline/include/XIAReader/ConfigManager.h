//
// Created by Vetle Wegner Ingeberg on 27/04/2026.
//

#ifndef CONFIGMANAGER_H
#define CONFIGMANAGER_H

#include <vector>
#include <iosfwd>

#include "DetectorTypes.h"
#include "XIA_CFD.h"

namespace YAML {
    class Node;
}

struct XIA_base_t;
struct Entry_t;


class ConfigManager {
private:
    struct DetectorInfo_t {
        size_t address;
        enum XIA::ADCSamplingFreq sfreq;
        enum DetectorType type;
        size_t detectorID;

        double quad, gain, shift;
        double cfd_shift;
        long long timestamp_shift;

        [[nodiscard]] double CalibrateEnergy(const unsigned short& channel) const {
            double ch = channel + (drand48() - 0.5);
            return quad * ch*ch + gain * ch + shift;
        }

        [[nodiscard]] XIA::XIA_CFD_t CalibrateCFD(const unsigned short& channel) const {
            return XIA::XIA_CFD_Decode(sfreq, uint16_t(channel));
        }
    };

private:
    std::vector<DetectorInfo_t> dinfo;

public:
    explicit ConfigManager(const YAML::Node& setup);
    static ConfigManager FromFile(const char *file);
    static ConfigManager FromFile(std::istream &);

    bool keep(const XIA_base_t* xia) const;
    Entry_t operator()(const XIA_base_t* raw) const;
};


#endif // CONFIGMANAGER_H
