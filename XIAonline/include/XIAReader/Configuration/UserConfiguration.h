//
// Created by Vetle Wegner Ingeberg on 27/04/2026.
//

#ifndef USERCONFIGURATION_H
#define USERCONFIGURATION_H


#include <iosfwd>

#include "ConfigManager.h"
#include "DetectorTypes.h"
#include "SortTypes.h"


namespace YAML {
    class Node;
}

class UserConfiguration {
public:
    UserConfiguration(const YAML::Node& setup);
    static UserConfiguration FromFile(const char *file);
    static UserConfiguration FromFile(std::istream &);

    [[nodiscard]] ConfigManager GetConfigManager() const;
    [[nodiscard]] double GetSplitTime() const { return split_time; }
    [[nodiscard]] double GetCoincidenceTime() const { return coincidence_time; }
    [[nodiscard]] DetectorType GetTrigger() const { return trigger_type; }
    [[nodiscard]] SortType GetSortType() const { return sort_type; }

private:
    ConfigManager config_manager;
    double split_time;
    double coincidence_time;
    DetectorType trigger_type;
    SortType sort_type;

};

#endif // USERCONFIGURATION_H
