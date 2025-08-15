#include "xiainterface.h"

#include <exception>
#include <stdexcept>

#include <iostream>


XIAInterface::ModuleInfo_t XIAInterfaceMock::modInfo[] = {
    {0xF, 16, 100, 1000},
    {0xF, 16, 250, 1001},
    {0xF, 16, 500, 1002},
    {0xF, 14, 500, 1003}
};


XIAInterface::~XIAInterface()
{
}

size_t XIAInterface::GetNumModules() const { return number_of_modules; }


XIAInterfaceMock::XIAInterfaceMock(const size_t &num_modules)
    : XIAInterface( num_modules )
{}


XIAInterface::ModuleInfo_t XIAInterfaceMock::GetModuleInfo(const size_t &moduleID) const
{
    if ( moduleID < GetNumModules() )
        return modInfo[moduleID];
    else
        throw std::runtime_error("Cannot get module info.");
}

XIAInterface::ChanLim_t XIAInterfaceMock::GetChnLimits(const size_t &module, const size_t &channel, const char *ChanParName)
{
    return {ChanPar_t(0), ChanPar_t(10)};
}

XIAInterface::ChanPar_t XIAInterfaceMock::GetChnParam(const size_t &module, const size_t &channel, const char *ChanParName)
{
    return ChanPar_t(5);
}

void XIAInterfaceMock::SetChnParam(const size_t &module, const size_t &channel, const char *ChanParName, const ChanPar_t &parameter)
{
    return;
}


XIAInterface::ModLim_t XIAInterfaceMock::GetModLimits(const size_t &module, const char *ModParName)
{
    return {ModPar_t(0), ModPar_t(1)};
}
XIAInterface::ModPar_t XIAInterfaceMock::GetModParam(const size_t &module, const char *ModParName){
    return ModPar_t(0);
}

void XIAInterfaceMock::SetModParam(const size_t &module, const char *ModParName, const XIAInterface::ModPar_t &parameter)
{
    std::cout << "Module: " << module << " Parameter '" << ModParName << "': " << parameter << std::endl;
    return;
}
