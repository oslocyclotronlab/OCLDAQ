#ifndef XIAINTERFACE_H
#define XIAINTERFACE_H

#include <variant>

class XIAInterface
{
private:
    const size_t number_of_modules;
public:
    struct ModuleInfo_t {
        unsigned short revision;
        unsigned short adc_bits;
        unsigned short adc_msps;
        unsigned int serial_number;
    };

    using ChanPar_t = double;
    using ModPar_t = unsigned int;

    using ChanLim_t = std::pair<ChanPar_t, ChanPar_t>;
    using ModLim_t = std::pair<ModPar_t, ModPar_t>;

public:
    XIAInterface(const size_t &num_modules) : number_of_modules( num_modules ){}
    ~XIAInterface();

    size_t GetNumModules() const;
    virtual ModuleInfo_t GetModuleInfo(const size_t &moduleID) const = 0;

    virtual ChanLim_t GetChnLimits(const size_t &module, const size_t &channel, const char *ChanParName) = 0;
    virtual ChanPar_t GetChnParam(const size_t &module, const size_t &channel, const char *ChanParName) = 0;
    virtual void SetChnParam(const size_t &module, const size_t &channel, const char *ChanParName, const ChanPar_t &parameter) = 0;

    virtual ModLim_t GetModLimits(const size_t &module, const char *ModParName) = 0;
    virtual ModPar_t GetModParam(const size_t &module, const char *ModParName) = 0;
    virtual void SetModParam(const size_t &module, const char *ModParName, const ModPar_t &parameter) = 0;
};


class XIAInterfaceMock : public XIAInterface
{
private:
    static ModuleInfo_t modInfo[];
public:
    XIAInterfaceMock(const size_t &num_modules = 4);

    ModuleInfo_t GetModuleInfo(const size_t &moduleID) const override;

    ChanLim_t GetChnLimits(const size_t &module, const size_t &channel, const char *ChanParName) override;
    ChanPar_t GetChnParam(const size_t &module, const size_t &channel, const char *ChanParName) override;
    void SetChnParam(const size_t &module, const size_t &channel, const char *ChanParName, const ChanPar_t &parameter) override;

    ModLim_t GetModLimits(const size_t &module, const char *ModParName) override;
    ModPar_t GetModParam(const size_t &module, const char *ModParName) override;
    void SetModParam(const size_t &module, const char *ModParName, const ModPar_t &parameter) override;

};



#endif // XIAINTERFACE_H
