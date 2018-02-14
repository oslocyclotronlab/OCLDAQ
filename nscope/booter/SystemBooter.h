#ifndef SYSTEMBOOTER_H
#define SYSTEMBOOTER_H

namespace DAQ {
namespace DDAS {

class Configuration;

/*!
 * \brief The SystemBooter class
 *
 * A class to manage the booting process for DDAS. All Readout and nscope
 * program rely on this class to boot the system. There are two separate
 * boot types: FullBoot and SettingsOnly. The former loads firmware and
 * settings into the system while the latter just loads the settings. The
 * basic usage pattern is demonstrated below.
 *
 * \code
 *
 * using namespace DAQ::DDAS;
 *
 * unique_ptr<Configuration> pConfig = Configuration::generator("DDASFirmwareVersions.txt", "cfgPixie16.txt");
 *
 * SystemBooter booter;
 * booter.boot(*pConfig, SystemBooter::FullBoot);
 *
 * \endcode
 *
 * One should realize that this does not handle any of the logic regarding
 * when and when not to synchronize or load firmware. External logic to this class
 * will determine whether the system should load the firmware or not. Synchronization
 * is unrelated to the boot process besides the fact that a firmware load could
 * ruin synchronization.
 *
 */
class SystemBooter
{

    // docs will provided in the source file. Doxygen output should be provided to users.

public:
    enum BootType { FullBoot = 0x7f, SettingsOnly=0x70 };

private:
    bool m_verbose;  /// enable or disable output

public:
    SystemBooter();

    void boot(Configuration& config, BootType type);

    void bootModuleByIndex(int modIndex, Configuration& config, BootType type);

    void setVerbose(bool enable);
    bool isVerbose() const;

    void populateHardwareMap(Configuration &config);
private:

    unsigned int computeBootMask(BootType type);

    void logModuleInfo(int modIndex, unsigned short ModRev, unsigned short ModSerNum, unsigned short ModADCBits,
                       unsigned short ModADCMSPS);


};

} // end DDAS namespace
} // end DAQ namespace

#endif // SYSTEMBOOTER_H
