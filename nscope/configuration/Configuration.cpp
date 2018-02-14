#include "Configuration.h"

#include "FirmwareVersionFileParser.h"
#include "ConfigurationParser.h"
#include "ModEvtFileParser.h"

#include <iostream>
#include <fstream>

namespace DAQ {
namespace DDAS {


/*!
 * \brief Set the crate id for the module
 * \param id    the id to assign
 *
 * Note that this is not currently used for anything.
 */
void Configuration::setCrateId(int id)
{
    m_crateId = id;
}

/*!
 * \return The crate id
 */
int Configuration::getCrateId() const
{
    return m_crateId;
}

/*!
 * \brief Set the number of modules in the crate
 * \param size  number of modules
 *
 * This resizes both the vectors storing the slot map and
 * module event lengths to be consistent.  The caller
 * should call setNumberOfModules prior to calling setSlotMap()
 * or setModuleEventLengths().
 */
void Configuration::setNumberOfModules(size_t size)
{
    m_slotMap.resize(size);
    m_modEvtLengths.resize(size);
    m_hardwareMap.resize(size);
}

/*!
 * \return the number of modules
 */
size_t Configuration::getNumberOfModules() const
{
    return m_slotMap.size();
}

/*!
 * \brief Assign a new slot map
 *
 * \param map   the slots that are occupied
 *
 * \throws std::runtime_error when length of argument is different than length of stored modevtlen vector
 *
 * It is important for the caller to first call setNumberOfModules()
 * before calling this to avoid an exception being thrown. To avoid
 * weird configurations, this ensures that the length of the slot map
 * is the same as the module event length vector at all times. If
 * the user has not set the number of modules previously, this cannot
 * be gauranteed and the method will almost always throw.
 *
 * \code
 *  Configuration config;
 *  config.setNumberOfModules(2);
 *  config.setSlotMap({2, 3});
 * \endcode
 */
void Configuration::setSlotMap(const std::vector<unsigned short> &map)
{
    if (map.size() != m_modEvtLengths.size()) {
        std::string errmsg;
        errmsg += "Configuration::setSlotMap() ";
        errmsg += "Inconsistent data for module evt lengths and slot mapping. ";
        errmsg += "Set number of modules first using Configuration::setNumberOfModules().";

        throw std::runtime_error(errmsg);
    }

    m_slotMap = map;
}

/*!
 * \return vector containing the slots that are filled
 */
std::vector<unsigned short> Configuration::getSlotMap() const
{
    return m_slotMap;
}

/*!
 * \brief Set the path to the .set file
 *
 * \param path  the path to the settings file
 */
void Configuration::setSettingsFilePath(const std::string &path)
{
    m_settingsFilePath = path;
}

/*!
 * \return the settings file path
 */
std::string Configuration::getSettingsFilePath() const
{
    return m_settingsFilePath;
}

/*!
 * \brief Set the firmware configuration for a hardware type
 *
 * \param specifier     the hardware type
 * \param config        the new configuration
 *
 * Any previous FirmwareConfiguration stored will be replaced by the new
 * configuration. If there is no previous configuration for the hardware type
 * it will be added.
 */
void Configuration::setFirmwareConfiguration(int specifier,
                                             const FirmwareConfiguration &config)
{
    m_fwMap[specifier] = config;
}


/*!
 * \brief Retrieve the current firmware specifier for the
 *
 * \param hdwrType      the hardware specifier associated with the firmware configuration
 *
 * \return  the firmware configuration associated with the hdwrType
 *
 * \throws std::runtime_error if no firmware configuration exists for the provided hdwrType
 */
FirmwareConfiguration& Configuration::getFirmwareConfiguration(int hdwrType)
{
    auto pSpec = m_fwMap.find(hdwrType);
    if (pSpec == m_fwMap.end()) {
        throw std::runtime_error("Unable to locate firmware configuration for firmware specifier");
    }

    return pSpec->second;
}

/*!
 * \brief Set the lengths of events for each module
 *
 * \param lengths   the module event lengths
 *
 * \throws std::runtime_error if size of lengths does not match size of stored slot map.
 *
 * It is necessary that the caller has previously invoked setNumberOfModules()
 * before calling this. The logic of this method aims to keep the slot map
 * and module event length vectors the same length. Without invoking
 * setNumberOfModules() this is most likely not going to be the case.
 */
void Configuration::setModuleEventLengths(const std::vector<int> &lengths)
{
    if (lengths.size() != m_slotMap.size()) {
        std::string errmsg;
        errmsg += "Configuration::setModuleEventLengths() ";
        errmsg += "Inconsistent data for module evt lengths and slot mapping. ";
        errmsg += "Set number of modules first using Configuration::setNumberOfModules().";
        throw std::runtime_error(errmsg);
    }

    m_modEvtLengths = lengths;
}

/*!
 * \return copy of module event lengths vector
 */
std::vector<int> Configuration::getModuleEventLengths() const
{
    return m_modEvtLengths;
}


std::vector<int> Configuration::getHardwareMap() const
{
    return m_hardwareMap;
}


void Configuration::setHardwareMap(const std::vector<int> &map)
{
    if (map.size() != m_slotMap.size()) {
        std::string errmsg;
        errmsg += "Configuration::setHardwareMap() ";
        errmsg += "Inconsistent data for hardware mapping and slot mapping. ";
        errmsg += "Set number of modules first using Configuration::setNumberOfModules().";
        throw std::runtime_error(errmsg);
    }

    m_hardwareMap = map;
}

/*!
 * \brief Print brief line of information for cfgPixie16.txt
 *
 * \param stream    the ostream to write to
 *
 * Prints out a message similar to :
 * "Crate number 1: 2 modules, in slots:2 10 DSPParFile: /path/to/file.set"
 *
 */
void Configuration::print(std::ostream &stream)
{
    stream << "Crate number " << m_crateId;
    stream << ": " << m_slotMap.size() << " modules, in slots:";

    for(auto& slot : m_slotMap){
        stream << slot << " ";
    }
    stream << "DSPParFile: " << m_settingsFilePath;


}


std::unique_ptr<Configuration>
Configuration::generate(const std::string &fwVsnPath, const std::string &cfgPixiePath)
{
    std::unique_ptr<Configuration> pConfig(new Configuration);

    FirmwareVersionFileParser fwFileParser;
    ConfigurationParser       configParser;

    std::ifstream input(fwVsnPath.c_str(), std::ios::in);

    if(input.fail()) {
      std::string errmsg("Configuration::generate() ");
      errmsg += "Failed to open the firmware version file : ";
      errmsg += fwVsnPath;
      throw std::runtime_error(errmsg);
    }

    fwFileParser.parse(input, *pConfig);

    input.close();
    input.clear();

    input.open(cfgPixiePath.c_str(), std::ios::in);

    if(input.fail()){
        std::string errmsg("Configuration::generate() ");
        errmsg += "Failed to open the system configuration file : ";
        errmsg += cfgPixiePath;
        throw std::runtime_error(errmsg);
    }

    configParser.parse(input, *pConfig);

    return std::move(pConfig);

}

std::unique_ptr<Configuration> Configuration::generate(const std::string &fwVsnPath,
                                                       const std::string &cfgPixiePath,
                                                       const std::string &modEvtLenPath)
{
    ModEvtFileParser modEvtParser;

    std::unique_ptr<Configuration> pConfig = generate(fwVsnPath, cfgPixiePath);

    int moduleCount = pConfig->getNumberOfModules();

    // read a configration file to tell Pixie16 how big an event is in
    // a particular module.  Within one module all channels MUST be set to
    // the same event length

    std::ifstream modevt;
    modevt.open(modEvtLenPath.c_str(), std::ios::in);

    if(!modevt.is_open()){
        std::string errmsg("Configuration::generate() ");
        errmsg += "Failed to open the module event length configuration file : ";
        errmsg += modEvtLenPath;
        throw std::runtime_error(errmsg);
    }

    modEvtParser.parse(modevt, *pConfig);

    return std::move(pConfig);
}


} // end DDAS namespace
} // end DAQ namespace
