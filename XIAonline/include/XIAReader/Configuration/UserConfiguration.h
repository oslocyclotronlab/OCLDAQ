//
// Created by Vetle Wegner Ingeberg on 27/04/2026.
//

#ifndef USERCONFIGURATION_H
#define USERCONFIGURATION_H


#include <iosfwd>

#include <Configuration/ConfigManager.h>
#include <Configuration/DetectorTypes.h>
#include <Configuration/SortTypes.h>

#include <yaml-cpp/yaml.h>

class UserConfiguration {
public:
    UserConfiguration(const YAML::Node& setup);
    static UserConfiguration FromFile(const char *file);
    static UserConfiguration FromFile(std::istream &);

    [[nodiscard]] ConfigManager GetConfigManager() const { return config_manager; }
    [[nodiscard]] const YAML::Node GetRawConfig() const { return userConfig; }
    [[nodiscard]] double GetSplitTime() const { return split_time; }
    [[nodiscard]] double GetCoincidenceTime() const { return coincidence_time; }
    [[nodiscard]] DetectorType GetTrigger() const { return trigger_type; }
    [[nodiscard]] SortType GetSortType() const { return sort_type; }
    [[nodiscard]] const size_t GetNumDetectors(const DetectorType& type) const;

private:
    const YAML::Node userConfig;
    ConfigManager config_manager;
    double split_time;
    double coincidence_time;
    DetectorType trigger_type;
    SortType sort_type;

};

#endif // USERCONFIGURATION_H
