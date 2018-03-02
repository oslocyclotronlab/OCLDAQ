#ifndef XIASETUP_H
#define XIASETUP_H

// This object stores all the previous settings of
// the application.

#include <string>

//#include <cereal/types/array.hpp>

#ifndef PIXIE16APP_EXPORT_H
#define PRESET_MAX_MODULES 24
#endif // PIXIE16APP_EXPORT_H


struct XIASetup
{
    // Number of modules.
    unsigned int numMod;

    // PXI mapping of the modules.
    unsigned short PXISlotMap[PRESET_MAX_MODULES];

    // Full path to the settings file.
    std::string SetFileName;

    // Full path to firmware file.
    std::string FirmwareFile;

    // If offline/online mode
    unsigned short offlineMode;

    // Start all modules synchronized.
    bool StartSync;

    // Synchronize the modules at start of run.
    bool SynchMod;

    template<class Archive>
    void serialize(Archive &archive)
    {
        archive(numMod, PXISlotMap, SetFileName, FirmwareFile, offlineMode, StartSync, SynchMod);
    }

};

#endif // XIASETUP_H
