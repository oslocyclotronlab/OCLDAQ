//
// Created by Vetle Wegner Ingeberg on 02/12/2024.
//

#ifndef BUILDALL_XIAINTERFACEV2_H
#define BUILDALL_XIAINTERFACEV2_H

#include "xiainterface.h"

#ifndef PRESET_MAX_MODULES
#define PRESET_MAX_MODULES 24
#endif // PRESET_MAX_MODULES

class XIAInterfaceAPI2 : public XIAInterface
{
public:
    XIAInterfaceAPI2(const size_t &num_modules);

    ModuleInfo_t GetModuleInfo(const size_t &moduleID) const override { return moduleInfo[moduleID]; };

    ChanLim_t GetChnLimits(const size_t &module, const size_t &channel, const char *ChanParName) override;
    ChanPar_t GetChnParam(const size_t &module, const size_t &channel, const char *ChanParName) override;
    void SetChnParam(const size_t &module, const size_t &channel, const char *ChanParName, const ChanPar_t &parameter) override;

    ModLim_t GetModLimits(const size_t &module, const char *ModParName) override;
    ModPar_t GetModParam(const size_t &module, const char *ModParName) override;
    void SetModParam(const size_t &module, const char *ModParName, const ModPar_t &parameter) override;

    unsigned int MeasureBLCut(const unsigned short &module, const unsigned short &channel) override;
    void MeasureBaseline(const unsigned short &module) override;

private:
    struct ModuleInfo_t moduleInfo[PRESET_MAX_MODULES];
};

#endif //BUILDALL_XIAINTERFACEV2_H
