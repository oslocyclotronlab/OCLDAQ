#ifndef XIAINTERFACE_REMOTE_H
#define XIAINTERFACE_REMOTE_H

#include "xiainterface.h"

#include <memory>
#include <string>
#include <vector>

class XIAInterfaceRemote : public XIAInterface {
public:
    static size_t ProbeNumModules(const std::string &host, int port);

    XIAInterfaceRemote(const std::string &host, int port, size_t num_modules);
    ~XIAInterfaceRemote() override;

    ModuleInfo_t GetModuleInfo(const size_t &moduleID) const override;

    ChanLim_t GetChnLimits(const size_t &module, const size_t &channel, const char *ChanParName) override;
    ChanPar_t GetChnParam(const size_t &module, const size_t &channel, const char *ChanParName) override;
    void SetChnParam(const size_t &module, const size_t &channel, const char *ChanParName, const ChanPar_t &parameter) override;

    ModLim_t GetModLimits(const size_t &module, const char *ModParName) override;
    ModPar_t GetModParam(const size_t &module, const char *ModParName) override;
    void SetModParam(const size_t &module, const char *ModParName, const ModPar_t &parameter) override;

    unsigned int MeasureBLCut(const unsigned short &module, const unsigned short &channel) override;
    void MeasureBaseline(const unsigned short &module) override;

    int CopyDSPParameters(const unsigned short &BitMap, const unsigned short &sourceModule,
                          const unsigned short &sourceChannel, unsigned short *DestinationMask) override;

    bool WriteSettings(const char *fname) override;

private:
    class ConfigClient;

    std::unique_ptr<ConfigClient> client_;
    std::vector<ModuleInfo_t> moduleInfo_;
};

#endif // XIAINTERFACE_REMOTE_H
