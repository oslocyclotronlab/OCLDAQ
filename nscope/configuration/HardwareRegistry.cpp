#include "HardwareRegistry.h"

#include <algorithm>
#include <tuple>
#include <map>



namespace HR = DAQ::DDAS::HardwareRegistry;
using Registry = std::map<int, HR::HardwareSpecification>;

// static registry
static Registry* gpRegistry = nullptr;
static int sDefaultFirstAvailableUserType = 100;
static int sNextAvailableUserType = sDefaultFirstAvailableUserType;


//////////////////////////////////////////////////////////////////////////////

// static utility methods

// set up the registry with default parameters
static void setUpRegistry(Registry& registry) {
    registry[HR::RevB_100MHz_12Bit] = {100, 12, 11, 10. }; // {freq, bits, hdwr rev, calib}
    registry[HR::RevC_100MHz_12Bit] = {100, 12, 12, 10. };
    registry[HR::RevD_100MHz_12Bit] = {100, 12, 13, 10. };
    registry[HR::RevF_100MHz_14Bit] = {100, 14, 15, 10. };
    registry[HR::RevF_100MHz_16Bit] = {100, 16, 15, 10. };
    registry[HR::RevF_250MHz_12Bit] = {250, 12, 15, 8. };
    registry[HR::RevF_250MHz_14Bit] = {250, 14, 15, 8. };
    registry[HR::RevF_250MHz_16Bit] = {250, 16, 15, 8. };
    registry[HR::RevF_500MHz_12Bit] = {500, 12, 15, 10. };
    registry[HR::RevF_500MHz_14Bit] = {500, 14, 15, 10. };
    registry[HR::RevF_500MHz_16Bit] = {500, 16, 15, 10. };
}

// create a new register and set default values
static Registry* createRegistry() {
    gpRegistry = new Registry;

    setUpRegistry(*gpRegistry);

    return gpRegistry;
}

// avoid static initialization order fiasco by using a construct on first use idiom
static Registry& getRegistry() {
    if (gpRegistry == nullptr) {
        return *createRegistry();
    } else {
        return *gpRegistry;
    }
}

///////////////////////////////////////////////////////////////////////////////


bool operator==(const HR::HardwareSpecification& lhs,
                const HR::HardwareSpecification& rhs)
{
    return ((lhs.s_adcFrequency == rhs.s_adcFrequency)
            && (lhs.s_adcResolution == rhs.s_adcResolution)
            && (lhs.s_hdwrRevision == rhs.s_hdwrRevision));
}


namespace DAQ {
namespace DDAS {
namespace HardwareRegistry {


/*!
 * \brief Configure the specifications associated with a hardware type
 *
 * \param type  the enumerated hardware type
 * \param spec  a specification to assign
 *
 * This method replaces whatever specification prexisted that was associated with
 * the hardware type.
 */
void configureHardwareType(int type, const HardwareSpecification &spec)
{
    getRegistry()[type] = spec;
}

/*!
 * \brief Retrieve a reference to the current hdwr specification for a hardware type
 *
 * \param type  the enumerated hardware type
 *
 * \return reference to a hardware specificiation
 *
 * \throws std::runtime_error if no specification exists for the hardware type provided.
 */
HardwareSpecification& getSpecification(int type)
{
    Registry& registry = getRegistry();

    auto pFound = registry.find(type);
    if (pFound == registry.end()) {
        std::string errmsg("HardwareRegistry::getSpecification() ");
        errmsg += "Failed to locate specification for provided hardware type.";
        throw std::runtime_error(errmsg);
    }

    return pFound->second;
}

/*!
 * \brief Reset the contents of the registry to the default state
 */
void resetToDefaults()
{
    Registry& registry = getRegistry();
    registry.clear();
    sNextAvailableUserType = sDefaultFirstAvailableUserType;
    setUpRegistry(registry);
}


/*!
 * \brief Lookup a hardware type enumeration given info about a module
 *
 * \param hdwrVersion   hardware revision
 * \param adcFreq       adc sampling frequency
 * \param adcRes        adc resolution (e.g. 12 or 14)
 *
 * \return an enumerated hardware type
 */
int computeHardwareType(int hdwrVersion, int adcFreq, int adcRes)
{
    HardwareSpecification spec = {adcFreq, adcRes, hdwrVersion};
    Registry& registry = getRegistry();
    auto res = std::find_if(registry.begin(), registry.end(),
              [&spec](const Registry::value_type& element) {
        return (element.second == spec);
    });

    if (res != registry.end()) {
        return res->first;
    } else {
        return Unknown;
    }
}

int createHardwareType(int hdwrVersion, int adcFreq, int adcRes, double clockCalibration)
{
    Registry& registry = getRegistry();

    int type = computeHardwareType(hdwrVersion, adcFreq, adcRes);
    if (type == Unknown) {
        registry[sNextAvailableUserType] = {adcFreq, adcRes, hdwrVersion, clockCalibration};
        type = sNextAvailableUserType++;
    }
    return type;
}

} // end HardwareRegistry namespace
} // end DDAS namespace
} // end DAQ namespace
